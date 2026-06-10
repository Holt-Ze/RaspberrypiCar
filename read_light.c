/*
 * PCF8591 光敏传感器读取
 * SDA → GPIO 18, SCL → GPIO 17 (软件 I2C, bus 3)
 *
 * 编译: gcc -o read_light read_light.c -lwiringPi
 * 运行: sudo ./read_light
 */

#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define PCF8591_ADDR  0x48

int main(void) {
    int fd = wiringPiI2CSetup(PCF8591_ADDR);
    if (fd < 0) {
        printf("打开 PCF8591 失败 (I2C bus 3)\n");
        printf("请先执行: sudo dtoverlay i2c-gpio i2c_gpio_sda=18 i2c_gpio_scl=17 bus=3\n");
        return 1;
    }

    printf("PCF8591 光敏传感器读取 (AIN0)\n");
    printf("Ctrl+C 退出\n\n");

    while (1) {
        // PCF8591: 写控制字节 → 等转换 → 读两次(第一次是旧值)
        wiringPiI2CWrite(fd, 0x00);
        delay(10);
        wiringPiI2CRead(fd);                    // 丢弃上一次转换结果
        int adc = wiringPiI2CRead(fd);           // 本次转换结果

        float voltage = adc / 255.0 * 3.3;
        int light_pct = 100 - (adc * 100 / 255);

        printf("\rADC: %3d | 电压: %.3fV | 光照: %3d%%  ",
               adc, voltage, light_pct);
        fflush(stdout);

        delay(200);
    }

    return 0;
}
