#!/usr/bin/env python3
"""生成课程设计报告所需的 SVG 流程图，并转为 PNG 嵌入 DOCX"""

import cairosvg, os

OUT = '/home/holt/Cproject/diagrams'
os.makedirs(OUT, exist_ok=True)

def svg2png(svg, name):
    path = os.path.join(OUT, name)
    with open(path + '.svg', 'w') as f:
        f.write(svg)
    cairosvg.svg2png(url=path + '.svg', write_to=path + '.png', output_width=1200)
    print(f'  ✓ {name} ({os.path.getsize(path+".png")//1024} KB)')
    return path + '.png'

# ═══════════════════════════════════════════════════
# 1. 硬件连接框图
# ═══════════════════════════════════════════════════
HW = '''<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 520"
     font-family="SimHei,Heiti SC,sans-serif" font-size="13">
  <defs>
    <linearGradient id="pi" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#2d5016"/><stop offset="100%" stop-color="#1a3008"/>
    </linearGradient>
    <linearGradient id="mod" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#2563eb"/><stop offset="100%" stop-color="#1d4ed8"/>
    </linearGradient>
    <filter id="shadow"><feDropShadow dx="2" dy="2" stdDeviation="3" flood-opacity="0.15"/></filter>
    <marker id="arr" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
      <polygon points="0 0, 10 3.5, 0 7" fill="#555"/>
    </marker>
  </defs>

  <!-- 背景 -->
  <rect width="900" height="520" rx="12" fill="#f8fafc" stroke="#cbd5e1" stroke-width="1.5"/>

  <!-- 树莓派 -->
  <rect x="280" y="40" width="340" height="440" rx="14" fill="url(#pi)" filter="url(#shadow)"/>
  <text x="450" y="75" text-anchor="middle" fill="#fff" font-size="18" font-weight="bold">Raspberry Pi 4 Model B</text>
  <line x1="330" y1="90" x2="570" y2="90" stroke="#4ade80" stroke-width="1" opacity="0.4"/>

  <!-- 引脚标签 -->
  <g fill="#e2e8f0" font-size="11">
    <text x="310" y="125">GPIO17 (Pin 11) — SCL ────────▶</text>
    <text x="310" y="155">GPIO18 (Pin 12) — SDA ────────▶</text>
    <text x="310" y="190">GPIO12 (Pin 32) — Trig ───────▶</text>
    <text x="310" y="220">GPIO6  (Pin 31) — Echo ◀──────</text>
    <text x="310" y="255">GPIO19 (Pin 35) — PWM ────────▶</text>
    <text x="310" y="285">GPIO21 (Pin 40) — I/O ────────▶</text>
  </g>

  <!-- 电源 -->
  <g fill="#94a3b8" font-size="10">
    <text x="310" y="330">3.3V (Pin 1/17) ——— PCF8591 / 蜂鸣器 VCC</text>
    <text x="310" y="355">5V   (Pin 2/4)  ——— HC-SR04 / 舵机 VCC</text>
    <text x="310" y="380">GND  (Pin 6/9/14/20/30/34/39) ——— 全部模块</text>
  </g>

  <!-- WiringPi 标签 -->
  <rect x="310" y="400" width="280" height="30" rx="6" fill="#4ade80" opacity="0.2"/>
  <text x="450" y="420" text-anchor="middle" fill="#4ade80" font-size="12">wiringPiSetupGpio() + wiringPiI2C + softPwm</text>
  <text x="450" y="445" text-anchor="middle" fill="#94a3b8" font-size="10">统一硬件抽象层</text>

  <!-- 左侧模块 -->
  <g filter="url(#shadow)">
    <rect x="30" y="55" width="180" height="80" rx="8" fill="url(#mod)"/>
    <text x="120" y="85" text-anchor="middle" fill="#fff" font-size="13" font-weight="bold">PCF8591 ADC</text>
    <text x="120" y="105" text-anchor="middle" fill="#bfdbfe" font-size="10">I2C 地址: 0x48</text>
    <text x="120" y="122" text-anchor="middle" fill="#bfdbfe" font-size="10">光敏电阻 → AIN0</text>
  </g>

  <g filter="url(#shadow)">
    <rect x="30" y="170" width="180" height="80" rx="8" fill="url(#mod)"/>
    <text x="120" y="200" text-anchor="middle" fill="#fff" font-size="13" font-weight="bold">HC-SR04 超声波</text>
    <text x="120" y="220" text-anchor="middle" fill="#bfdbfe" font-size="10">40kHz 测距</text>
    <text x="120" y="237" text-anchor="middle" fill="#bfdbfe" font-size="10">Trig/Echo (分压)</text>
  </g>

  <g filter="url(#shadow)">
    <rect x="30" y="280" width="180" height="70" rx="8" fill="url(#mod)"/>
    <text x="120" y="308" text-anchor="middle" fill="#fff" font-size="13" font-weight="bold">SG90 舵机</text>
    <text x="120" y="328" text-anchor="middle" fill="#bfdbfe" font-size="10">PWM 20ms 周期</text>
  </g>

  <g filter="url(#shadow)">
    <rect x="30" y="375" width="180" height="65" rx="8" fill="url(#mod)"/>
    <text x="120" y="400" text-anchor="middle" fill="#fff" font-size="13" font-weight="bold">有源蜂鸣器</text>
    <text x="120" y="420" text-anchor="middle" fill="#bfdbfe" font-size="10">高电平触发</text>
  </g>

  <!-- 连接线 -->
  <g stroke="#94a3b8" stroke-width="1.5" fill="none" marker-end="url(#arr)">
    <path d="M210,95 C240,95 250,95 280,113"/>
    <path d="M210,210 C240,210 250,200 280,205"/>
    <path d="M210,315 C240,315 250,250 280,250"/>
    <path d="M210,407 C240,407 250,280 280,280"/>
  </g>

  <!-- 右侧说明 -->
  <g font-size="10" fill="#64748b">
    <text x="650" y="130">⚡ HC-SR04 Echo 须经</text>
    <text x="660" y="148">2.2kΩ+3.3kΩ 分压</text>
    <text x="660" y="166">(5V→3.0V)</text>
    <text x="650" y="210">🔧 大舵机(MG996R)</text>
    <text x="660" y="228">建议独立 5V/2A 供电</text>
    <text x="650" y="280">📝 编译: gcc -lwiringPi</text>
    <text x="650" y="310">📝 运行: sudo ./程序</text>
  </g>
</svg>'''

# ═══════════════════════════════════════════════════
# 2. 主程序流程图
# ═══════════════════════════════════════════════════
FLOW = '''<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 600 900"
     font-family="SimHei,Heiti SC,sans-serif" font-size="12">
  <defs>
    <linearGradient id="start" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#1e40af"/><stop offset="100%" stop-color="#3b82f6"/>
    </linearGradient>
    <linearGradient id="proc" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#2563eb"/><stop offset="100%" stop-color="#60a5fa"/>
    </linearGradient>
    <linearGradient id="dec" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#ea580c"/><stop offset="100%" stop-color="#f97316"/>
    </linearGradient>
    <linearGradient id="day" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#eab308"/><stop offset="100%" stop-color="#facc15"/>
    </linearGradient>
    <linearGradient id="night" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#7c3aed"/><stop offset="100%" stop-color="#a78bfa"/>
    </linearGradient>
    <filter id="sh"><feDropShadow dx="1" dy="2" stdDeviation="2" flood-opacity="0.12"/></filter>
    <marker id="arr" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto">
      <polygon points="0 0, 8 3, 0 6" fill="#475569"/>
    </marker>
  </defs>

  <rect width="600" height="900" rx="10" fill="#f1f5f9"/>

  <!-- 节点坐标: 中心x=300 -->

  <!-- START -->
  <rect x="200" y="20" width="200" height="40" rx="20" fill="url(#start)" filter="url(#sh)"/>
  <text x="300" y="45" text-anchor="middle" fill="#fff" font-size="13" font-weight="bold">程序启动</text>
  <line x1="300" y1="60" x2="300" y2="80" stroke="#475569" stroke-width="1.5" marker-end="url(#arr)"/>

  <!-- 硬件初始化 -->
  <rect x="180" y="82" width="240" height="52" rx="8" fill="url(#proc)" filter="url(#sh)"/>
  <text x="300" y="103" text-anchor="middle" fill="#fff" font-size="12" font-weight="bold">硬件初始化</text>
  <text x="300" y="122" text-anchor="middle" fill="#bfdbfe" font-size="10">wiringPiSetup / pinMode / I2C / softPwm</text>
  <line x1="300" y1="134" x2="300" y2="158" stroke="#475569" stroke-width="1.5" marker-end="url(#arr)"/>

  <!-- 读光照 -->
  <rect x="190" y="160" width="220" height="40" rx="8" fill="url(#proc)" filter="url(#sh)"/>
  <text x="300" y="185" text-anchor="middle" fill="#fff" font-size="12" font-weight="bold">读取光照 light_read()</text>
  <line x1="300" y1="200" x2="300" y2="224" stroke="#475569" stroke-width="1.5" marker-end="url(#arr)"/>

  <!-- 判定昼/夜 -->
  <polygon points="300,226 380,266 300,306 220,266" fill="url(#dec)" filter="url(#sh)"/>
  <text x="300" y="270" text-anchor="middle" fill="#fff" font-size="12" font-weight="bold">白天模式?</text>

  <!-- 是→白天 -->
  <line x1="220" y1="266" x2="120" y2="266" stroke="#475569" stroke-width="1.5" marker-end="url(#arr)"/>
  <text x="165" y="258" text-anchor="middle" fill="#16a34a" font-size="10" font-weight="bold">是</text>

  <rect x="30" y="238" width="90" height="56" rx="8" fill="url(#day)" filter="url(#sh)"/>
  <text x="75" y="260" text-anchor="middle" fill="#422006" font-size="11" font-weight="bold">☀ 白天</text>
  <text x="75" y="278" text-anchor="middle" fill="#422006" font-size="9">道闸常开</text>

  <!-- 白天回到循环 -->
  <path d="M75,294 L75,800 L150,800 L150,780" stroke="#475569" stroke-width="1.5" fill="none" marker-end="url(#arr)"/>

  <!-- 否→夜晚状态机 -->
  <line x1="380" y1="266" x2="460" y2="266" stroke="#475569" stroke-width="1.5" marker-end="url(#arr)"/>
  <text x="425" y="258" text-anchor="middle" fill="#dc2626" font-size="10" font-weight="bold">否</text>

  <!-- 夜晚状态机框 -->
  <rect x="460" y="210" width="130" height="112" rx="8" fill="url(#night)" filter="url(#sh)"/>
  <text x="525" y="235" text-anchor="middle" fill="#fff" font-size="12" font-weight="bold">🌙 夜晚状态机</text>
  <g font-size="9" fill="#d1d5db">
    <text x="525" y="255" text-anchor="middle">NIGHT_WAIT</text>
    <text x="525" y="270" text-anchor="middle">    ↓</text>
    <text x="525" y="285" text-anchor="middle">NIGHT_CAR_HERE</text>
    <text x="525" y="300" text-anchor="middle">    ↓</text>
    <text x="525" y="315" text-anchor="middle">NIGHT_CLOSING</text>
  </g>

  <!-- 夜晚回到循环 -->
  <path d="M525,322 L525,330 L470,330 L470,350" stroke="#475569" stroke-width="1.5" fill="none"/>

  <!-- 延时+循环 -->
  <path d="M120,780 L300,780" stroke="#475569" stroke-width="1.5" fill="none" marker-end="url(#arr)"/>
  <path d="M525,350 L560,350 L560,400 L200,400 L200,780" stroke="#475569" stroke-width="1.5" fill="none"/>

  <rect x="200" y="770" width="200" height="40" rx="8" fill="url(#proc)" filter="url(#sh)"/>
  <text x="300" y="795" text-anchor="middle" fill="#fff" font-size="12" font-weight="bold">delay(150~300ms) 循环</text>

  <!-- 回到读光照 -->
  <path d="M300,810 L300,840 L560,840 L560,490 L560,490 L300,490 L300,160" stroke="#475569" stroke-width="1.5" fill="none" marker-end="url(#arr)" stroke-dasharray="6,4"/>
  <text x="570" y="620" fill="#64748b" font-size="10">循环</text>

  <!-- 图例 -->
  <g transform="translate(30,850)">
    <rect x="0" y="0" width="160" height="40" rx="6" fill="#fff" stroke="#cbd5e1" stroke-width="1"/>
    <rect x="10" y="10" width="24" height="16" rx="4" fill="url(#proc)"/>
    <text x="42" y="23" font-size="9" fill="#475569">处理步骤</text>
    <polygon points="90,18 102,18 96,28" fill="url(#dec)"/>
    <text x="110" y="23" font-size="9" fill="#475569">条件判断</text>
  </g>
</svg>'''

# ═══════════════════════════════════════════════════
# 3. 夜晚状态机转换图
# ═══════════════════════════════════════════════════
FSM = '''<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 800 600"
     font-family="SimHei,Heiti SC,sans-serif" font-size="12">
  <defs>
    <filter id="sh"><feDropShadow dx="2" dy="3" stdDeviation="3" flood-opacity="0.1"/></filter>
    <marker id="arrB" markerWidth="10" markerHeight="8" refX="9" refY="4" orient="auto">
      <polygon points="0 0, 10 4, 0 8" fill="#1e40af"/>
    </marker>
    <marker id="arrR" markerWidth="10" markerHeight="8" refX="9" refY="4" orient="auto">
      <polygon points="0 0, 10 4, 0 8" fill="#dc2626"/>
    </marker>
    <marker id="arrO" markerWidth="10" markerHeight="8" refX="9" refY="4" orient="auto">
      <polygon points="0 0, 10 4, 0 8" fill="#ea580c"/>
    </marker>
    <marker id="arrG" markerWidth="10" markerHeight="8" refX="9" refY="4" orient="auto">
      <polygon points="0 0, 10 4, 0 8" fill="#16a34a"/>
    </marker>
  </defs>

  <rect width="800" height="600" rx="12" fill="#f8fafc"/>
  <text x="400" y="30" text-anchor="middle" font-size="18" font-weight="bold" fill="#1e293b">夜晚模式状态机 (FSM)</text>
  <text x="400" y="50" text-anchor="middle" font-size="11" fill="#94a3b8">State Machine — Night Mode</text>

  <!-- State 1: NIGHT_WAIT (左上) -->
  <rect x="80" y="80" width="220" height="100" rx="12" fill="#eff6ff" stroke="#3b82f6" stroke-width="2.5" filter="url(#sh)"/>
  <text x="190" y="110" text-anchor="middle" font-size="14" font-weight="bold" fill="#1e40af">NIGHT_WAIT</text>
  <text x="190" y="130" text-anchor="middle" font-size="11" fill="#475569">等待车辆到达</text>
  <line x1="105" y1="140" x2="275" y2="140" stroke="#93c5fd" stroke-width="0.8"/>
  <text x="190" y="156" text-anchor="middle" font-size="10" fill="#64748b">栏杆: 关闭 (0°)</text>
  <text x="190" y="172" text-anchor="middle" font-size="10" fill="#64748b">蜂鸣器: 静音</text>

  <!-- State 2: NIGHT_CAR_HERE (右上) -->
  <rect x="500" y="80" width="220" height="100" rx="12" fill="#fef2f2" stroke="#ef4444" stroke-width="2.5" filter="url(#sh)"/>
  <text x="610" y="110" text-anchor="middle" font-size="14" font-weight="bold" fill="#b91c1c">NIGHT_CAR_HERE</text>
  <text x="610" y="130" text-anchor="middle" font-size="11" fill="#475569">车辆在通道中</text>
  <line x1="525" y1="140" x2="695" y2="140" stroke="#fca5a5" stroke-width="0.8"/>
  <text x="610" y="156" text-anchor="middle" font-size="10" fill="#64748b">栏杆: 打开 (90°)</text>
  <text x="610" y="172" text-anchor="middle" font-size="10" fill="#64748b">蜂鸣器: 3声短鸣</text>

  <!-- State 3: NIGHT_CLOSING (右下) -->
  <rect x="500" y="400" width="220" height="100" rx="12" fill="#fff7ed" stroke="#f97316" stroke-width="2.5" filter="url(#sh)"/>
  <text x="610" y="430" text-anchor="middle" font-size="14" font-weight="bold" fill="#9a3412">NIGHT_CLOSING</text>
  <text x="610" y="450" text-anchor="middle" font-size="11" fill="#475569">关门倒计时中</text>
  <line x1="525" y1="460" x2="695" y2="460" stroke="#fdba74" stroke-width="0.8"/>
  <text x="610" y="476" text-anchor="middle" font-size="10" fill="#64748b">栏杆: 仍打开</text>
  <text x="610" y="492" text-anchor="middle" font-size="10" fill="#64748b">显示剩余秒数</text>

  <!-- State 4: NIGHT_COOLDOWN (左下) -->
  <rect x="80" y="400" width="220" height="100" rx="12" fill="#f0fdf4" stroke="#22c55e" stroke-width="2.5" filter="url(#sh)"/>
  <text x="190" y="430" text-anchor="middle" font-size="14" font-weight="bold" fill="#166534">NIGHT_COOLDOWN</text>
  <text x="190" y="450" text-anchor="middle" font-size="11" fill="#475569">冷却期</text>
  <line x1="105" y1="460" x2="275" y2="460" stroke="#86efac" stroke-width="0.8"/>
  <text x="190" y="476" text-anchor="middle" font-size="10" fill="#64748b">栏杆: 关闭 (0°)</text>
  <text x="190" y="492" text-anchor="middle" font-size="10" fill="#64748b">蜂鸣器: 1声确认</text>

  <!-- 转换 WAIT → CAR_HERE -->
  <path d="M300,130 L500,130" stroke="#1e40af" stroke-width="2.5" fill="none" marker-end="url(#arrB)"/>
  <rect x="330" y="108" width="140" height="24" rx="5" fill="#dbeafe"/>
  <text x="400" y="124" text-anchor="middle" font-size="10" fill="#1e40af" font-weight="bold">car_detected() = True</text>
  <text x="400" y="140" text-anchor="middle" font-size="9" fill="#64748b">(40ms内两次均 &lt; 80cm)</text>

  <!-- 转换 CAR_HERE → CLOSING -->
  <path d="M610,180 L610,400" stroke="#dc2626" stroke-width="2.5" fill="none" marker-end="url(#arrR)"/>
  <rect x="622" y="260" width="130" height="45" rx="5" fill="#fee2e2"/>
  <text x="687" y="277" text-anchor="middle" font-size="10" fill="#dc2626" font-weight="bold">car_detected() = False</text>
  <text x="687" y="293" text-anchor="middle" font-size="9" fill="#64748b">车已离开</text>

  <!-- 转换 CLOSING → COOLDOWN -->
  <path d="M500,450 L300,450" stroke="#ea580c" stroke-width="2.5" fill="none" marker-end="url(#arrO)"/>
  <rect x="330" y="428" width="140" height="24" rx="5" fill="#ffedd5"/>
  <text x="400" y="444" text-anchor="middle" font-size="10" fill="#ea580c" font-weight="bold">elapsed ≥ 3 秒</text>

  <!-- 转换 COOLDOWN → WAIT -->
  <path d="M190,400 L190,180" stroke="#16a34a" stroke-width="2.5" fill="none" marker-end="url(#arrG)"/>
  <rect x="60" y="260" width="110" height="24" rx="5" fill="#dcfce7"/>
  <text x="115" y="276" text-anchor="middle" font-size="10" fill="#16a34a" font-weight="bold">elapsed ≥ 3 秒</text>
  <text x="115" y="292" text-anchor="middle" font-size="9" fill="#64748b">冷却结束</text>

  <!-- 图例 -->
  <g transform="translate(80,530)">
    <rect width="640" height="50" rx="8" fill="#fff" stroke="#e2e8f0" stroke-width="1"/>
    <g font-size="10" fill="#475569">
      <rect x="15" y="15" width="40" height="18" rx="4" fill="#eff6ff" stroke="#3b82f6" stroke-width="1"/>
      <text x="65" y="28">状态节点</text>
      <line x1="130" y1="24" x2="170" y2="24" stroke="#1e40af" stroke-width="2" marker-end="url(#arrB)"/>
      <text x="180" y="28">转换路径</text>
      <text x="270" y="28" fill="#64748b">| 触发条件写在路径上</text>
    </g>
  </g>
</svg>'''

# ═══════════════════════════════════════════════════
# 4. 软件架构层次图
# ═══════════════════════════════════════════════════
ARCH = '''<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 800 500"
     font-family="SimHei,Heiti SC,sans-serif" font-size="13">
  <defs>
    <marker id="arr" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto">
      <polygon points="0 0, 8 3, 0 6" fill="#475569"/>
    </marker>
    <filter id="sh"><feDropShadow dx="1" dy="2" stdDeviation="2" flood-opacity="0.1"/></filter>
    <linearGradient id="l1" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#1e40af"/><stop offset="100%" stop-color="#3b82f6"/>
    </linearGradient>
    <linearGradient id="l2" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#2563eb"/><stop offset="100%" stop-color="#60a5fa"/>
    </linearGradient>
    <linearGradient id="l3" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#0d9488"/><stop offset="100%" stop-color="#14b8a6"/>
    </linearGradient>
    <linearGradient id="l4" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#6366f1"/><stop offset="100%" stop-color="#818cf8"/>
    </linearGradient>
    <linearGradient id="l5" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0%" stop-color="#475569"/><stop offset="100%" stop-color="#64748b"/>
    </linearGradient>
  </defs>

  <rect width="800" height="500" rx="12" fill="#f8fafc"/>

  <!-- Layer 1: 应用层 -->
  <rect x="60" y="30" width="680" height="80" rx="10" fill="url(#l1)" filter="url(#sh)"/>
  <text x="400" y="58" text-anchor="middle" fill="#fff" font-size="16" font-weight="bold">应用层 — main()</text>
  <text x="400" y="82" text-anchor="middle" fill="#bfdbfe" font-size="12">昼夜模式管理 │ 状态机调度 (FSM) │ 控制台输出 (Unicode)</text>
  <text x="400" y="100" text-anchor="middle" fill="#93c5fd" font-size="10">application logic</text>

  <line x1="400" y1="110" x2="400" y2="130" stroke="#475569" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Layer 2: 功能模块层 -->
  <rect x="60" y="135" width="680" height="80" rx="10" fill="url(#l2)" filter="url(#sh)"/>
  <text x="400" y="163" text-anchor="middle" fill="#fff" font-size="16" font-weight="bold">功能模块层</text>
  <text x="400" y="187" text-anchor="middle" fill="#bfdbfe" font-size="12">ultrasonic_read() │ light_read() │ servo_set() │ buzzer_beep() │ print_bar()</text>
  <text x="400" y="205" text-anchor="middle" fill="#93c5fd" font-size="10">functional modules</text>

  <line x1="400" y1="215" x2="400" y2="235" stroke="#475569" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Layer 3: WiringPi 硬件抽象层 -->
  <rect x="60" y="240" width="680" height="80" rx="10" fill="url(#l3)" filter="url(#sh)"/>
  <text x="400" y="268" text-anchor="middle" fill="#fff" font-size="16" font-weight="bold">WiringPi 硬件抽象层 (HAL) ⭐</text>
  <text x="400" y="292" text-anchor="middle" fill="#ccfbf1" font-size="12">digitalWrite/Read │ wiringPiI2C │ softPwm │ pinMode │ micros │ delay</text>
  <text x="400" y="310" text-anchor="middle" fill="#99f6e4" font-size="10">hardware abstraction layer — 统一 API，一行 wiringPiSetupGpio() 初始化全部</text>

  <line x1="400" y1="320" x2="400" y2="340" stroke="#475569" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Layer 4: Linux 驱动层 -->
  <rect x="60" y="345" width="680" height="65" rx="10" fill="url(#l4)" filter="url(#sh)"/>
  <text x="400" y="373" text-anchor="middle" fill="#fff" font-size="16" font-weight="bold">Linux 内核驱动层</text>
  <text x="400" y="395" text-anchor="middle" fill="#c7d2fe" font-size="12">GPIO 子系统 │ I2C (i2c-gpio overlay) │ softPwm 线程 │ BCM2711 时钟</text>
  <text x="400" y="408" text-anchor="middle" fill="#a5b4fc" font-size="10">kernel drivers</text>

  <line x1="400" y1="410" x2="400" y2="425" stroke="#475569" stroke-width="2" marker-end="url(#arr)"/>

  <!-- Layer 5: 硬件层 -->
  <rect x="60" y="430" width="680" height="55" rx="10" fill="url(#l5)" filter="url(#sh)"/>
  <text x="400" y="458" text-anchor="middle" fill="#fff" font-size="16" font-weight="bold">BCM2711 硬件层</text>
  <text x="400" y="478" text-anchor="middle" fill="#cbd5e1" font-size="12">GPIO 寄存器 (0xFE200000) │ I2C 控制器 │ PWM 定时器 │ HC-SR04 / PCF8591 / SG90</text>
</svg>'''

# ═══════════════════════════════════════════════════
# Generate all
# ═══════════════════════════════════════════════════
print('生成流程图...')
p1 = svg2png(HW, 'fig1-1_hardware')
p2 = svg2png(FLOW, 'fig3-1_flowchart')
p3 = svg2png(FSM, 'fig3-2_statemachine')
p4 = svg2png(ARCH, 'fig3-3_architecture')

print(f'\n全部完成，保存于 {OUT}/')
print(f'  {p1}')
print(f'  {p2}')
print(f'  {p3}')
print(f'  {p4}')
