#include "tft_display.h"
#include "tft_icons.h"

// ======= CHỌN BACKEND =======
#if defined(ARDUINO)

// -------- Arduino backend (Adafruit ST7735) --------
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Pin của bạn:
#define TFT_CS    11
#define TFT_RST   18
#define TFT_DC    8
#define TFT_MOSI  9
#define TFT_SCLK  10
#define TFT_LED   17

static Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
static int s_rot = 1;

// vẽ icon thật bằng primitive của Adafruit_GFX
static void drawSun(int x, int y, int scale) {
  int r = 10 * scale;
  tft.fillCircle(x, y, r, ST77XX_YELLOW);
}
static void drawCloud(int x, int y, int scale) {
  int r = 10 * scale;
  tft.fillCircle(x, y, r, ST77XX_WHITE);
  tft.fillCircle(x + 12*scale, y, r + 2*scale, ST77XX_WHITE);
  tft.fillCircle(x + 24*scale, y + 4*scale, r - 2*scale, ST77XX_WHITE);
  tft.fillRect(x, y + 8*scale, 30*scale, 10*scale, ST77XX_WHITE);
}
static void drawRain(int x, int y, int scale) {
  drawCloud(x, y, scale);
  for (int i = 0; i < 5; ++i) {
    tft.fillRect(x + 5*scale + i*5*scale, y + 20*scale, 2*scale, 6*scale, ST77XX_BLUE);
  }
}

// override các hàm icon (weak) từ tft_icons.c
void tft_icon_sunny (int16_t x,int16_t y,int scale){ drawSun(x,y,scale); }
void tft_icon_cloudy(int16_t x,int16_t y,int scale){ drawCloud(x,y,scale); }
void tft_icon_rain  (int16_t x,int16_t y,int scale){ drawRain(x,y,scale); }

int tft_init(void) {
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(s_rot);
  tft.fillScreen(ST77XX_BLACK);
  return 0;
}

void tft_set_rotation(int rot) {
  s_rot = rot;
  tft.setRotation(s_rot);
}

void tft_clear(void) {
  tft.fillScreen(ST77XX_BLACK);
}

static void printText(int16_t x, int16_t y, uint16_t color, uint8_t size, const char* s) {
  tft.setCursor(x, y); tft.setTextColor(color); tft.setTextSize(size); tft.print(s);
}

void tft_render_day(const tft_day_t* d) {
  if (!d) return;
  tft_clear();

  printText(10, 10, ST77XX_WHITE, 2, d->day);

  // icon
  switch (d->icon) {
    case TFT_ICON_SUNNY:  tft_icon_sunny(80, 50, 1);  break;
    case TFT_ICON_CLOUDY: tft_icon_cloudy(60, 40, 1); break;
    case TFT_ICON_RAIN:   tft_icon_rain(60, 40, 1);   break;
  }

  // info
  tft.setTextSize(1);
  printText(10, 90,  ST77XX_CYAN, 1,   "Status: ");
  printText(62, 90,  ST77XX_CYAN, 1,   (d->icon==TFT_ICON_SUNNY?"Sunny": d->icon==TFT_ICON_CLOUDY?"Cloudy":"Rain"));
  printText(10, 110, ST77XX_GREEN,1,   "Temp: ");
  tft.setCursor(50,110); tft.setTextColor(ST77XX_GREEN); tft.print(d->temp_c); tft.print(" C");
  printText(10, 130, ST77XX_BLUE, 1,   "Humidity: ");
  tft.setCursor(74,130); tft.setTextColor(ST77XX_BLUE);  tft.print(d->humidity_pct); tft.print(" %");
}

#else
// -------- Mock backend (không Arduino/không driver) --------
#include <stdio.h>
static int s_rot = 1;
int  tft_init(void){ printf("[TFT MOCK] init, rot=%d\n", s_rot); return 0; }
void tft_set_rotation(int rot){ s_rot=rot; printf("[TFT MOCK] set rot=%d\n", s_rot); }
void tft_clear(void){ printf("[TFT MOCK] clear\n"); }
void tft_render_day(const tft_day_t* d){
  if(!d) return;
  printf("[TFT MOCK] %s | %dC | %d%% | %s\n",
         d->day, d->temp_c, d->humidity_pct,
         d->icon==TFT_ICON_SUNNY?"Sunny": d->icon==TFT_ICON_CLOUDY?"Cloudy":"Rain");
}
#endif
