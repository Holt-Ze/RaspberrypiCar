/*
 * HC-SR04 超声波测距测试
 *
 * Trig → GPIO 12 (输出)
 * Echo → GPIO 6  (输入, 需 5V→3.3V 分压)
 * VCC → 5V (注意: 必须5V供电)
 * GND → GND
 *
 * 工作原理:
 *   1. Trig 发 10µs 高电平脉冲
 *   2. Echo 返回高电平，持续时长 = 声波往返时间
 *   3. 距离 = 高电平时间 * 声速 / 2
 *
 * 编译: gcc -o ultrasonic ultrasonic.c -lwiringPi
 * 运行: sudo ./ultrasonic
 */

#include <stdio.h>
#include <wiringPi.h>

#define TRIG  12
#define ECHO  6

/* 等待引脚变为指定电平，返回高电平持续时间(µs)，超时返回 -1 */
static long pulse_in(int pin, int level, long timeout_us) {
    long start = micros();

    while (digitalRead(pin) != level)
        if (micros() - start > timeout_us) return -1;

    long t_rise = micros();

    while (digitalRead(pin) == level)
        if (micros() - start > timeout_us + 200000L) return -1;

    return micros() - t_rise;
}

int main(void) {
    wiringPiSetupGpio();

    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    digitalWrite(TRIG, LOW);

    printf("HC-SR04 超声波测距\n");
    printf("Trig=GPIO%d  Echo=GPIO%d\n", TRIG, ECHO);
    printf("Ctrl+C 退出\n\n");

    while (1) {
        // 发 10µs 触发脉冲
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);

        // 读 Echo 高电平持续时间
        long duration = pulse_in(ECHO, HIGH, 100000L);

        if (duration < 0) {
            printf("\r超时 (无回波)                    ");
        } else {
            float dist = duration * 0.0343f / 2.0f;
            printf("\r脉宽: %4ld µs | 距离: %5.1f cm  ", duration, dist);
        }
        fflush(stdout);

        delay(100);
    }

    return 0;
}
