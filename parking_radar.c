/*
 * 智能停车场道闸系统 v4.0 — WiringPi 重构版
 *
 * 演示流程:
 *   1. 不遮光 → ☀ 白天, 栏杆常开, 无反应
 *   2. 遮住光敏 → 🌙 夜晚, 栏杆关闭, 开始监控
 *   3. 手靠近超声波(<80cm) → 🚗 来车, 栏杆开 + 蜂鸣
 *   4. 手移开 → 3s 倒计时 → 栏杆关闭
 *   5. 冷却 3s → 重新等待下一辆车
 *
 * 硬件:
 *   PCF8591 光敏: SDA=GPIO18, SCL=GPIO17 (I2C bus 3, AIN0)
 *   HC-SR04 超声波: Trig=GPIO12, Echo=GPIO6
 *   舵机(栏杆): GPIO19 (softPwm, 20ms 周期)
 *   蜂鸣器: GPIO21 (高电平触发)
 *
 * 编译: gcc -o parking_radar parking_radar.c -lwiringPi
 * 运行: sudo ./parking_radar
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiI2C.h>

// ==================== 硬件引脚定义 ====================
#define TRIG_PIN        12
#define ECHO_PIN        6
#define BUZZER_PIN      21
#define SERVO_PIN       19
#define PCF8591_ADDR    0x48

// ==================== 伺服参数 (softPwm range=200 = 20ms周期) ====================
// 0°=0.5ms→duty=5, 90°=1.5ms→duty=15, 180°=2.5ms→duty=25
#define SERVO_MIN_DUTY  5
#define SERVO_MAX_DUTY  25
#define SERVO_RANGE     200

#define BARRIER_OPEN    90
#define BARRIER_CLOSE   0

// ==================== 演示参数 ====================
#define DAY_THRESHOLD   40      // 光照高于40% = 白天
#define CAR_DISTANCE    80      // 车辆检测阈值 (cm)
#define CLOSE_SEC       3       // 车离开后关门倒计时 (秒)
#define COOLDOWN_SEC    3       // 关门后冷却时间 (秒)

// ==================== 夜晚状态机 ====================
enum {
    NIGHT_WAIT,      // 等待来车
    NIGHT_CAR_HERE,  // 车在通道, 栏杆开
    NIGHT_CLOSING,   // 车离开, 倒计时关门
    NIGHT_COOLDOWN,  // 刚关门, 冷却中
};

static int i2c_fd;  // PCF8591 文件描述符

// ==================== 超声波测距 ====================
static float ultrasonic_read(void) {
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long t0 = micros();
    while (digitalRead(ECHO_PIN) == LOW)
        if (micros() - t0 > 50000L) return -1;

    long t1 = micros();
    while (digitalRead(ECHO_PIN) == HIGH)
        if (micros() - t1 > 50000L) return -1;

    return (micros() - t1) * 0.0343f / 2.0f;
}

/* 双采样防抖: 40ms内两次均 < 阈值才算检测到 */
static int car_detected(void) {
    float d1 = ultrasonic_read();
    delayMicroseconds(40000);
    float d2 = ultrasonic_read();
    return (d1 > 0 && d1 < CAR_DISTANCE && d2 > 0 && d2 < CAR_DISTANCE);
}

// ==================== 光敏传感器 ====================
static int light_read(void) {
    if (i2c_fd < 0) return -1;
    wiringPiI2CWrite(i2c_fd, 0x00);        // 选AIN0, 单端
    delay(10);
    wiringPiI2CRead(i2c_fd);                // 丢弃旧值
    int adc = wiringPiI2CRead(i2c_fd);      // 本次转换
    if (adc < 0) return -1;
    return 100 - (adc * 100 / 255);         // 转光照百分比
}

// ==================== 舵机 ====================
static void servo_set(int angle) {
    int duty = SERVO_MIN_DUTY + (SERVO_MAX_DUTY - SERVO_MIN_DUTY) * angle / 180;
    softPwmWrite(SERVO_PIN, duty);
}

// ==================== 蜂鸣器 ====================
static void buzzer_beep(int times, int on_ms, int off_ms) {
    for (int i = 0; i < times; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(on_ms);
        digitalWrite(BUZZER_PIN, LOW);
        if (i < times - 1) delay(off_ms);
    }
}

// ==================== 控制台显示 ====================
static void print_bar(const char *status, int open) {
    printf("\n  %s  [%s]  %s\n",
           open ? "▐▬▬▬▬▬▬▬▌" : "▐████████▌",
           open ? "开" : "关",
           status);
}

// ==================== 主程序 ====================
int main(void) {
    // ----- 初始化 WiringPi (BCM GPIO 编号) -----
    wiringPiSetupGpio();

    // ----- 超声波 -----
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);

    // ----- 光敏 (I2C) -----
    i2c_fd = wiringPiI2CSetup(PCF8591_ADDR);

    // ----- 蜂鸣器 -----
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // ----- 舵机 (softPwm, 20ms周期) -----
    softPwmCreate(SERVO_PIN, 0, SERVO_RANGE);
    delay(300);

    // ----- 初始模式判定 -----
    int light = (i2c_fd >= 0) ? light_read() : 100;
    int is_day = (light > DAY_THRESHOLD);
    int night_state = NIGHT_WAIT;
    time_t timer_start = 0;
    servo_set(is_day ? BARRIER_OPEN : BARRIER_CLOSE);

    int last_day = is_day, last_state = -1;

    printf("╔══════════════════════════════════════╗\n");
    printf("║  智能停车场道闸 v4.0 (WiringPi)  ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  遮住光敏 → 进入夜晚模式          ║\n");
    printf("║  手靠近超声波 → 模拟来车          ║\n");
    printf("║  手移开 → %ds 后关门              ║\n", CLOSE_SEC);
    printf("║  关门后冷却 %ds → 可再次触发      ║\n", COOLDOWN_SEC);
    printf("╚══════════════════════════════════════╝\n\n");

    // ==================== 主循环 ====================
    while (1) {
        // 读光照 (I2C异常时默认100%=白天)
        light = (i2c_fd >= 0) ? light_read() : 100;
        if (light < 0) light = 100;
        is_day = (light > DAY_THRESHOLD);

        // ----- 昼/夜切换 -----
        if (is_day != last_day) {
            printf("\n>>> %s模式 <<<\n", is_day ? "☀ 白天" : "🌙 夜晚");
            if (is_day) {
                servo_set(BARRIER_OPEN);
                digitalWrite(BUZZER_PIN, LOW);
                printf("    栏杆常开，自由通行\n\n");
            } else {
                servo_set(BARRIER_CLOSE);
                night_state = NIGHT_WAIT;
                printf("    栏杆关闭，等待来车...\n\n");
            }
            last_day = is_day;
        }

        if (is_day) {
            // ===== 白天: 自由通行 =====
            printf("\r☀ 白天  光照:%3d%%  ▐▬▬▬▬▬▬▬▌[开]  自由通行  ", light);
            fflush(stdout);
            delay(300);

        } else {
            // ===== 夜晚状态机 =====
            switch (night_state) {

            case NIGHT_WAIT:
                if (car_detected()) {
                    servo_set(BARRIER_OPEN);
                    buzzer_beep(3, 150, 80);
                    night_state = NIGHT_CAR_HERE;
                    print_bar("🚗 来车，栏杆打开", 1);
                }
                break;

            case NIGHT_CAR_HERE:
                if (!car_detected()) {
                    timer_start = time(NULL);
                    night_state = NIGHT_CLOSING;
                    printf("\n  ⏳ 车已离开，%ds 后关门...\n", CLOSE_SEC);
                }
                break;

            case NIGHT_CLOSING: {
                int remain = CLOSE_SEC - (int)(time(NULL) - timer_start);
                if (remain <= 0) {
                    servo_set(BARRIER_CLOSE);
                    buzzer_beep(1, 200, 0);
                    night_state = NIGHT_COOLDOWN;
                    timer_start = time(NULL);
                    print_bar("🔒 栏杆关闭，冷却中", 0);
                } else {
                    printf("\r  ⏳ 关门倒计时: %d 秒  ", remain);
                    fflush(stdout);
                }
                break;
            }

            case NIGHT_COOLDOWN: {
                int remain = COOLDOWN_SEC - (int)(time(NULL) - timer_start);
                if (remain <= 0) {
                    night_state = NIGHT_WAIT;
                    printf("\n  ✅ 冷却结束，等待下一辆车\n\n");
                } else {
                    printf("\r  🕐 冷却中: %d 秒  等待来车...", remain);
                    fflush(stdout);
                }
                break;
            }
            }

            if (night_state != last_state) last_state = night_state;
            delay(150);
        }
    }

    return 0;
}
