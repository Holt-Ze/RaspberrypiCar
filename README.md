# 智能停车场道闸系统 (Smart Parking Barrier System)

基于树莓派 4 的嵌入式课程设计项目，模拟停车场道闸的昼夜自动控制。

**v4.0 — WiringPi 重构版**：统一使用 WiringPi 库替代 sysfs/mmio/I2C-ioctl 三种低层接口，
代码量减少 ~40%，可读性大幅提升。

## 功能演示

| 场景 | 光照 | 栏杆状态 | 说明 |
|------|------|----------|------|
| ☀ 白天 | \> 40% | 常开 | 自由通行，超声波和蜂鸣器不工作 |
| 🌙 夜晚 | ≤ 40% | 关闭 | 进入监控模式，等待来车 |

**夜晚状态机:**

```
WAIT ──车来(<80cm)──▶ CAR_HERE ──车离开──▶ CLOSING(3s倒计时)
  ▲                                                  │
  └──────────── COOLDOWN(3s冷却) ◀──────────────────┘
```

1. 遮住光敏传感器 → 进入夜晚模式，栏杆关闭
2. 手靠近超声波 (< 80cm) → 模拟来车，栏杆打开 + 蜂鸣提示
3. 手移开 → 3 秒倒计时后栏杆关闭
4. 冷却 3 秒 → 重新等待下一辆车

## 硬件清单

| 序号 | 组件 | 型号 | 数量 |
|------|------|------|------|
| 1 | 树莓派 | Raspberry Pi 4 Model B | 1 |
| 2 | ADC 模块 | PCF8591 | 1 |
| 3 | 光敏传感器 | 光敏电阻模块 (接 PCF8591 AIN0) | 1 |
| 4 | 超声波测距 | HC-SR04 | 1 |
| 5 | 舵机 | SG90 / MG996R | 1 |
| 6 | 蜂鸣器 | 有源蜂鸣器 (高电平触发) | 1 |
| 7 | 电阻 | 2.2kΩ + 3.3kΩ (Echo 分压) | 各 1 |
| 8 | 杜邦线 + 面包板 | — | 若干 |

> 详细接线图见 [WIRING.md](WIRING.md)

## 项目结构

```
Cproject/
├── read_light.c       # 光敏传感器单独测试
├── led_control.c      # LED 闪烁测试 (GPIO sysfs)
├── servo_light.c      # 光敏 + 舵机 + 蜂鸣器联动测试
├── ultrasonic.c       # HC-SR04 超声波测距测试
├── parking_radar.c    # ★ 主程序: 智能停车场完整逻辑
├── WIRING.md          # 详细接线图
└── README.md          # 本文件
```

每个 `.c` 文件都是独立的可执行程序，各测试模块的功能：

| 文件 | 硬件接口 | 功能 |
|------|----------|------|
| `read_light.c` | PCF8591 (wiringPiI2C) | 循环读取光敏传感器 ADC 值并显示百分比 |
| `led_control.c` | GPIO18 (wiringPi) | LED 闪烁 10 次，演示 digitalWrite 控制 |
| `servo_light.c` | PCF8591 + 舵机(softPwm) + 蜂鸣器 | 根据光照自动控制舵机角度和蜂鸣器报警 |
| `ultrasonic.c` | HC-SR04 (wiringPi) | 循环测距并显示距离 |
| `parking_radar.c` | 全部硬件 (wiringPi + softPwm + wiringPiI2C) | ★ v4.0 完整道闸系统，含昼/夜状态机 |

## 编译与运行

### 前置条件

在 `/boot/config.txt` 中添加以下配置并重启:

```
dtoverlay=i2c-gpio,i2c_gpio_sda=18,i2c_gpio_scl=17,bus=3
dtoverlay=pwm,pin=19,func=2
```

### 编译

```bash
# 所有程序统一链接 WiringPi
gcc -o led_control led_control.c -lwiringPi
gcc -o read_light read_light.c -lwiringPi
gcc -o servo_light servo_light.c -lwiringPi
gcc -o ultrasonic ultrasonic.c -lwiringPi
gcc -o parking_radar parking_radar.c -lwiringPi
```

### 运行

所有程序需要 `sudo` (访问 `/dev/gpiomem`, `/sys/class/gpio` 等):

```bash
# 启动前加载设备树 overlay
sudo dtoverlay i2c-gpio i2c_gpio_sda=18 i2c_gpio_scl=17 bus=3
sudo dtoverlay pwm pin=19 func=2

# 各模块测试
sudo ./read_light      # 光敏传感器读数
sudo ./ultrasonic      # 超声波测距
sudo ./servo_light     # 光敏 → 舵机 + 蜂鸣器

# 主程序: 完整停车场系统
sudo ./parking_radar
```

按 `Ctrl+C` 退出。

## GPIO 引脚分配

| GPIO (BCM) | 物理引脚 | 连接设备 | 接口方式 |
|------------|----------|----------|----------|
| GPIO17 | 11 | PCF8591 SCL | 软件 I2C |
| GPIO18 | 12 | PCF8591 SDA | 软件 I2C |
| GPIO12 | 32 | HC-SR04 Trig | mmap 直接寄存器 |
| GPIO6 | 31 | HC-SR04 Echo | mmap 直接寄存器 (经分压) |
| GPIO19 | 35 | 舵机信号线 | PWM1 |
| GPIO21 | 40 | 蜂鸣器 I/O | sysfs GPIO |

## 软件架构

```
parking_radar.c (v4.0 WiringPi)
├── wiringPi              → GPIO: 超声波 Trig/Echo, 蜂鸣器
├── wiringPiI2C           → I2C: PCF8591 光敏传感器
└── softPwm               → PWM: 舵机 (GPIO19, 20ms周期)
```

- **WiringPi** 统一了全部硬件访问，替代了旧版的三套低层接口
- 超声波测距使用 `digitalWrite`/`digitalRead` + `micros()` 微秒级计时
- 光敏传感器通过 `wiringPiI2CSetup` / `wiringPiI2CRead` 读取 PCF8591
- 舵机使用 `softPwmCreate` + `softPwmWrite` 生成 20ms 周期 PWM 信号
- **统一、简洁、可读**——没有 sysfs 文件操作、没有 mmap 寄存器偏移、没有 ioctl 魔法数字

## 关键参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `DAY_THRESHOLD` | 40% | 光照低于此值进入夜晚模式 |
| `CAR_DISTANCE` | 80 cm | 超声波检测距离阈值 |
| `CLOSE_SEC` | 3 秒 | 车离开后关门倒计时 |
| `COOLDOWN_SEC` | 3 秒 | 关门后冷却时间 |

## 注意事项

1. **HC-SR04 Echo 必须接分压电路** — Echo 输出 5V，树莓派 GPIO 只能承受 3.3V，不接分压会损坏 GPIO
2. HC-SR04 需要 **5V 供电**，不能接 3.3V
3. 大舵机 (MG996R) 建议外接电源，树莓派 5V 引脚可能供电不足
4. 所有程序需 `sudo` 运行
