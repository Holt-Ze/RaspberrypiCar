/*
 * 生成课程设计报告所需的 SVG 流程图，并可选转为 PNG
 *
 * 编译: gcc -o generate_diagrams generate_diagrams.c
 * 运行: ./generate_diagrams
 *
 * 依赖: rsvg-convert (librsvg2-bin) 用于 SVG→PNG 转换 (可选)
 *       sudo apt install librsvg2-bin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OUT_DIR "/home/holt/Cproject/diagrams"

/* ==================== 工具函数 ==================== */

static int write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "  错误: 无法创建 %s\n", path);
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

static long file_size(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return sz;
}

/* 调用 rsvg-convert 将 SVG 转为 PNG; 失败仅警告不中断 */
static void svg_to_png(const char *name) {
    char svg_path[512], png_path[512], cmd[1024];
    snprintf(svg_path, sizeof(svg_path), "%s/%s.svg", OUT_DIR, name);
    snprintf(png_path, sizeof(png_path), "%s/%s.png", OUT_DIR, name);
    snprintf(cmd, sizeof(cmd),
             "rsvg-convert -w 1200 -o \"%s\" \"%s\" 2>/dev/null",
             png_path, svg_path);
    if (system(cmd) == 0) {
        printf("  + %s.png (%ld KB)\n", name, file_size(png_path) / 1024);
    } else {
        printf("  ~ %s.png 跳过 (需 apt install librsvg2-bin)\n", name);
    }
}

static void generate(const char *name, const char *svg) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s.svg", OUT_DIR, name);
    printf("  ✓ %s.svg", name);
    if (write_file(path, svg) != 0) return;
    fflush(stdout);
    svg_to_png(name);
}

/* ==================== SVG 字符串 ==================== */

static const char SVG_HW[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 900 520\"\n"
"     font-family=\"SimHei,Heiti SC,sans-serif\" font-size=\"13\">\n"
"  <defs>\n"
"    <linearGradient id=\"pi\" x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\">\n"
"      <stop offset=\"0%\" stop-color=\"#2d5016\"/><stop offset=\"100%\" stop-color=\"#1a3008\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"mod\" x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\">\n"
"      <stop offset=\"0%\" stop-color=\"#2563eb\"/><stop offset=\"100%\" stop-color=\"#1d4ed8\"/>\n"
"    </linearGradient>\n"
"    <filter id=\"shadow\"><feDropShadow dx=\"2\" dy=\"2\" stdDeviation=\"3\" flood-opacity=\"0.15\"/></filter>\n"
"    <marker id=\"arr\" markerWidth=\"10\" markerHeight=\"7\" refX=\"9\" refY=\"3.5\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 10 3.5, 0 7\" fill=\"#555\"/>\n"
"    </marker>\n"
"  </defs>\n"
"\n"
"  <rect width=\"900\" height=\"520\" rx=\"12\" fill=\"#f8fafc\" stroke=\"#cbd5e1\" stroke-width=\"1.5\"/>\n"
"\n"
"  <!-- Raspberry Pi -->\n"
"  <rect x=\"280\" y=\"40\" width=\"340\" height=\"440\" rx=\"14\" fill=\"url(#pi)\" filter=\"url(#shadow)\"/>\n"
"  <text x=\"450\" y=\"75\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"18\" font-weight=\"bold\">Raspberry Pi 4 Model B</text>\n"
"  <line x1=\"330\" y1=\"90\" x2=\"570\" y2=\"90\" stroke=\"#4ade80\" stroke-width=\"1\" opacity=\"0.4\"/>\n"
"\n"
"  <g fill=\"#e2e8f0\" font-size=\"11\">\n"
"    <text x=\"310\" y=\"125\">GPIO17 (Pin 11) — SCL ────────▶</text>\n"
"    <text x=\"310\" y=\"155\">GPIO18 (Pin 12) — SDA ────────▶</text>\n"
"    <text x=\"310\" y=\"190\">GPIO12 (Pin 32) — Trig ───────▶</text>\n"
"    <text x=\"310\" y=\"220\">GPIO6  (Pin 31) — Echo ◀──────</text>\n"
"    <text x=\"310\" y=\"255\">GPIO19 (Pin 35) — PWM ────────▶</text>\n"
"    <text x=\"310\" y=\"285\">GPIO21 (Pin 40) — I/O ────────▶</text>\n"
"  </g>\n"
"\n"
"  <g fill=\"#94a3b8\" font-size=\"10\">\n"
"    <text x=\"310\" y=\"330\">3.3V (Pin 1/17) ——— PCF8591 / 蜂鸣器 VCC</text>\n"
"    <text x=\"310\" y=\"355\">5V   (Pin 2/4)  ——— HC-SR04 / 舵机 VCC</text>\n"
"    <text x=\"310\" y=\"380\">GND  (Pin 6/9/14/20/30/34/39) ——— 全部模块</text>\n"
"  </g>\n"
"\n"
"  <rect x=\"310\" y=\"400\" width=\"280\" height=\"30\" rx=\"6\" fill=\"#4ade80\" opacity=\"0.2\"/>\n"
"  <text x=\"450\" y=\"420\" text-anchor=\"middle\" fill=\"#4ade80\" font-size=\"12\">wiringPiSetupGpio() + wiringPiI2C + softPwm</text>\n"
"  <text x=\"450\" y=\"445\" text-anchor=\"middle\" fill=\"#94a3b8\" font-size=\"10\">统一硬件抽象层</text>\n"
"\n"
"  <!-- 左侧模块 -->\n"
"  <g filter=\"url(#shadow)\">\n"
"    <rect x=\"30\" y=\"55\" width=\"180\" height=\"80\" rx=\"8\" fill=\"url(#mod)\"/>\n"
"    <text x=\"120\" y=\"85\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"13\" font-weight=\"bold\">PCF8591 ADC</text>\n"
"    <text x=\"120\" y=\"105\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">I2C 地址: 0x48</text>\n"
"    <text x=\"120\" y=\"122\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">光敏电阻 → AIN0</text>\n"
"  </g>\n"
"\n"
"  <g filter=\"url(#shadow)\">\n"
"    <rect x=\"30\" y=\"170\" width=\"180\" height=\"80\" rx=\"8\" fill=\"url(#mod)\"/>\n"
"    <text x=\"120\" y=\"200\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"13\" font-weight=\"bold\">HC-SR04 超声波</text>\n"
"    <text x=\"120\" y=\"220\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">40kHz 测距</text>\n"
"    <text x=\"120\" y=\"237\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">Trig/Echo (分压)</text>\n"
"  </g>\n"
"\n"
"  <g filter=\"url(#shadow)\">\n"
"    <rect x=\"30\" y=\"280\" width=\"180\" height=\"70\" rx=\"8\" fill=\"url(#mod)\"/>\n"
"    <text x=\"120\" y=\"308\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"13\" font-weight=\"bold\">SG90 舵机</text>\n"
"    <text x=\"120\" y=\"328\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">PWM 20ms 周期</text>\n"
"  </g>\n"
"\n"
"  <g filter=\"url(#shadow)\">\n"
"    <rect x=\"30\" y=\"375\" width=\"180\" height=\"65\" rx=\"8\" fill=\"url(#mod)\"/>\n"
"    <text x=\"120\" y=\"400\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"13\" font-weight=\"bold\">有源蜂鸣器</text>\n"
"    <text x=\"120\" y=\"420\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">高电平触发</text>\n"
"  </g>\n"
"\n"
"  <!-- 连接线 -->\n"
"  <g stroke=\"#94a3b8\" stroke-width=\"1.5\" fill=\"none\" marker-end=\"url(#arr)\">\n"
"    <path d=\"M210,95 C240,95 250,95 280,113\"/>\n"
"    <path d=\"M210,210 C240,210 250,200 280,205\"/>\n"
"    <path d=\"M210,315 C240,315 250,250 280,250\"/>\n"
"    <path d=\"M210,407 C240,407 250,280 280,280\"/>\n"
"  </g>\n"
"\n"
"  <!-- 右侧说明 -->\n"
"  <g font-size=\"10\" fill=\"#64748b\">\n"
"    <text x=\"650\" y=\"130\">⚡ HC-SR04 Echo 须经</text>\n"
"    <text x=\"660\" y=\"148\">2.2kΩ+3.3kΩ 分压</text>\n"
"    <text x=\"660\" y=\"166\">(5V→3.0V)</text>\n"
"    <text x=\"650\" y=\"210\">🔧 大舵机(MG996R)</text>\n"
"    <text x=\"660\" y=\"228\">建议独立 5V/2A 供电</text>\n"
"    <text x=\"650\" y=\"280\">📝 编译: gcc -lwiringPi</text>\n"
"    <text x=\"650\" y=\"310\">📝 运行: sudo ./程序</text>\n"
"  </g>\n"
"</svg>\n";

static const char SVG_FLOW[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 600 900\"\n"
"     font-family=\"SimHei,Heiti SC,sans-serif\" font-size=\"12\">\n"
"  <defs>\n"
"    <linearGradient id=\"start\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#1e40af\"/><stop offset=\"100%\" stop-color=\"#3b82f6\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"proc\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#2563eb\"/><stop offset=\"100%\" stop-color=\"#60a5fa\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"dec\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#ea580c\"/><stop offset=\"100%\" stop-color=\"#f97316\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"day\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#eab308\"/><stop offset=\"100%\" stop-color=\"#facc15\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"night\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#7c3aed\"/><stop offset=\"100%\" stop-color=\"#a78bfa\"/>\n"
"    </linearGradient>\n"
"    <filter id=\"sh\"><feDropShadow dx=\"1\" dy=\"2\" stdDeviation=\"2\" flood-opacity=\"0.12\"/></filter>\n"
"    <marker id=\"arr\" markerWidth=\"8\" markerHeight=\"6\" refX=\"7\" refY=\"3\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 8 3, 0 6\" fill=\"#475569\"/>\n"
"    </marker>\n"
"  </defs>\n"
"\n"
"  <rect width=\"600\" height=\"900\" rx=\"10\" fill=\"#f1f5f9\"/>\n"
"\n"
"  <!-- START -->\n"
"  <rect x=\"200\" y=\"20\" width=\"200\" height=\"40\" rx=\"20\" fill=\"url(#start)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"300\" y=\"45\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"13\" font-weight=\"bold\">程序启动</text>\n"
"  <line x1=\"300\" y1=\"60\" x2=\"300\" y2=\"80\" stroke=\"#475569\" stroke-width=\"1.5\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- 硬件初始化 -->\n"
"  <rect x=\"180\" y=\"82\" width=\"240\" height=\"52\" rx=\"8\" fill=\"url(#proc)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"300\" y=\"103\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"12\" font-weight=\"bold\">硬件初始化</text>\n"
"  <text x=\"300\" y=\"122\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"10\">wiringPiSetup / pinMode / I2C / softPwm</text>\n"
"  <line x1=\"300\" y1=\"134\" x2=\"300\" y2=\"158\" stroke=\"#475569\" stroke-width=\"1.5\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- 读光照 -->\n"
"  <rect x=\"190\" y=\"160\" width=\"220\" height=\"40\" rx=\"8\" fill=\"url(#proc)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"300\" y=\"185\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"12\" font-weight=\"bold\">读取光照 light_read()</text>\n"
"  <line x1=\"300\" y1=\"200\" x2=\"300\" y2=\"224\" stroke=\"#475569\" stroke-width=\"1.5\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- 判定昼/夜 -->\n"
"  <polygon points=\"300,226 380,266 300,306 220,266\" fill=\"url(#dec)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"300\" y=\"270\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"12\" font-weight=\"bold\">白天模式?</text>\n"
"\n"
"  <!-- 是→白天 -->\n"
"  <line x1=\"220\" y1=\"266\" x2=\"120\" y2=\"266\" stroke=\"#475569\" stroke-width=\"1.5\" marker-end=\"url(#arr)\"/>\n"
"  <text x=\"165\" y=\"258\" text-anchor=\"middle\" fill=\"#16a34a\" font-size=\"10\" font-weight=\"bold\">是</text>\n"
"\n"
"  <rect x=\"30\" y=\"238\" width=\"90\" height=\"56\" rx=\"8\" fill=\"url(#day)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"75\" y=\"260\" text-anchor=\"middle\" fill=\"#422006\" font-size=\"11\" font-weight=\"bold\">☀ 白天</text>\n"
"  <text x=\"75\" y=\"278\" text-anchor=\"middle\" fill=\"#422006\" font-size=\"9\">道闸常开</text>\n"
"\n"
"  <path d=\"M75,294 L75,800 L150,800 L150,780\" stroke=\"#475569\" stroke-width=\"1.5\" fill=\"none\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- 否→夜晚状态机 -->\n"
"  <line x1=\"380\" y1=\"266\" x2=\"460\" y2=\"266\" stroke=\"#475569\" stroke-width=\"1.5\" marker-end=\"url(#arr)\"/>\n"
"  <text x=\"425\" y=\"258\" text-anchor=\"middle\" fill=\"#dc2626\" font-size=\"10\" font-weight=\"bold\">否</text>\n"
"\n"
"  <rect x=\"460\" y=\"210\" width=\"130\" height=\"112\" rx=\"8\" fill=\"url(#night)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"525\" y=\"235\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"12\" font-weight=\"bold\">🌙 夜晚状态机</text>\n"
"  <g font-size=\"9\" fill=\"#d1d5db\">\n"
"    <text x=\"525\" y=\"255\" text-anchor=\"middle\">NIGHT_WAIT</text>\n"
"    <text x=\"525\" y=\"270\" text-anchor=\"middle\">    ↓</text>\n"
"    <text x=\"525\" y=\"285\" text-anchor=\"middle\">NIGHT_CAR_HERE</text>\n"
"    <text x=\"525\" y=\"300\" text-anchor=\"middle\">    ↓</text>\n"
"    <text x=\"525\" y=\"315\" text-anchor=\"middle\">NIGHT_CLOSING</text>\n"
"  </g>\n"
"\n"
"  <path d=\"M525,322 L525,330 L470,330 L470,350\" stroke=\"#475569\" stroke-width=\"1.5\" fill=\"none\"/>\n"
"\n"
"  <!-- 延时+循环 -->\n"
"  <path d=\"M120,780 L300,780\" stroke=\"#475569\" stroke-width=\"1.5\" fill=\"none\" marker-end=\"url(#arr)\"/>\n"
"  <path d=\"M525,350 L560,350 L560,400 L200,400 L200,780\" stroke=\"#475569\" stroke-width=\"1.5\" fill=\"none\"/>\n"
"\n"
"  <rect x=\"200\" y=\"770\" width=\"200\" height=\"40\" rx=\"8\" fill=\"url(#proc)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"300\" y=\"795\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"12\" font-weight=\"bold\">delay(150~300ms) 循环</text>\n"
"\n"
"  <path d=\"M300,810 L300,840 L560,840 L560,490 L300,490 L300,160\" stroke=\"#475569\" stroke-width=\"1.5\" fill=\"none\" marker-end=\"url(#arr)\" stroke-dasharray=\"6,4\"/>\n"
"  <text x=\"570\" y=\"620\" fill=\"#64748b\" font-size=\"10\">循环</text>\n"
"\n"
"  <!-- 图例 -->\n"
"  <g transform=\"translate(30,850)\">\n"
"    <rect x=\"0\" y=\"0\" width=\"160\" height=\"40\" rx=\"6\" fill=\"#fff\" stroke=\"#cbd5e1\" stroke-width=\"1\"/>\n"
"    <rect x=\"10\" y=\"10\" width=\"24\" height=\"16\" rx=\"4\" fill=\"url(#proc)\"/>\n"
"    <text x=\"42\" y=\"23\" font-size=\"9\" fill=\"#475569\">处理步骤</text>\n"
"    <polygon points=\"90,18 102,18 96,28\" fill=\"url(#dec)\"/>\n"
"    <text x=\"110\" y=\"23\" font-size=\"9\" fill=\"#475569\">条件判断</text>\n"
"  </g>\n"
"</svg>\n";

static const char SVG_FSM[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 800 600\"\n"
"     font-family=\"SimHei,Heiti SC,sans-serif\" font-size=\"12\">\n"
"  <defs>\n"
"    <filter id=\"sh\"><feDropShadow dx=\"2\" dy=\"3\" stdDeviation=\"3\" flood-opacity=\"0.1\"/></filter>\n"
"    <marker id=\"arrB\" markerWidth=\"10\" markerHeight=\"8\" refX=\"9\" refY=\"4\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 10 4, 0 8\" fill=\"#1e40af\"/>\n"
"    </marker>\n"
"    <marker id=\"arrR\" markerWidth=\"10\" markerHeight=\"8\" refX=\"9\" refY=\"4\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 10 4, 0 8\" fill=\"#dc2626\"/>\n"
"    </marker>\n"
"    <marker id=\"arrO\" markerWidth=\"10\" markerHeight=\"8\" refX=\"9\" refY=\"4\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 10 4, 0 8\" fill=\"#ea580c\"/>\n"
"    </marker>\n"
"    <marker id=\"arrG\" markerWidth=\"10\" markerHeight=\"8\" refX=\"9\" refY=\"4\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 10 4, 0 8\" fill=\"#16a34a\"/>\n"
"    </marker>\n"
"  </defs>\n"
"\n"
"  <rect width=\"800\" height=\"600\" rx=\"12\" fill=\"#f8fafc\"/>\n"
"  <text x=\"400\" y=\"30\" text-anchor=\"middle\" font-size=\"18\" font-weight=\"bold\" fill=\"#1e293b\">夜晚模式状态机 (FSM)</text>\n"
"  <text x=\"400\" y=\"50\" text-anchor=\"middle\" font-size=\"11\" fill=\"#94a3b8\">State Machine — Night Mode</text>\n"
"\n"
"  <!-- State 1: NIGHT_WAIT -->\n"
"  <rect x=\"80\" y=\"80\" width=\"220\" height=\"100\" rx=\"12\" fill=\"#eff6ff\" stroke=\"#3b82f6\" stroke-width=\"2.5\" filter=\"url(#sh)\"/>\n"
"  <text x=\"190\" y=\"110\" text-anchor=\"middle\" font-size=\"14\" font-weight=\"bold\" fill=\"#1e40af\">NIGHT_WAIT</text>\n"
"  <text x=\"190\" y=\"130\" text-anchor=\"middle\" font-size=\"11\" fill=\"#475569\">等待车辆到达</text>\n"
"  <line x1=\"105\" y1=\"140\" x2=\"275\" y2=\"140\" stroke=\"#93c5fd\" stroke-width=\"0.8\"/>\n"
"  <text x=\"190\" y=\"156\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">栏杆: 关闭 (0°)</text>\n"
"  <text x=\"190\" y=\"172\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">蜂鸣器: 静音</text>\n"
"\n"
"  <!-- State 2: NIGHT_CAR_HERE -->\n"
"  <rect x=\"500\" y=\"80\" width=\"220\" height=\"100\" rx=\"12\" fill=\"#fef2f2\" stroke=\"#ef4444\" stroke-width=\"2.5\" filter=\"url(#sh)\"/>\n"
"  <text x=\"610\" y=\"110\" text-anchor=\"middle\" font-size=\"14\" font-weight=\"bold\" fill=\"#b91c1c\">NIGHT_CAR_HERE</text>\n"
"  <text x=\"610\" y=\"130\" text-anchor=\"middle\" font-size=\"11\" fill=\"#475569\">车辆在通道中</text>\n"
"  <line x1=\"525\" y1=\"140\" x2=\"695\" y2=\"140\" stroke=\"#fca5a5\" stroke-width=\"0.8\"/>\n"
"  <text x=\"610\" y=\"156\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">栏杆: 打开 (90°)</text>\n"
"  <text x=\"610\" y=\"172\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">蜂鸣器: 3声短鸣</text>\n"
"\n"
"  <!-- State 3: NIGHT_CLOSING -->\n"
"  <rect x=\"500\" y=\"400\" width=\"220\" height=\"100\" rx=\"12\" fill=\"#fff7ed\" stroke=\"#f97316\" stroke-width=\"2.5\" filter=\"url(#sh)\"/>\n"
"  <text x=\"610\" y=\"430\" text-anchor=\"middle\" font-size=\"14\" font-weight=\"bold\" fill=\"#9a3412\">NIGHT_CLOSING</text>\n"
"  <text x=\"610\" y=\"450\" text-anchor=\"middle\" font-size=\"11\" fill=\"#475569\">关门倒计时中</text>\n"
"  <line x1=\"525\" y1=\"460\" x2=\"695\" y2=\"460\" stroke=\"#fdba74\" stroke-width=\"0.8\"/>\n"
"  <text x=\"610\" y=\"476\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">栏杆: 仍打开</text>\n"
"  <text x=\"610\" y=\"492\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">显示剩余秒数</text>\n"
"\n"
"  <!-- State 4: NIGHT_COOLDOWN -->\n"
"  <rect x=\"80\" y=\"400\" width=\"220\" height=\"100\" rx=\"12\" fill=\"#f0fdf4\" stroke=\"#22c55e\" stroke-width=\"2.5\" filter=\"url(#sh)\"/>\n"
"  <text x=\"190\" y=\"430\" text-anchor=\"middle\" font-size=\"14\" font-weight=\"bold\" fill=\"#166534\">NIGHT_COOLDOWN</text>\n"
"  <text x=\"190\" y=\"450\" text-anchor=\"middle\" font-size=\"11\" fill=\"#475569\">冷却期</text>\n"
"  <line x1=\"105\" y1=\"460\" x2=\"275\" y2=\"460\" stroke=\"#86efac\" stroke-width=\"0.8\"/>\n"
"  <text x=\"190\" y=\"476\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">栏杆: 关闭 (0°)</text>\n"
"  <text x=\"190\" y=\"492\" text-anchor=\"middle\" font-size=\"10\" fill=\"#64748b\">蜂鸣器: 1声确认</text>\n"
"\n"
"  <!-- WAIT → CAR_HERE -->\n"
"  <path d=\"M300,130 L500,130\" stroke=\"#1e40af\" stroke-width=\"2.5\" fill=\"none\" marker-end=\"url(#arrB)\"/>\n"
"  <rect x=\"330\" y=\"108\" width=\"140\" height=\"24\" rx=\"5\" fill=\"#dbeafe\"/>\n"
"  <text x=\"400\" y=\"124\" text-anchor=\"middle\" font-size=\"10\" fill=\"#1e40af\" font-weight=\"bold\">car_detected() = True</text>\n"
"  <text x=\"400\" y=\"140\" text-anchor=\"middle\" font-size=\"9\" fill=\"#64748b\">(40ms内两次均 &lt; 80cm)</text>\n"
"\n"
"  <!-- CAR_HERE → CLOSING -->\n"
"  <path d=\"M610,180 L610,400\" stroke=\"#dc2626\" stroke-width=\"2.5\" fill=\"none\" marker-end=\"url(#arrR)\"/>\n"
"  <rect x=\"622\" y=\"260\" width=\"130\" height=\"45\" rx=\"5\" fill=\"#fee2e2\"/>\n"
"  <text x=\"687\" y=\"277\" text-anchor=\"middle\" font-size=\"10\" fill=\"#dc2626\" font-weight=\"bold\">car_detected() = False</text>\n"
"  <text x=\"687\" y=\"293\" text-anchor=\"middle\" font-size=\"9\" fill=\"#64748b\">车已离开</text>\n"
"\n"
"  <!-- CLOSING → COOLDOWN -->\n"
"  <path d=\"M500,450 L300,450\" stroke=\"#ea580c\" stroke-width=\"2.5\" fill=\"none\" marker-end=\"url(#arrO)\"/>\n"
"  <rect x=\"330\" y=\"428\" width=\"140\" height=\"24\" rx=\"5\" fill=\"#ffedd5\"/>\n"
"  <text x=\"400\" y=\"444\" text-anchor=\"middle\" font-size=\"10\" fill=\"#ea580c\" font-weight=\"bold\">elapsed ≥ 3 秒</text>\n"
"\n"
"  <!-- COOLDOWN → WAIT -->\n"
"  <path d=\"M190,400 L190,180\" stroke=\"#16a34a\" stroke-width=\"2.5\" fill=\"none\" marker-end=\"url(#arrG)\"/>\n"
"  <rect x=\"60\" y=\"260\" width=\"110\" height=\"24\" rx=\"5\" fill=\"#dcfce7\"/>\n"
"  <text x=\"115\" y=\"276\" text-anchor=\"middle\" font-size=\"10\" fill=\"#16a34a\" font-weight=\"bold\">elapsed ≥ 3 秒</text>\n"
"  <text x=\"115\" y=\"292\" text-anchor=\"middle\" font-size=\"9\" fill=\"#64748b\">冷却结束</text>\n"
"\n"
"  <!-- 图例 -->\n"
"  <g transform=\"translate(80,530)\">\n"
"    <rect width=\"640\" height=\"50\" rx=\"8\" fill=\"#fff\" stroke=\"#e2e8f0\" stroke-width=\"1\"/>\n"
"    <g font-size=\"10\" fill=\"#475569\">\n"
"      <rect x=\"15\" y=\"15\" width=\"40\" height=\"18\" rx=\"4\" fill=\"#eff6ff\" stroke=\"#3b82f6\" stroke-width=\"1\"/>\n"
"      <text x=\"65\" y=\"28\">状态节点</text>\n"
"      <line x1=\"130\" y1=\"24\" x2=\"170\" y2=\"24\" stroke=\"#1e40af\" stroke-width=\"2\" marker-end=\"url(#arrB)\"/>\n"
"      <text x=\"180\" y=\"28\">转换路径</text>\n"
"      <text x=\"270\" y=\"28\" fill=\"#64748b\">| 触发条件写在路径上</text>\n"
"    </g>\n"
"  </g>\n"
"</svg>\n";

static const char SVG_ARCH[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 800 500\"\n"
"     font-family=\"SimHei,Heiti SC,sans-serif\" font-size=\"13\">\n"
"  <defs>\n"
"    <marker id=\"arr\" markerWidth=\"8\" markerHeight=\"6\" refX=\"7\" refY=\"3\" orient=\"auto\">\n"
"      <polygon points=\"0 0, 8 3, 0 6\" fill=\"#475569\"/>\n"
"    </marker>\n"
"    <filter id=\"sh\"><feDropShadow dx=\"1\" dy=\"2\" stdDeviation=\"2\" flood-opacity=\"0.1\"/></filter>\n"
"    <linearGradient id=\"l1\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#1e40af\"/><stop offset=\"100%\" stop-color=\"#3b82f6\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"l2\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#2563eb\"/><stop offset=\"100%\" stop-color=\"#60a5fa\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"l3\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#0d9488\"/><stop offset=\"100%\" stop-color=\"#14b8a6\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"l4\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#6366f1\"/><stop offset=\"100%\" stop-color=\"#818cf8\"/>\n"
"    </linearGradient>\n"
"    <linearGradient id=\"l5\" x1=\"0\" y1=\"0\" x2=\"1\" y2=\"0\">\n"
"      <stop offset=\"0%\" stop-color=\"#475569\"/><stop offset=\"100%\" stop-color=\"#64748b\"/>\n"
"    </linearGradient>\n"
"  </defs>\n"
"\n"
"  <rect width=\"800\" height=\"500\" rx=\"12\" fill=\"#f8fafc\"/>\n"
"\n"
"  <!-- Layer 1: 应用层 -->\n"
"  <rect x=\"60\" y=\"30\" width=\"680\" height=\"80\" rx=\"10\" fill=\"url(#l1)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"400\" y=\"58\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"16\" font-weight=\"bold\">应用层 — main()</text>\n"
"  <text x=\"400\" y=\"82\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"12\">昼夜模式管理 │ 状态机调度 (FSM) │ 控制台输出 (Unicode)</text>\n"
"  <text x=\"400\" y=\"100\" text-anchor=\"middle\" fill=\"#93c5fd\" font-size=\"10\">application logic</text>\n"
"\n"
"  <line x1=\"400\" y1=\"110\" x2=\"400\" y2=\"130\" stroke=\"#475569\" stroke-width=\"2\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- Layer 2: 功能模块层 -->\n"
"  <rect x=\"60\" y=\"135\" width=\"680\" height=\"80\" rx=\"10\" fill=\"url(#l2)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"400\" y=\"163\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"16\" font-weight=\"bold\">功能模块层</text>\n"
"  <text x=\"400\" y=\"187\" text-anchor=\"middle\" fill=\"#bfdbfe\" font-size=\"12\">ultrasonic_read() │ light_read() │ servo_set() │ buzzer_beep() │ print_bar()</text>\n"
"  <text x=\"400\" y=\"205\" text-anchor=\"middle\" fill=\"#93c5fd\" font-size=\"10\">functional modules</text>\n"
"\n"
"  <line x1=\"400\" y1=\"215\" x2=\"400\" y2=\"235\" stroke=\"#475569\" stroke-width=\"2\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- Layer 3: WiringPi HAL -->\n"
"  <rect x=\"60\" y=\"240\" width=\"680\" height=\"80\" rx=\"10\" fill=\"url(#l3)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"400\" y=\"268\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"16\" font-weight=\"bold\">WiringPi 硬件抽象层 (HAL) ⭐</text>\n"
"  <text x=\"400\" y=\"292\" text-anchor=\"middle\" fill=\"#ccfbf1\" font-size=\"12\">digitalWrite/Read │ wiringPiI2C │ softPwm │ pinMode │ micros │ delay</text>\n"
"  <text x=\"400\" y=\"310\" text-anchor=\"middle\" fill=\"#99f6e4\" font-size=\"10\">hardware abstraction layer — 统一 API，一行 wiringPiSetupGpio() 初始化全部</text>\n"
"\n"
"  <line x1=\"400\" y1=\"320\" x2=\"400\" y2=\"340\" stroke=\"#475569\" stroke-width=\"2\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- Layer 4: Linux 驱动层 -->\n"
"  <rect x=\"60\" y=\"345\" width=\"680\" height=\"65\" rx=\"10\" fill=\"url(#l4)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"400\" y=\"373\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"16\" font-weight=\"bold\">Linux 内核驱动层</text>\n"
"  <text x=\"400\" y=\"395\" text-anchor=\"middle\" fill=\"#c7d2fe\" font-size=\"12\">GPIO 子系统 │ I2C (i2c-gpio overlay) │ softPwm 线程 │ BCM2711 时钟</text>\n"
"  <text x=\"400\" y=\"408\" text-anchor=\"middle\" fill=\"#a5b4fc\" font-size=\"10\">kernel drivers</text>\n"
"\n"
"  <line x1=\"400\" y1=\"410\" x2=\"400\" y2=\"425\" stroke=\"#475569\" stroke-width=\"2\" marker-end=\"url(#arr)\"/>\n"
"\n"
"  <!-- Layer 5: 硬件层 -->\n"
"  <rect x=\"60\" y=\"430\" width=\"680\" height=\"55\" rx=\"10\" fill=\"url(#l5)\" filter=\"url(#sh)\"/>\n"
"  <text x=\"400\" y=\"458\" text-anchor=\"middle\" fill=\"#fff\" font-size=\"16\" font-weight=\"bold\">BCM2711 硬件层</text>\n"
"  <text x=\"400\" y=\"478\" text-anchor=\"middle\" fill=\"#cbd5e1\" font-size=\"12\">GPIO 寄存器 (0xFE200000) │ I2C 控制器 │ PWM 定时器 │ HC-SR04 / PCF8591 / SG90</text>\n"
"</svg>\n";

/* ==================== 主程序 ==================== */

typedef struct {
    const char *name;
    const char *svg;
} diagram_t;

int main(void) {
    /* 创建输出目录 */
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", OUT_DIR);
    system(mkdir_cmd);

    printf("生成流程图...\n");

    diagram_t diagrams[] = {
        {"fig1-1_hardware",     SVG_HW},
        {"fig3-1_flowchart",    SVG_FLOW},
        {"fig3-2_statemachine", SVG_FSM},
        {"fig3-3_architecture", SVG_ARCH},
    };

    for (int i = 0; i < 4; i++) {
        generate(diagrams[i].name, diagrams[i].svg);
    }

    printf("\n全部完成，保存于 %s/\n", OUT_DIR);
    return 0;
}
