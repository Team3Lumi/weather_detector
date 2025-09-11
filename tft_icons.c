#include "tft_icons.h"
#include "lcd.h"
#include <math.h>

// __attribute__((weak)) void tft_icon_sunny(int16_t x, int16_t y, int scale) { (void)x;(void)y;(void)scale; }
// __attribute__((weak)) void tft_icon_cloudy(int16_t x, int16_t y, int scale) { (void)x;(void)y;(void)scale; }
// __attribute__((weak)) void tft_icon_rain(int16_t x, int16_t y, int scale) { (void)x;(void)y;(void)scale; }

static inline void fill_circle(int cx,int cy,int r,uint16_t color){
    for (int dy=-r; dy<=r; ++dy){
        int dx = (int)(sqrtf((float)r*r - (float)dy*dy) + 0.5f);
        LCD_DrawLine(cx-dx, cy+dy, cx+dx, cy+dy, color);
    }
}

void tft_icon_sunny(int16_t x,int16_t y,int scale){
    int r = 10*scale;
    fill_circle(x, y, r, 0xFFE0); // YELLOW
    const int R1=14*scale, R2=22*scale;
    const int dx[8]={1,1,0,-1,-1,-1,0,1};
    const int dy[8]={0,1,1, 1, 0,-1,-1,-1};
    for(int i=0;i<8;i++){
        LCD_DrawLine(x+dx[i]*R1, y+dy[i]*R1, x+dx[i]*R2, y+dy[i]*R2, 0xFFE0);
    }
}

void tft_icon_cloudy(int16_t x,int16_t y,int scale){
    int r = 10*scale;
    fill_circle(x-12*scale, y,          r, 0xFFFF);
    fill_circle(x+ 4*scale, y-5*scale,  r+2*scale, 0xFFFF);
    fill_circle(x+20*scale, y,          r, 0xFFFF);
    for(int yy=0; yy<12*scale; ++yy) LCD_DrawLine(x-24*scale, y+yy, x+24*scale, y+yy, 0xFFFF);
}

void tft_icon_rain(int16_t x,int16_t y,int scale){
    tft_icon_cloudy(x, y-6*scale, scale);
    for (int i=-14*scale;i<=14*scale;i+=14*scale){
        LCD_DrawLine(x+i, y+10*scale, x+i-5*scale, y+20*scale, 0x001F); // BLUE
    }
}