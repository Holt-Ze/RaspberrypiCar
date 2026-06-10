/*
 * LED 控制 - GPIO 18 (BCM编号)
 * S 接 GPIO 18, VCC 接 3.3V, GND 接地
 *
 * 高电平 → 灯灭（两端都是3.3V，无压差）
 * 低电平 → 灯亮（电流从3.3V流过LED到GPIO）
 *
 * 编译: gcc -o led_control led_control.c -lwiringPi
 * 运行: sudo ./led_control
 */

#include <stdio.h>
#include <wiringPi.h>

#define LED_PIN     18
#define DELAY_MS    500

int main(void) {
    wiringPiSetupGpio();
    pinMode(LED_PIN, OUTPUT);

    printf("GPIO %d LED 控制程序\n", LED_PIN);
    printf("Ctrl+C 退出\n\n");

    for (int i = 0; i < 10; i++) {
        printf("亮 (GPIO 低电平)\n");
        digitalWrite(LED_PIN, LOW);
        delay(DELAY_MS);

        printf("灭 (GPIO 高电平)\n");
        digitalWrite(LED_PIN, HIGH);
        delay(DELAY_MS);
    }

    digitalWrite(LED_PIN, HIGH);
    printf("结束\n");
    return 0;
}
