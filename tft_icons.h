#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Vẽ biểu tượng thời tiết ở (x,y) với kích thước cơ bản (scale=1,2..)
void tft_icon_sunny(int16_t x, int16_t y, int scale);
void tft_icon_cloudy(int16_t x, int16_t y, int scale);
void tft_icon_rain(int16_t x, int16_t y, int scale);

#ifdef __cplusplus
}
#endif
