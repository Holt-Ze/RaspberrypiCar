/*
 * 生成课程设计报告 .docx (v2.0 — WiringPi重构版)
 *
 * 通过组装 OOXML 文件并打包为 .docx (ZIP) 实现，无需外部库依赖。
 *
 * 格式: A4, 左3cm, 右1.5cm, 宋体正文12pt, 固定行距22pt
 *
 * 编译: gcc -o generate_report generate_report.c
 * 运行: ./generate_report
 *
 * 依赖: zip 命令 (系统自带), 图表 PNG 须由 generate_diagrams 先生成
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* ==================== 路径配置 ==================== */
#define OUT_DOCX    "/home/holt/Cproject/智能停车场道闸系统_课程设计报告.docx"
#define DIAGRAM_DIR "/home/holt/Cproject/diagrams"
#define TMP_DIR     "/tmp/docx_build_XXXXXX"

/* ==================== OOXML 常量 (单位: twip = 1/1440 inch, half-pt) ==================== */
#define MARGIN_LEFT   "1701"   /* 3.0 cm */
#define MARGIN_RIGHT  "850"    /* 1.5 cm */
#define MARGIN_TOP    "1440"   /* 2.54 cm */
#define MARGIN_BOTTOM "1440"
#define PAGE_W        "11906"  /* A4 21.0 cm */
#define PAGE_H        "16838"  /* A4 29.7 cm */

/* 字体大小 (half-points): 12pt=24, 14pt=28, 10.5pt=21, 16pt=32, 18pt=36, 22pt=44 */
#define SZ_BODY  "24"
#define SZ_H1    "28"
#define SZ_SMALL "21"
#define SZ_TOC   "32"
#define SZ_COVER "44"

/* 行距 (twips = pt*20): 22pt=440, 36pt=720, 44pt=880 */
#define LS_BODY "440"
#define LS_36   "720"
#define LS_44   "880"

#define FONT "宋体"
#define FONT_HEI "黑体"

/* 文图路径名称 */
static const char *img_names[] = {
    "fig1-1_hardware.png",
    "fig3-1_flowchart.png",
    "fig3-2_statemachine.png",
    "fig3-3_architecture.png",
};
#define N_IMAGES 4

/* ==================== 工具函数 ==================== */

static FILE *xml_fp = NULL;  /* 当前正在写的 XML 文件 */

static int write_raw(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) { fprintf(stderr, "  错误: 无法创建 %s\n", path); return -1; }
    fputs(content, f);
    fclose(f);
    return 0;
}

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) { fprintf(stderr, "  警告: 未找到 %s\n", src); return -1; }
    FILE *out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(in); fclose(out);
    return 0;
}

static void mkdir_p(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path);
    system(cmd);
}

/* XML 转义: & < > " ' → 实体引用 */
static void xml_esc(FILE *f, const char *s) {
    for (; *s; s++) {
        switch (*s) {
        case '&':  fputs("&amp;", f); break;
        case '<':  fputs("&lt;", f); break;
        case '>':  fputs("&gt;", f); break;
        case '"':  fputs("&quot;", f); break;
        case '\'': fputs("&apos;", f); break;
        default:   fputc(*s, f); break;
        }
    }
}

/* ==================== OOXML 结构生成 ==================== */

/* ─── [Content_Types].xml ─── */
static void write_content_types(const char *dir) {
    char path[512];
    snprintf(path, sizeof(path), "%s/[Content_Types].xml", dir);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
          "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">\n"
          "  <Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>\n"
          "  <Default Extension=\"xml\" ContentType=\"application/xml\"/>\n"
          "  <Default Extension=\"png\" ContentType=\"image/png\"/>\n"
          "  <Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>\n"
          "  <Override PartName=\"/word/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>\n"
          "</Types>\n", f);
    fclose(f);
}

/* ─── _rels/.rels ─── */
static void write_rels(const char *dir) {
    char path[512];
    snprintf(path, sizeof(path), "%s/_rels/.rels", dir);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
          "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n"
          "  <Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>\n"
          "</Relationships>\n", f);
    fclose(f);
}

/* ─── word/_rels/document.xml.rels ─── */
static void write_doc_rels(const char *dir) {
    char path[512];
    snprintf(path, sizeof(path), "%s/word/_rels/document.xml.rels", dir);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
               "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">\n");
    for (int i = 0; i < N_IMAGES; i++)
        fprintf(f, "  <Relationship Id=\"rIdImg%d\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"media/%s\"/>\n",
                i + 1, img_names[i]);
    fputs("  <Relationship Id=\"rIdStyles\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>\n"
          "</Relationships>\n", f);
    fclose(f);
}

/* ─── word/styles.xml (最小样式表) ─── */
static void write_styles(const char *dir) {
    char path[512];
    snprintf(path, sizeof(path), "%s/word/styles.xml", dir);
    FILE *f = fopen(path, "w");
    if (!f) return;
    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
          "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">\n"
          "  <w:style w:type=\"paragraph\" w:styleId=\"Normal\">\n"
          "    <w:name w:val=\"Normal\"/>\n"
          "    <w:rPr>\n"
          "      <w:rFonts w:eastAsia=\"宋体\" w:ascii=\"宋体\" w:hAnsi=\"宋体\"/>\n"
          "      <w:sz w:val=\"24\"/>\n"
          "    </w:rPr>\n"
          "  </w:style>\n"
          "</w:styles>\n", f);
    fclose(f);
}

/* ==================== document.xml 构建辅助 ==================== */

/* 文档 XML 声明 + 根元素 */
static void doc_begin(FILE *f) {
    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
          "<w:document\n"
          "  xmlns:wpc=\"http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas\"\n"
          "  xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"\n"
          "  xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n"
          "  xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"\n"
          "  xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"\n"
          "  xmlns:v=\"urn:schemas-microsoft-com:vml\"\n"
          "  xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\"\n"
          "  xmlns:w10=\"urn:schemas-microsoft-com:office:word\"\n"
          "  xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"\n"
          "  xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\">\n"
          "<w:body>\n", f);
}

static void doc_end(FILE *f) {
    fputs("  <w:sectPr>\n"
          "    <w:pgSz w:w=\"" PAGE_W "\" w:h=\"" PAGE_H "\"/>\n"
          "    <w:pgMar w:left=\"" MARGIN_LEFT "\" w:right=\"" MARGIN_RIGHT "\""
                        " w:top=\"" MARGIN_TOP "\" w:bottom=\"" MARGIN_BOTTOM "\"/>\n"
          "  </w:sectPr>\n"
          "</w:body>\n"
          "</w:document>\n", f);
}

/* ── 段落 / 文本 ── */

/* 打开一个段落 */
static void p_open(FILE *f) { fputs("  <w:p>\n", f); }

/* 关闭段落 */
static void p_close(FILE *f) { fputs("  </w:p>\n", f); }

/* 段落属性 */
static void pPr(FILE *f, const char *font, const char *sz, const char *jc,
                const char *line, const char *before, const char *after,
                const char *indent, int bold) {
    fprintf(f, "    <w:pPr>\n"
               "      <w:rPr>\n"
               "        <w:rFonts w:eastAsia=\"%s\" w:ascii=\"%s\" w:hAnsi=\"%s\"/>\n"
               "        <w:sz w:val=\"%s\"/>\n"
               "        <w:b w:val=\"%d\"/>\n"
               "      </w:rPr>\n",
            font, font, font, sz, bold);
    if (line)    fprintf(f, "      <w:spacing w:line=\"%s\" w:lineRule=\"exact\" w:before=\"%s\" w:after=\"%s\"/>\n", line, before ? before : "0", after ? after : "0");
    if (jc)      fprintf(f, "      <w:jc w:val=\"%s\"/>\n", jc);
    if (indent)  fprintf(f, "      <w:ind w:firstLine=\"%s\"/>\n", indent);
    fputs("    </w:pPr>\n", f);
}

/* 纯文本 run */
static void run_text(FILE *f, const char *font, const char *sz,
                     const char *text, int bold) {
    fprintf(f, "    <w:r>\n"
               "      <w:rPr>\n"
               "        <w:rFonts w:eastAsia=\"%s\" w:ascii=\"%s\" w:hAnsi=\"%s\"/>\n"
               "        <w:sz w:val=\"%s\"/>\n"
               "        <w:b w:val=\"%d\"/>\n"
               "      </w:rPr>\n"
               "      <w:t xml:space=\"preserve\">", font, font, font, sz, bold);
    xml_esc(f, text);
    fputs("</w:t>\n"
          "    </w:r>\n", f);
}

/* 添加一个完整段落 */
static void add_para(FILE *f, const char *text, const char *font, const char *sz,
                     const char *jc, const char *line, const char *before,
                     const char *after, const char *indent, int bold) {
    p_open(f);
    pPr(f, font, sz, jc, line, before, after, indent, bold);
    run_text(f, font, sz, text, bold);
    p_close(f);
}

/* 正文段落 (宋体12pt, 两端对齐, 行距22pt, 首行缩进2字符) */
static void body(FILE *f, const char *text) {
    add_para(f, text, FONT, SZ_BODY, "both", LS_BODY, "0", "0", "480", 0);
}

/* 正文无缩进 */
static void body_ni(FILE *f, const char *text) {
    add_para(f, text, FONT, SZ_BODY, "both", LS_BODY, "0", "0", NULL, 0);
}

/* 章节标题 (宋体14pt加粗) */
static void heading(FILE *f, const char *text, int level) {
    if (level == 0)
        add_para(f, text, FONT_HEI, "36", "center", LS_44, "240", "240", NULL, 1);
    else
        add_para(f, text, FONT, SZ_H1, "left", LS_BODY, "120", "120", NULL, 1);
}

/* 图名 (五号宋体, 居中) */
static void fig_caption(FILE *f, const char *text) {
    add_para(f, text, FONT, SZ_SMALL, "center", LS_BODY, "40", "120", NULL, 0);
}

/* 表名 (五号宋体加粗, 居中) */
static void table_caption(FILE *f, const char *text) {
    add_para(f, text, FONT, SZ_SMALL, "center", LS_BODY, "120", "40", NULL, 1);
}

/* 嵌入图片 */
static void embed_image(FILE *f, int img_idx, double width_inches) {
    /* rIdImg1, rIdImg2, ... */
    long cx = (long)(width_inches * 914400); /* EMU per inch */
    long cy = cx * 2 / 3;                   /* 4:3 近似 */

    fprintf(f, "  <w:p>\n"
               "    <w:pPr><w:jc w:val=\"center\"/></w:pPr>\n"
               "    <w:r>\n"
               "      <w:drawing>\n"
               "        <wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">\n"
               "          <wp:extent cx=\"%ld\" cy=\"%ld\"/>\n"
               "          <wp:docPr id=\"%d\" name=\"Picture %d\"/>\n"
               "          <a:graphic xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">\n"
               "            <a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">\n"
               "              <pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">\n"
               "                <pic:nvPicPr>\n"
               "                  <pic:cNvPr id=\"0\" name=\"Picture %d\"/>\n"
               "                  <pic:cNvPicPr/>\n"
               "                </pic:nvPicPr>\n"
               "                <pic:blipFill>\n"
               "                  <a:blip r:embed=\"rIdImg%d\"/>\n"
               "                  <a:stretch><a:fillRect/></a:stretch>\n"
               "                </pic:blipFill>\n"
               "                <pic:spPr>\n"
               "                  <a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"%ld\" cy=\"%ld\"/></a:xfrm>\n"
               "                  <a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>\n"
               "                </pic:spPr>\n"
               "              </pic:pic>\n"
               "            </a:graphicData>\n"
               "          </a:graphic>\n"
               "        </wp:inline>\n"
               "      </w:drawing>\n"
               "    </w:r>\n"
               "  </w:p>\n",
            cx, cy, img_idx, img_idx, img_idx, img_idx, cx, cy);
}

/* 添加表格 */
static void add_table(FILE *f, const char **headers, int ncols,
                      const char ***rows, int nrows) {
    fputs("  <w:tbl>\n"
          "    <w:tblPr>\n"
          "      <w:tblStyle w:val=\"TableGrid\"/>\n"
          "      <w:jc w:val=\"center\"/>\n"
          "      <w:tblW w:w=\"9000\" w:type=\"dxa\"/>\n"
          "    </w:tblPr>\n"
          "    <w:tblGrid>\n", f);
    for (int c = 0; c < ncols; c++)
        fputs("      <w:gridCol w:w=\"1500\"/>\n", f);
    fputs("    </w:tblGrid>\n", f);

    /* 表头 */
    fputs("    <w:tr>\n", f);
    for (int c = 0; c < ncols; c++) {
        fprintf(f, "      <w:tc>\n"
                   "        <w:tcPr><w:tcW w:w=\"1500\" w:type=\"dxa\"/></w:tcPr>\n"
                   "        <w:p>\n"
                   "          <w:pPr>\n"
                   "            <w:jc w:val=\"center\"/>\n"
                   "            <w:rPr>\n"
                   "              <w:rFonts w:eastAsia=\"" FONT "\" w:ascii=\"" FONT "\" w:hAnsi=\"" FONT "\"/>\n"
                   "              <w:sz w:val=\"" SZ_SMALL "\"/>\n"
                   "              <w:b w:val=\"1\"/>\n"
                   "            </w:rPr>\n"
                   "          </w:pPr>\n"
                   "          <w:r>\n"
                   "            <w:rPr>\n"
                   "              <w:rFonts w:eastAsia=\"" FONT "\"/>\n"
                   "              <w:sz w:val=\"" SZ_SMALL "\"/>\n"
                   "              <w:b w:val=\"1\"/>\n"
                   "            </w:rPr>\n"
                   "            <w:t>");
        xml_esc(f, headers[c]);
        fputs("</w:t>\n"
              "          </w:r>\n"
              "        </w:p>\n"
              "      </w:tc>\n", f);
    }
    fputs("    </w:tr>\n", f);

    /* 数据行 */
    for (int r = 0; r < nrows; r++) {
        fputs("    <w:tr>\n", f);
        for (int c = 0; c < ncols; c++) {
            fprintf(f, "      <w:tc>\n"
                       "        <w:tcPr><w:tcW w:w=\"1500\" w:type=\"dxa\"/></w:tcPr>\n"
                       "        <w:p>\n"
                       "          <w:pPr>\n"
                       "            <w:jc w:val=\"center\"/>\n"
                       "            <w:rPr>\n"
                       "              <w:rFonts w:eastAsia=\"" FONT "\"/>\n"
                       "              <w:sz w:val=\"" SZ_SMALL "\"/>\n"
                       "            </w:rPr>\n"
                       "          </w:pPr>\n"
                       "          <w:r>\n"
                       "            <w:rPr>\n"
                       "              <w:rFonts w:eastAsia=\"" FONT "\"/>\n"
                       "              <w:sz w:val=\"" SZ_SMALL "\"/>\n"
                       "            </w:rPr>\n"
                       "            <w:t>");
            xml_esc(f, rows[r][c]);
            fputs("</w:t>\n"
                  "          </w:r>\n"
                  "        </w:p>\n"
                  "      </w:tc>\n", f);
        }
        fputs("    </w:tr>\n", f);
    }
    fputs("  </w:tbl>\n", f);
}

/* ── 空行 ── */
static void empty_line(FILE *f, const char *line_sp) {
    add_para(f, "", FONT, SZ_BODY, "left", line_sp, "0", "0", NULL, 0);
}

/* ── 分页 (section break) ── */
static void page_break(FILE *f) {
    fputs("  <w:p>\n"
          "    <w:r>\n"
          "      <w:br w:type=\"page\"/>\n"
          "    </w:r>\n"
          "  </w:p>\n", f);
}

/* ==================== 报告内容 ==================== */

static void write_document(const char *dir) {
    char path[512];
    snprintf(path, sizeof(path), "%s/word/document.xml", dir);
    FILE *f = fopen(path, "w");
    if (!f) return;

    doc_begin(f);

    /* ═══════════ 封面 ═══════════ */
    for (int i = 0; i < 3; i++) empty_line(f, LS_BODY);
    heading(f, "智能应用系统开发", 0);
    heading(f, "课程设计（实训）报告", 0);
    for (int i = 0; i < 4; i++) empty_line(f, LS_36);

    const char *cover_fields[] = {
        "课    程：智能应用系统开发",
        "课    题：基于树莓派的智能停车场道闸系统",
        "姓    名：__________",
        "学    号：__________",
        "专    业：__________",
        "班    级：__________",
        "指导教师：__________",
    };
    for (int i = 0; i < 7; i++)
        add_para(f, cover_fields[i], FONT, SZ_BODY, "center", LS_36, "0", "0", NULL, 0);

    for (int i = 0; i < 3; i++) empty_line(f, LS_36);
    add_para(f, "泰州学院信息工程学院", FONT, SZ_BODY, "center", LS_BODY, "0", "0", NULL, 0);
    page_break(f);

    /* ═══════════ 目录 ═══════════ */
    for (int i = 0; i < 2; i++) empty_line(f, LS_BODY);
    add_para(f, "目  录", FONT_HEI, SZ_TOC, "center", LS_BODY, "0", "0", NULL, 1);
    for (int i = 0; i < 2; i++) empty_line(f, "360");

    const char *toc[][2] = {
        {"1  项目背景", "1"},
        {"  1.1  项目选题与应用场景", "1"},
        {"  1.2  开发环境与硬件清单", "2"},
        {"  1.3  功能要求与性能指标", "2"},
        {"2  需求分析", "3"},
        {"  2.1  功能需求分解", "3"},
        {"  2.2  硬件模块选型分析", "4"},
        {"  2.3  项目重点与难点分析", "5"},
        {"3  程序设计", "6"},
        {"  3.1  整体软件执行流程", "6"},
        {"  3.2  核心控制逻辑：夜晚状态机设计", "7"},
        {"  3.3  模块划分与函数封装", "8"},
        {"  3.4  关键技术实现说明", "9"},
        {"4  系统性能分析与调试测试", "10"},
        {"  4.1  硬件层面分析", "10"},
        {"  4.2  软件层面分析", "11"},
        {"  4.3  系统误差分析", "12"},
        {"  4.4  测试场景与数据记录", "12"},
        {"5  结论与展望", "14"},
        {"  5.1  项目完成总结", "14"},
        {"  5.2  掌握的硬件开发技能", "14"},
        {"  5.3  存在问题与改进措施", "15"},
        {"  5.4  心得体会", "15"},
        {"参考文献", "16"},
        {"成绩评定表", "17"},
    };
    int n_toc = sizeof(toc) / sizeof(toc[0]);
    for (int i = 0; i < n_toc; i++) {
        char line[128];
        int dots = 55 - (int)strlen(toc[i][0]);
        if (dots < 2) dots = 2;
        /* 构建点线 */
        int pos = sprintf(line, "%s ", toc[i][0]);
        for (int d = 0; d < dots; d++) line[pos++] = '.';
        sprintf(line + pos, " %s", toc[i][1]);
        const char *ind = (toc[i][0][0] == ' ' && toc[i][0][1] == ' ') ? "480" : NULL;
        add_para(f, line, FONT, SZ_BODY, "left", "400", "0", "0", ind, 0);
    }
    page_break(f);

    /* ═══════════ 第1章 项目背景 ═══════════ */
    heading(f, "1  项目背景", 1);
    heading(f, "1.1  项目选题与应用场景", 2);
    body(f, "随着城市化进程加速和汽车保有量的持续增长，传统停车场的管理方式已难以满足效率和安全性的要求。居民小区和商业停车场中，夜间车辆进出需要人工值守，人力成本较高；而白天车辆频繁进出时，道闸的频繁启闭又会增加设备的机械损耗。针对这一实际问题，本项目设计并实现了一套基于树莓派的智能停车场道闸系统，通过光照传感器自动感知昼夜环境变化，并利用超声波测距实现车辆检测，从而实现停车场道闸的智能化、无人化管理。");
    body(f, "本项目模拟的停车场道闸系统区分白天和夜晚两种工作模式：白天光照充足时，道闸保持常开状态，车辆自由通行，减少道闸机械部件的无效动作；夜晚光照不足时，道闸关闭并进入监控模式，当超声波传感器检测到有车辆靠近时自动打开道闸，车辆通过后延迟关闭，并设置冷却时间防止连续误触发。系统通过蜂鸣器提供声音反馈，提醒驾驶员注意道闸状态变化。");

    heading(f, "1.2  开发环境与硬件清单", 2);
    body(f, "本项目基于树莓派 4 Model B 嵌入式平台开发，使用 C 语言编写，采用 WiringPi 库统一管理全部硬件外设，实现了代码的简洁性、可读性和可维护性。与旧版使用 sysfs、mmap、I2C ioctl 三套不同底层接口的方案相比，WiringPi 重构版代码量减少约 40%，开发效率显著提升。");

    table_caption(f, "表 1-1  开发环境");
    {
        const char *h1[] = {"项目", "说明"};
        const char *r1_1[] = {"操作系统", "Raspberry Pi OS (Debian Linux)"};
        const char *r1_2[] = {"编译器", "GCC (GNU C Compiler, C99 标准)"};
        const char *r1_3[] = {"编程语言", "C 语言"};
        const char *r1_4[] = {"硬件接口库", "WiringPi (GPIO + I2C + softPwm)"};
        const char *r1_5[] = {"调试工具", "printf 实时输出、dmesg 内核日志"};
        const char **rows1[] = {r1_1, r1_2, r1_3, r1_4, r1_5};
        add_table(f, h1, 2, rows1, 5);
    }

    table_caption(f, "表 1-2  硬件清单");
    {
        const char *h2[] = {"序号", "组件", "型号", "数量", "用途"};
        const char *r2_1[] = {"1", "主控制器", "Raspberry Pi 4 Model B", "1", "系统核心，运行控制程序"};
        const char *r2_2[] = {"2", "ADC 模块", "PCF8591", "1", "模数转换，读取光敏传感器"};
        const char *r2_3[] = {"3", "光敏传感器", "光敏电阻模块", "1", "检测环境光照，判断昼/夜"};
        const char *r2_4[] = {"4", "超声波模块", "HC-SR04", "1", "测量前方障碍物距离"};
        const char *r2_5[] = {"5", "舵机", "SG90 / MG996R", "1", "模拟道闸栏杆升降"};
        const char *r2_6[] = {"6", "蜂鸣器", "有源蜂鸣器（高电平触发）", "1", "车辆到达/离开声音提示"};
        const char *r2_7[] = {"7", "电阻", "2.2kΩ + 3.3kΩ", "各1", "HC-SR04 Echo 5V→3.3V 分压"};
        const char *r2_8[] = {"8", "杜邦线+面包板", "—", "若干", "电路连接"};
        const char **rows2[] = {r2_1, r2_2, r2_3, r2_4, r2_5, r2_6, r2_7, r2_8};
        add_table(f, h2, 5, rows2, 8);
    }

    fig_caption(f, "图 1-1  系统硬件连接框图");
    embed_image(f, 1, 5.5);

    heading(f, "1.3  功能要求与性能指标", 2);
    body(f, "功能要求方面：（1）光敏传感器实时采集环境光照强度（0-100%），并根据阈值自动切换昼夜模式。（2）白天模式（光照 > 40%）：道闸保持常开，超声波和蜂鸣器不工作，车辆自由通行。（3）夜晚模式（光照 ≤ 40%）：道闸关闭进入监控，检测到车辆靠近（距离 < 80 cm）时自动开闸，车辆离开后延迟 3 秒关闭，关门后冷却 3 秒再进入下一轮检测。（4）蜂鸣器在车辆到达时发出 3 声短鸣，道闸关闭时发出 1 声长鸣确认。（5）控制台实时显示光照百分比、当前模式、状态机状态和道闸开闭状态。");

    table_caption(f, "表 1-3  性能指标");
    {
        const char *h3[] = {"指标", "要求", "说明"};
        const char *r3_1[] = {"光照采样周期", "≤ 500 ms", "保证昼夜切换的及时性"};
        const char *r3_2[] = {"超声波测距范围", "2 cm – 200 cm", "覆盖车辆检测所需距离"};
        const char *r3_3[] = {"超声波测量精度", "± 1 cm", "满足车辆检测判据"};
        const char *r3_4[] = {"昼夜切换响应", "≤ 1 s", "光照变化后快速响应"};
        const char *r3_5[] = {"道闸动作时间", "≤ 0.5 s", "舵机从 0° 转至 90°"};
        const char *r3_6[] = {"连续运行稳定性", "> 1 h", "无崩溃、无内存泄漏"};
        const char **rows3[] = {r3_1, r3_2, r3_3, r3_4, r3_5, r3_6};
        add_table(f, h3, 3, rows3, 6);
    }
    page_break(f);

    /* ═══════════ 第2章 需求分析 ═══════════ */
    heading(f, "2  需求分析", 1);
    heading(f, "2.1  功能需求分解", 2);
    body(f, "根据应用场景，将整体功能拆解为以下五个子系统：");
    body(f, "（1）环境感知子系统：光敏电阻将环境光变化转换为模拟电压，经 PCF8591 ADC 进行 8 位模数转换（分辨率 256 级），树莓派通过 WiringPi I2C 接口读取数字量并映射为光照百分比（0-100%，0% 为全暗，100% 为最亮）。");
    body(f, "（2）车辆检测子系统：HC-SR04 超声波传感器发射 40 kHz 脉冲并接收回波，当距离小于车辆检测阈值（80 cm）时判定为有车辆到达。采用双采样防抖机制——40 ms 内连续采集两次，两次均低于阈值才确认有效检测，有效滤除传感器噪声导致的误触发。");
    body(f, "（3）道闸控制子系统：舵机作为执行机构模拟道闸栏杆。WiringPi 的 softPwm 模块生成 20 ms 周期 PWM 信号（range=200），占空比 5/200（0.5 ms）对应 0° 关闸、25/200（2.5 ms）对应 180° 全开。本系统设开启角度 90°（占空比 15/200），关闭角度 0°。");
    body(f, "（4）声音提示子系统：有源蜂鸣器内置振荡电路，只需 GPIO 高低电平控制发声/静音。车辆到达时发出 3 声短鸣（150 ms 开 / 80 ms 关），道闸关闭时发出 1 声确认音（200 ms）。");
    body(f, "（5）状态管理与逻辑控制子系统：核心控制逻辑采用有限状态机（FSM）设计。夜晚模式包含四个状态：等待车辆（NIGHT_WAIT）、车辆到达开闸（NIGHT_CAR_HERE）、车辆离开倒计时关闸（NIGHT_CLOSING）、关闸后冷却（NIGHT_COOLDOWN）。");

    heading(f, "2.2  硬件模块选型分析", 2);
    body(f, "主控平台选择 Raspberry Pi 4 Model B 的理由：具有 40 针 GPIO 接口，兼容 I2C、PWM 等多种外设协议；运行完整 Linux 操作系统，WiringPi 库提供统一硬件抽象层；相比 Arduino 等单片机具有更强的计算能力和更丰富的调试手段。");
    body(f, "光敏传感器 + PCF8591 ADC：光敏电阻模块输出模拟信号，树莓派本身无内置 ADC，需外接 ADC 模块。PCF8591 通过 I2C 总线通信仅需 2 根信号线（SDA/SCL），8 位分辨率对光照检测精度足够，工作电压 3.3V 与树莓派 GPIO 电平兼容。使用 WiringPi 的 wiringPiI2CSetup() 一行代码即可完成 I2C 设备初始化。");
    body(f, "超声波传感器 HC-SR04：测距范围 2 cm – 400 cm，精度可达 3 mm。采用 40 kHz 超声波方向性好。需注意 Echo 引脚输出 5V 电平，树莓派 GPIO 仅耐受 3.3V，必须使用 2.2kΩ + 3.3kΩ 电阻分压（Vout = 5V × 3.3 / 5.5 = 3.0V）后再接入 GPIO6，否则可能永久损坏 GPIO。使用 WiringPi 的 digitalWrite()/digitalRead() + micros() 即可实现微秒级精确测距。");
    body(f, "舵机 SG90：微型模拟舵机，重量轻、响应快。工作电压 4.8-6V，可由树莓派 5V 引脚供电。WiringPi 的 softPwmCreate() 生成 20 ms 周期 PWM 信号，无需额外配置硬件 PWM 设备树覆盖层（dtoverlay pwm），简化了系统启动流程。大舵机（MG996R）建议外接独立电源。");
    body(f, "蜂鸣器采用有源蜂鸣器：内置振荡器，仅需 GPIO 高低电平即可控制，digitalWrite() 一行代码完成控制。");

    heading(f, "2.3  项目重点与难点分析", 2);
    body(f, "（1）GPIO 电平兼容：树莓派 GPIO 工作电压 3.3V，耐受上限 3.6V。HC-SR04 Echo 引脚输出 5V 必须通过电阻分压后接入 GPIO6。WiringPi 虽然简化了编程，但不能改变电气特性——电平转换必须在硬件层面完成。");
    body(f, "（2）超声波时序精度：HC-SR04 测距需要微秒级时序控制。WiringPi 的 micros() 函数基于系统单调时钟，提供微秒级时间戳，配合 digitalWrite()/digitalRead() 的纳秒级响应，满足 Trig 10 μs 脉冲和 Echo 微秒级脉宽测量的精度需求。");
    body(f, "（3）信号抗干扰：光敏传感器读数可能受环境光线瞬时变化影响，超声波也可能受环境噪声干扰。本系统采用双采样法（两次测量 40 ms 内完成且均满足条件才确认），有效降低噪声导致的误判。");
    body(f, "（4）舵机供电稳定性：SG90 小舵机空载时可由树莓派 5V 引脚直接供电。若使用 MG996R 大舵机，启动电流可能拉低 5V 总线电压，导致舵机抖动或树莓派复位。解决方案是使用外部 5V/2A 电源为舵机独立供电，并与树莓派共地。");
    page_break(f);

    /* ═══════════ 第3章 程序设计 ═══════════ */
    heading(f, "3  程序设计", 1);
    heading(f, "3.1  整体软件执行流程", 2);
    body(f, "主程序 main() 启动后按以下流程执行：第一步，硬件初始化——调用 wiringPiSetupGpio() 以 BCM GPIO 编号初始化 WiringPi 库；pinMode() 配置 Trig/Echo/蜂鸣器引脚方向；wiringPiI2CSetup() 打开 PCF8591 I2C 设备；softPwmCreate() 创建舵机 PWM 通道（range=200, 20ms 周期）。第二步，模式初始化——读取当前光照值判定初始模式，设置舵机初始角度，输出欢迎界面。第三步，进入 150-300 ms 周期的无限主循环。");

    fig_caption(f, "图 3-1  主程序流程图");
    embed_image(f, 2, 5.0);

    heading(f, "3.2  核心控制逻辑：夜晚状态机设计", 2);
    body(f, "夜晚模式下的状态机是整个系统的核心逻辑，采用有限状态机（FSM）设计，包含四个状态和四条转换路径。");

    fig_caption(f, "图 3-2  夜晚状态机转换图");
    embed_image(f, 3, 5.5);

    body(f, "状态转换条件与动作详述如下：");
    body(f, "（1）NIGHT_WAIT → NIGHT_CAR_HERE：条件为 car_detected() 返回 true（40 ms 内两次超声波读数均 < 80 cm）；动作为 servo_set(90°) 开闸、buzzer_beep(3, 150, 80) 蜂鸣 3 声短鸣、print_bar() 输出\xe2\x80\x9c来车\xe2\x80\x9d提示。");
    body(f, "（2）NIGHT_CAR_HERE → NIGHT_CLOSING：条件为 car_detected() 返回 false（40 ms 内两次读数均 ≥ 80 cm 或超时）；动作为 timer_start = time(NULL) 启动 3 秒倒计时、输出\xe2\x80\x9c车已离开\xe2\x80\x9d提示。");
    body(f, "（3）NIGHT_CLOSING → NIGHT_COOLDOWN：条件为 time(NULL) - timer_start ≥ 3 秒；动作为 servo_set(0°) 关闸、buzzer_beep(1, 200, 0) 蜂鸣 1 声确认、timer_start 重置为当前时间。");
    body(f, "（4）NIGHT_COOLDOWN → NIGHT_WAIT：条件为 time(NULL) - timer_start ≥ 3 秒；动作为输出\xe2\x80\x9c冷却结束\xe2\x80\x9d、状态机回到等待模式。");
    body(f, "防抖机制设计：car_detected() 在 40 ms 内进行两次独立超声波测距，仅当两次距离值均有效（> 0 表示未超时）且均小于 CAR_DISTANCE（80 cm）时返回 true。该双采样机制有效滤除了传感器噪声、瞬时反射或电磁干扰导致的单次异常读数。");

    heading(f, "3.3  模块划分与函数封装", 2);
    body(f, "程序采用模块化设计，按功能职责进行函数封装。与旧版使用 sysfs、mmap、I2C ioctl 三套不同底层接口的方案相比，v4.0 使用 WiringPi 统一了全部硬件访问，各模块间通过函数调用传递数据。");

    table_caption(f, "表 3-1  模块划分与职责");
    {
        const char *h4[] = {"模块", "函数", "WiringPi API", "职责"};
        const char *r4_1[] = {"主控模块", "main()", "wiringPiSetupGpio()", "初始化全部硬件、管理昼夜切换、驱动状态机"};
        const char *r4_2[] = {"超声波测距", "ultrasonic_read()\ncar_detected()", "digitalWrite()\ndigitalRead()\nmicros()", "HC-SR04 微秒级测距\n双采样防抖车辆检测"};
        const char *r4_3[] = {"光敏传感器", "light_read()", "wiringPiI2CSetup()\nwiringPiI2CWrite()\nwiringPiI2CRead()", "PCF8591 ADC 读取\n转为光照百分比"};
        const char *r4_4[] = {"舵机控制", "servo_set()", "softPwmCreate()\nsoftPwmWrite()", "20ms周期PWM\n角度线性映射"};
        const char *r4_5[] = {"蜂鸣器", "buzzer_beep()", "digitalWrite()\ndelay()", "多段鸣响控制"};
        const char *r4_6[] = {"控制台显示", "print_bar()", "—", "Unicode 道闸状态可视化"};
        const char **rows4[] = {r4_1, r4_2, r4_3, r4_4, r4_5, r4_6};
        add_table(f, h4, 4, rows4, 6);
    }

    fig_caption(f, "图 3-3  软件架构层次图");
    embed_image(f, 4, 5.5);

    heading(f, "3.4  关键技术实现说明", 2);
    body(f, "WiringPi 统一接口是本版重构的核心改进。旧版使用三套不同的底层接口——sysfs 文件操作控制蜂鸣器和舵机（每次操作需要 open→write→close 三次系统调用，合计约 5-10 μs 开销）、I2C 设备节点 + ioctl 读取 PCF8591 ADC、mmap 内存映射直接操作 GPIO 寄存器（约 50 ns 延迟）驱动 HC-SR04。三种方式各有不同的初始化流程、错误处理模式和代码风格，使得程序难以维护和理解。");
    body(f, "v4.0 使用 WiringPi 后，所有 GPIO 操作统一为 digitalWrite()/digitalRead()/pinMode()，I2C 操作统一为 wiringPiI2CSetup()/wiringPiI2CRead()/wiringPiI2CWrite()，PWM 操作统一为 softPwmCreate()/softPwmWrite()。一行 wiringPiSetupGpio() 替代了旧版分散在各处的 mmap 初始化、I2C 设备打开、GPIO sysfs export 等操作。代码总行数从 299 行减少至 243 行（减少 19%），而可读性和可维护性大幅提升。");
    body(f, "容错设计方面：wiringPiI2CSetup() 失败时返回 -1，主循环以光照值 100%（白天）为默认值，保证 I2C 异常时道闸保持打开（故障安全原则）。ultrasonic_read() 超时返回 -1，car_detected() 将其视为\xe2\x80\x9c未检测到车辆\xe2\x80\x9d。关键系统调用的返回值均经过检查，确保任何单点故障不会导致系统崩溃。");
    body(f, "以下为主程序状态机核心代码：");

    /* C 代码块 */
    {
        const char *code[] = {
            "switch (night_state) {",
            "case NIGHT_WAIT:",
            "    if (car_detected()) {               // 双采样防抖",
            "        servo_set(BARRIER_OPEN);        // softPwmWrite 开闸",
            "        buzzer_beep(3, 150, 80);        // 3声短鸣",
            "        night_state = NIGHT_CAR_HERE;",
            "    }",
            "    break;",
            "case NIGHT_CAR_HERE:",
            "    if (!car_detected()) {              // 车已离开",
            "        timer_start = time(NULL);",
            "        night_state = NIGHT_CLOSING;",
            "    }",
            "    break;",
            "case NIGHT_CLOSING:",
            "    if (time(NULL) - timer_start >= CLOSE_SEC) {",
            "        servo_set(BARRIER_CLOSE);       // 关闸",
            "        buzzer_beep(1, 200, 0);         // 1声确认",
            "        night_state = NIGHT_COOLDOWN;",
            "        timer_start = time(NULL);",
            "    }",
            "    break;",
            "case NIGHT_COOLDOWN:",
            "    if (time(NULL) - timer_start >= COOLDOWN_SEC)",
            "        night_state = NIGHT_WAIT;       // 回到等待",
            "    break;",
            "}",
        };
        int n_code = sizeof(code) / sizeof(code[0]);
        for (int i = 0; i < n_code; i++) {
            add_para(f, code[i], "Courier New", "17", "left", "240", "0", "0", NULL, 0);
        }
    }
    page_break(f);

    /* ═══════════ 第4章 系统性能分析与调试测试 ═══════════ */
    heading(f, "4  系统性能分析与调试测试", 1);
    heading(f, "4.1  硬件层面分析", 2);
    body(f, "（1）电平兼容性：树莓派 GPIO 工作电压 3.3V，耐受上限 3.6V。PCF8591 ADC 和蜂鸣器模块均工作于 3.3V，与树莓派 GPIO 电平兼容。HC-SR04 VCC 需 5V 供电，Trig 输入 3.3V 可被正确识别（高电平阈值约 2.5V），Echo 输出 5V 必须经 2.2kΩ+3.3kΩ 分压至 3.0V 后接入 GPIO6。舵机信号输入 3.3V 满足 SG90 高电平识别阈值（2.0V）。WiringPi 简化了软件层面，但电气安全必须在硬件层面保证。");
    body(f, "（2）供电稳定性：树莓派 5V 引脚最大输出电流典型约 1A（与 USB 共享）。同时连接 HC-SR04（~15 mA）、PCF8591（< 1 mA）、蜂鸣器（~30 mA）和 SG90 舵机（空载 ~100 mA，堵转可达 750 mA）时，舵机动作瞬间可能导致电压骤降。推荐为大舵机提供独立 5V/2A 电源。");
    body(f, "（3）信号抗干扰：HC-SR04 工作频率 40 kHz，实验室环境不易受干扰，多传感器场景下可能产生串扰。光敏传感器对 LED 灯 PWM 调光敏感（100 Hz 波动），通过双采样降低干扰影响。面包板杜邦线连接存在接触电阻，长时间运行可能信号衰减。");

    heading(f, "4.2  软件层面分析", 2);
    body(f, "（1）程序时序精度：主循环周期白天 300 ms、夜晚 150 ms，由 delay() 控制（实际精度受 Linux 进程调度影响约 ± 5 ms）。超声波测距使用 micros() 获取微秒级时间戳（分辨率约 1 μs），满足 HC-SR04 的 10 μs Trig 脉冲需求。softPwm 由 WiringPi 内部线程驱动，基于 100 μs 基准时钟，对舵机控制精度足够。");
    body(f, "（2）运行稳定性：程序不使用动态内存分配（无 malloc），全部变量为静态分配，无内存泄漏风险。WiringPi 在程序退出时自动释放 GPIO 和 PWM 资源，无需像旧版 sysfs 方案那样手动清理 export 文件。I2C 通信失败时以默认参数降级运行不崩溃。");
    body(f, "（3）代码鲁棒性：对 wiringPiI2CSetup()、light_read()、ultrasonic_read() 等关键函数的返回值均做了检查。所有异常路径均有降级处理（默认为白天模式、忽略单次超时），保证系统在任何单一故障下不会完全失效。");
    body(f, "（4）资源占用：CPU 占用率 < 1%（主循环大部分时间在 delay() 阻塞）。内存占用约 60 KB（含 WiringPi 库开销）。文件描述符仅 wiringPiI2CSetup 维持的 1 个 I2C 连接，远低于进程限制 1024。");

    heading(f, "4.3  系统误差分析", 2);
    body(f, "（1）光照传感器误差：PCF8591 ADC 为 8 位分辨率，量化误差 ± 1 LSB（约 ± 0.4% 光照百分比）。光敏电阻响应曲线非线性，但本系统仅需判断阈值（40%），对线性度要求不高。");
    body(f, "（2）超声波测距误差：理论误差来源于声速温度依赖——v = 331.4 + 0.6 × T(℃)，温度每变化 1℃ 声速变化约 0.17%。使用固定声速 343 m/s（对应 20℃），在 15-25℃ 环境下最大误差约 ± 2%。micros() 时间测量误差 < 1 μs 对应距离误差 < 0.02 mm 可忽略。HC-SR04 自身精度 ± 3 mm。综合距离误差在 ± 1 cm 以内。");
    body(f, "（3）舵机角度误差：SG90 定位精度约 ± 1°，softPwm 的 100 μs 基准时钟引入的分辨率误差约 0.9°（2.5 ms / 200 steps），综合误差 < 2°，满足道闸模拟需求。");

    heading(f, "4.4  测试场景与数据记录", 2);
    body(f, "针对系统各功能模块和集成场景，设计了以下 8 项测试。");

    table_caption(f, "表 4-1  测试场景与结果");
    {
        const char *h5[] = {"编号", "测试内容", "测试方法", "预期结果", "实际结果"};
        const char *r5_1[] = {"T1", "光敏基础读数", "手电筒/室内/遮光三条件", "区分度良好", "通过"};
        const char *r5_2[] = {"T2", "超声波测距精度", "5/20/50/100cm 各5次取平均", "误差 ≤ ±1cm", "通过"};
        const char *r5_3[] = {"T3", "昼夜模式切换", "遮光→放开→遮光循环", "< 1s 内切换", "通过，< 0.5s"};
        const char *r5_4[] = {"T4", "来车检测+道闸动作", "手靠近/移开模拟来车/离开", "开闸蜂鸣→关闸", "功能正常"};
        const char *r5_5[] = {"T5", "防抖验证", "快速挥手掠过(< 0.1s)", "不误触发", "通过，正确过滤"};
        const char *r5_6[] = {"T6", "冷却期验证", "关闸后立即再次靠近", "冷却期内不响应", "符合设计"};
        const char *r5_7[] = {"T7", "连续运行稳定性", "持续运行 30 分钟", "不崩溃、无泄漏", "稳定运行"};
        const char *r5_8[] = {"T8", "I2C 故障容错", "运行中拔掉 PCF8591 SDA", "默认白天,不崩溃", "容错正常"};
        const char **rows5[] = {r5_1, r5_2, r5_3, r5_4, r5_5, r5_6, r5_7, r5_8};
        add_table(f, h5, 5, rows5, 8);
    }

    body(f, "测试数据分析：光照传感器在不同光照条件下的读数区分度良好，阈值 40% 能够有效区分室内照明和遮光状态。超声波测距精度在 100 cm 范围内满足 ± 1 cm 要求。双采样防抖有效过滤了快速挥手的瞬时干扰（T5 验证通过），正常的持续靠近（> 40 ms）均能正确触发。3 秒关门倒计时和 3 秒冷却期逻辑正确，状态转换无异常跳变。30 分钟连续运行测试期间无崩溃或外设通信中断。");
    body(f, "存在的异常现象及原因分析：（1）光照阈值附近模式振荡——当光照值在 40% 附近波动时系统出现昼夜模式快速切换，原因是代码中无迟滞区间，改进方向为引入施密特触发器双阈值（如 > 45% 白天、< 35% 夜晚）。（2）舵机供电不足时抖动——MG996R 大舵机未外接电源时舵机动作瞬间出现控制台输出卡顿（0.1-0.3 秒），原因是舵机启动电流拉低 5V 总线，改进方向为独立供电。（3）首次运行 I2C 错误——未预先执行 sudo dtoverlay i2c-gpio 命令时程序无法打开 I2C 设备，解决方法为在启动脚本中先执行 dtoverlay。");
    page_break(f);

    /* ═══════════ 第5章 结论与展望 ═══════════ */
    heading(f, "5  结论与展望", 1);
    heading(f, "5.1  项目完成总结", 2);
    body(f, "本项目成功设计并实现了一套基于树莓派的智能停车场道闸系统，达成全部功能目标——环境感知、车辆检测、道闸控制、声音提示和状态管理五大模块均稳定运行。v4.0 使用 WiringPi 库重构后，代码统一性、可读性和可维护性大幅提升，所有硬件访问方式从三套不同的底层接口统一为一套 WiringPi API，代码行数减少 19%。验证了从裸机寄存器编程到使用硬件抽象库的工程实践演进路径。");

    heading(f, "5.2  掌握的硬件开发技能", 2);
    body(f, "通过本项目系统性地学习和实践了以下嵌入式 Linux 开发技能：（1）WiringPi 库的使用——掌握了 GPIO 数字输入输出（pinMode/digitalWrite/digitalRead）、I2C 通信（wiringPiI2CSetup/wiringPiI2CRead）、软件 PWM 生成（softPwmCreate/softPwmWrite）等 API。（2）外设驱动开发——PCF8591 ADC 的 I2C 控制协议、HC-SR04 超声波的微秒级时序测量、SG90 舵机的 PWM 信号生成。（3）硬件接口设计——GPIO 电平兼容性（5V/3.3V 转换）、电阻分压电路设计与计算、供电稳定性分析。（4）嵌入式软件设计方法——有限状态机建模、模块化函数封装、容错设计模式（故障安全原则）。");

    heading(f, "5.3  存在问题与改进措施", 2);
    table_caption(f, "表 5-1  问题与改进措施");
    {
        const char *h6[] = {"序号", "存在问题", "改进措施"};
        const char *r6_1[] = {"1", "无迟滞阈值，光照在40%附近频繁切换", "引入施密特双阈值(>45%白天/<35%夜晚)"};
        const char *r6_2[] = {"2", "超声波无温度补偿，测距受温度影响", "加入DS18B20温度传感器动态修正声速"};
        const char *r6_3[] = {"3", "蜂鸣器阻塞式（鸣响期间无法响应传感器）", "改为非阻塞模式：状态变量+time(NULL)计时"};
        const char *r6_4[] = {"4", "程序无持久化日志记录", "添加日志模块，写入/var/log/parking.log"};
        const char *r6_5[] = {"5", "异常退出时资源未清理", "注册SIGINT信号处理函数进行资源回收"};
        const char *r6_6[] = {"6", "单线程轮询架构，响应速度有限", "多线程：超声波/光敏独立线程，主线程状态机"};
        const char *r6_7[] = {"7", "无远程监控能力", "添加MQTT/HTTP上报停车场状态到远程服务器"};
        const char **rows6[] = {r6_1, r6_2, r6_3, r6_4, r6_5, r6_6, r6_7};
        add_table(f, h6, 3, rows6, 7);
    }

    heading(f, "5.4  心得体会", 2);
    body(f, "本次课程设计让我深刻体会到嵌入式系统开发\xe2\x80\x9c软硬兼施\xe2\x80\x9d的特点。与纯软件开发不同，嵌入式系统需要同时关注硬件层面的电气特性（电平兼容、供电稳定性、信号完整性）和软件层面的逻辑设计（时序控制、状态管理、容错处理）。一个看似简单的\xe2\x80\x9c检测来车→开闸→关闸\xe2\x80\x9d流程，在实际实现中需要处理 GPIO 电平转换、微秒级脉冲时序测量、传感器去抖动、异常容错等一系列细节问题。");
    body(f, "在技术演进路径上，从旧版的直接寄存器操作（sysfs/mmap/I2C ioctl）到新版 WiringPi 统一抽象层，我深刻理解了硬件抽象库（HAL）在嵌入式开发中的价值——它将开发者从底层的位操作和寄存器地址中解放出来，让我们能够专注于应用逻辑的设计和优化。同时，理解底层原理依然重要：当 WiringPi 的 softPwm 不够精确或 I2C 通信出现异常时，了解底层寄存器操作可以帮助我们快速定位问题根源。");
    body(f, "通过从模块测试（LED 闪烁 → 光敏读数 → 超声波测距 → 光敏+舵机联动）到系统集成（完整停车场道闸）的渐进式开发过程，我体会到了\xe2\x80\x9c分而治之、逐层集成\xe2\x80\x9d的重要性——每个模块独立验证后再组合，能够有效隔离问题、降低调试难度。这一方法论不仅适用于嵌入式开发，也是所有复杂系统设计的通用原则。");
    page_break(f);

    /* ═══════════ 参考文献 ═══════════ */
    heading(f, "参考文献", 0);
    {
        const char *refs[] = {
            "[1] 吴军. 嵌入式系统设计与实践[M]. 北京: 机械工业出版社, 2019: 156-189.",
            "[2] 刘火良, 杨森. Linux 设备驱动开发详解[M]. 北京: 机械工业出版社, 2020: 203-245.",
            "[3] Raspberry Pi Foundation. BCM2711 ARM Peripherals Datasheet[Z]. https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf, 2020.",
            "[4] Gordon Henderson. WiringPi GPIO Interface Library[Z]. https://github.com/WiringPi/WiringPi, 2024.",
            "[5] 周立功. I2C 总线应用系统设计[M]. 北京: 北京航空航天大学出版社, 2018: 45-78.",
            "[6] NXP Semiconductors. PCF8591 8-bit A/D and D/A converter Datasheet[Z]. https://www.nxp.com/docs/en/data-sheet/PCF8591.pdf, 2013.",
            "[7] 张辉, 李强. 基于 Linux sysfs 接口的 GPIO 驱动方法研究[J]. 电子技术应用, 2020, 46(3): 112-116.",
            "[8] ElecFreaks. HC-SR04 Ultrasonic Ranging Module User Manual[Z]. https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf, 2018.",
            "[9] 陈皓. C 语言接口与实现[M]. 北京: 人民邮电出版社, 2018: 301-330.",
        };
        int n_refs = sizeof(refs) / sizeof(refs[0]);
        for (int i = 0; i < n_refs; i++)
            add_para(f, refs[i], FONT, SZ_SMALL, "left", LS_BODY, "0", "0", NULL, 0);
    }
    page_break(f);

    /* ═══════════ 成绩评定表 ═══════════ */
    heading(f, "成绩评定表", 0);
    table_caption(f, "表 附-1  课程设计（实训）成绩评定表");
    {
        const char *h7[] = {"评定项目", "系数", "5（优）", "4（良）", "3（合格）", "1（不合格）", "评定结果"};
        const char *r7_1[] = {"设计(实训)\n方案", "6",
            "硬件选型合理，适配树莓派平台；控制/数据处理算法在通用方案上优化改进，运行效率高、时序稳定",
            "硬件选型正确，适配树莓派；采用行业主流算法，逻辑正确，可稳定实现基础控制",
            "硬件选型基本适配；所用基础算法可实现项目核心功能，无明显逻辑错误",
            "硬件选型错误、与树莓派不兼容；算法逻辑错误，无法实现项目基础功能", ""};
        const char *r7_2[] = {"设计作品\n(实训结果)", "8",
            "C语言代码结构清晰、模块化、规范性强，可稳定编译运行；硬件接线规范，外设驱动稳定，程序鲁棒性好；测试场景全面，调试记录完整，运行结果分析到位",
            "代码结构清晰，模块化编写；硬件接线正确，可实现全部预设功能；完成核心功能调试，有对应测试数据与基础分析",
            "代码可实现项目主要功能；硬件接线无误，可正常运行；完成基础功能测试，有简单调试记录",
            "代码无法编译运行；硬件接线错误，外设无法正常驱动；未完成核心功能，无调试与测试内容", ""};
        const char *r7_3[] = {"设计(实训)\n报告", "6",
            "文字表达准确、流畅，符合学术规范；文献资料翔实，有代表性、实效性；思路清晰，逻辑严密，结构严谨，论证充分",
            "课程设计报告符合学术规范；参考了文献资料；论文能围绕论点展开论述，结构较清晰",
            "课程设计报告基本符合学术规范；论文能围绕论点展开论述",
            "课程设计报告不能围绕论点展开论述", ""};
        const char *r7_4[] = {"总评得分", "", "", "", "", "", ""};
        const char *r7_5[] = {"总评等级", "", "", "", "", "", ""};
        const char **rows7[] = {r7_1, r7_2, r7_3, r7_4, r7_5};
        add_table(f, h7, 7, rows7, 5);
    }

    /* 注释 */
    empty_line(f, LS_BODY);
    add_para(f, "注：", FONT, SZ_SMALL, "left", LS_BODY, "0", "0", NULL, 1);
    body_ni(f, "总评得分 = Σ（分项得分 × 系数）。总评得分按如下对应关系转换为等级：");
    body_ni(f, "90～100 优    80～89 良    70～79 中    60～69 及格    0～59 不及格");

    doc_end(f);
    fclose(f);
}

/* ==================== 主程序 ==================== */

int main(void) {
    printf("生成课程设计报告 .docx ...\n");

    /* 1. 创建临时目录 */
    char tmp[256];
    strcpy(tmp, TMP_DIR);
    char *td = mkdtemp(tmp);
    if (!td) {
        fprintf(stderr, "错误: 无法创建临时目录\n");
        return 1;
    }
    printf("  临时目录: %s\n", td);

    /* 2. 创建子目录 */
    char word_media[512], word_rels[512], root_rels[512];
    snprintf(word_media, sizeof(word_media), "%s/word/media", td);
    snprintf(word_rels, sizeof(word_rels), "%s/word/_rels", td);
    snprintf(root_rels, sizeof(root_rels), "%s/_rels", td);
    mkdir_p(word_media);
    mkdir_p(word_rels);
    mkdir_p(root_rels);

    /* 3. 写入 XML 文件 */
    printf("  生成 XML...\n");
    write_content_types(td);
    write_rels(td);
    write_doc_rels(td);
    write_styles(td);
    write_document(td);
    printf("  ✓ 所有 XML 文件已生成\n");

    /* 4. 复制图片 */
    printf("  复制图片...\n");
    for (int i = 0; i < N_IMAGES; i++) {
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", DIAGRAM_DIR, img_names[i]);
        snprintf(dst, sizeof(dst), "%s/word/media/%s", td, img_names[i]);
        if (copy_file(src, dst) == 0)
            printf("    ✓ %s\n", img_names[i]);
        else
            printf("    ✗ %s (跳过)\n", img_names[i]);
    }

    /* 5. 打包为 ZIP → .docx */
    printf("  打包 DOCX...\n");
    {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd),
                 "cd \"%s\" && zip -r -q docx_tmp.zip . && mv docx_tmp.zip \"%s\"",
                 td, OUT_DOCX);
        int ret = system(cmd);
        if (ret != 0) {
            fprintf(stderr, "错误: ZIP 打包失败\n");
            return 1;
        }
    }

    /* 6. 清理临时目录 */
    {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", td);
        system(cmd);
    }

    /* 7. 检查输出 */
    {
        long sz = -1;
        FILE *f = fopen(OUT_DOCX, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            sz = ftell(f);
            fclose(f);
        }
        if (sz > 0) {
            printf("\n✅ 报告已生成: %s\n", OUT_DOCX);
            printf("📄 文件大小: %ld KB\n", sz / 1024);
        } else {
            printf("\n❌ 报告生成失败\n");
            return 1;
        }
    }

    return 0;
}
