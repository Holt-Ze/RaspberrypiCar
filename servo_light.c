/*
 * 光敏传感器 + 舵机 + 蜂鸣器 联动测试
 *
 * 光照 > 50%  → 舵机 180°
 * 光照 20~50% → 舵机 0°
 * 光照 < 20%  → 舵机 0°, 蜂鸣器间歇报警
 *
 * PCF8591 SDA=GPIO18, SCL=GPIO17 (I2C bus 3)
 * 舵机: GPIO19 (softPwm)
 * 蜂鸣器: GPIO21
 *
 * 编译: gcc -o servo_light servo_light.c -lwiringPi
 * 运行: sudo ./servo_light
 */

#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiI2C.h>

#define PCF8591_ADDR    0x48
#define SERVO_PIN       19
#define BUZZER_PIN      21

#define SERVO_THRESHOLD   50
#define BUZZER_THRESHOLD  20

/* 舵机角度 → softPwm 映射: range=200 对应 20ms 周期 */
static void servo_set(int angle) {
    // 0° = 0.5ms = 5/200, 180° = 2.5ms = 25/200
    int duty = 5 + (angle * 20 / 180);
    softPwmWrite(SERVO_PIN, duty);
}

int main(void) {
    wiringPiSetupGpio();

    int adc_fd = wiringPiI2CSetup(PCF8591_ADDR);
    if (adc_fd < 0) {
        printf("PCF8591 初始化失败\n");
        return 1;
    }

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    softPwmCreate(SERVO_PIN, 0, 200);
    delay(500);

    printf("光敏传感器 + 舵机 + 蜂鸣器\n");
    printf(">%d%% 舵机180° | <%d%% 蜂鸣报警\n\n", SERVO_THRESHOLD, BUZZER_THRESHOLD);

    int last_servo = -1, last_buzzer = -1, beep = 0;

    while (1) {
        // 读光敏
        wiringPiI2CWrite(adc_fd, 0x00);
        delay(10);
        wiringPiI2CRead(adc_fd);
        int adc = wiringPiI2CRead(adc_fd);
        if (adc < 0) { delay(200); continue; }

        int light = 100 - (adc * 100 / 255);

        // 舵机
        int servo_pos;
        if (light > SERVO_THRESHOLD) { servo_set(180); servo_pos = 180; }
        else                         { servo_set(0);   servo_pos = 0;   }

        // 蜂鸣器间歇
        int buzzer_on;
        if (light < BUZZER_THRESHOLD) {
            beep = !beep;
            digitalWrite(BUZZER_PIN, beep ? HIGH : LOW);
            buzzer_on = 1;
        } else {
            digitalWrite(BUZZER_PIN, LOW);
            beep = 0;
            buzzer_on = 0;
        }

        if (servo_pos != last_servo || buzzer_on != last_buzzer) {
            const char *st;
            if (light < BUZZER_THRESHOLD)      st = "🚨 警报";
            else if (light > SERVO_THRESHOLD)  st = "☀ 明亮";
            else                               st = "⛅ 中等";

            printf("ADC:%-4d 光照:%-4d  舵机:%-4s  蜂鸣:%-4s  %s\n",
                   adc, light,
                   servo_pos == 180 ? "180°" : "0°",
                   buzzer_on ? "🔊" : "🔇", st);
            last_servo  = servo_pos;
            last_buzzer = buzzer_on;
        } else {
            printf("\rADC:%-4d 光照:%-4d  舵机:%-4s  蜂鸣:%-4s  ",
                   adc, light,
                   servo_pos == 180 ? "180°" : "0°",
                   buzzer_on ? "🔊" : "🔇");
            fflush(stdout);
        }

        delay(300);
    }

    return 0;
}
