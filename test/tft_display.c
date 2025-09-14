// tft_display.c  — ESP-IDF backend (dựa trên driver lcd.c của bạn)

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "lcd.h"            // LCD_Init, LCD_Direction, LCD_Clear, LCD_DrawLine, LCD_ShowString,...
#include "tft_display.h"    // tft_day_t, prototype
#include "tft_icons.h"      // tft_icon_sunny/cloudy/rain (weak ở tft_icons.c)

#ifndef LCD_W
#define LCD_W 160           // fallback cho ST7735 160x128
#endif
#ifndef LCD_H
#define LCD_H 128
#endif

// fallback màu nếu lcd.h chưa định nghĩa
#ifndef BLACK
#define BLACK 0x0000
#endif
#ifndef WHITE
#define WHITE 0xFFFF
#endif
#ifndef RED
#define RED   0xF800
#endif
#ifndef CYAN
#define CYAN  0x07FF
#endif
#ifndef YELLOW
#define YELLOW 0xFFE0
#endif
#ifndef BLUE
#define BLUE  0x001F
#endif

// gói gọi ShowString để tránh cảnh báo -Wpointer-sign
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
static inline void TXT(int x, int y, uint16_t bg, uint16_t fg,
                       const char *s, uint8_t font, uint8_t mode)
{
    LCD_ShowString(x, y, bg, fg, (uint8_t*)s, font, mode);
}
#pragma GCC diagnostic pop

// ========== API ==========
int tft_init(uint8_t rotation)
{
    LCD_Init();                       // init SPI + IC LCD (reset, SLPOUT, DISPON…)
    LCD_Direction(rotation & 3);      // 0..3
    LCD_Clear(BLACK);
    return 0;
}

void tft_set_rotation(uint8_t rotation)
{
    LCD_Direction(rotation & 3);
}

void tft_clear(void)
{
    LCD_Clear(BLACK);
}

// ========== Vẽ 1 ngày ==========
static void draw_header(const tft_day_t *d)
{
    // header bar
    const int H = 22;
    for (int y = 0; y < H; ++y) LCD_DrawLine(0, y, LCD_W - 1, y, 0x0015); // xanh đậm
    // ngày (YYYY-MM-DD)
    TXT(6, 3, 0x0015, WHITE, d->day, 19, 0);
}

static void draw_separators(void)
{
    const int y1 = 23, y2 = LCD_H - 32;
    LCD_DrawLine(0, y1, LCD_W - 1, y1, 0x39E7);
    LCD_DrawLine(0, y2, LCD_W - 1, y2, 0x39E7);
}

static void draw_icon(int cx, int cy, int scale, int icon)
{
    switch (icon) {
        case TFT_ICON_SUNNY:  tft_icon_sunny (cx, cy, scale); break;
        case TFT_ICON_CLOUDY: tft_icon_cloudy(cx, cy, scale); break;
        case TFT_ICON_RAIN:   tft_icon_rain  (cx, cy, scale); break;
        default:              tft_icon_cloudy(cx, cy, scale); break;
    }
}

void tft_render_day(const tft_day_t *d)
{
    if (!d) return;

    LCD_Clear(BLACK);

    // Header + dividers
    draw_header(d);
    draw_separators();

    // Icon giữa màn
    const int body_cx = LCD_W / 2;
    const int body_cy = ( (LCD_H - 32) + 24 ) / 2;
    draw_icon(body_cx, body_cy, 1, d->icon);

    // Thông tin
    char buf[48];

    // Trạng thái
    const char *status =
        (d->icon == TFT_ICON_SUNNY)  ? "Sunny"  :
        (d->icon == TFT_ICON_CLOUDY) ? "Cloudy" : "Rain";
    TXT(6, LCD_H - 28, BLACK, CYAN, "Status:", 16, 0);
    TXT(58, LCD_H - 28, BLACK, CYAN, status, 16, 0);

    // Nhiệt độ
    snprintf(buf, sizeof(buf), "Temp: %d C", d->temp_c);
    TXT(6, LCD_H - 18, BLACK, 0x07E0 /*GREEN*/, buf, 16, 0);

    // % mưa / ẩm (tuỳ struct của bạn)
    #ifdef TFT_DAY_HAS_RAIN_PCT
      snprintf(buf, sizeof(buf), "Rain: %d %%", d->rain_pct);
    #else
      snprintf(buf, sizeof(buf), "Hum : %d %%", d->humidity_pct);
    #endif
    TXT(6, LCD_H - 8, BLACK, BLUE, buf, 16, 0);
}
