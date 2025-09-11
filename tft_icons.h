#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void tft_icon_sunny (int16_t x, int16_t y, int scale);
void tft_icon_cloudy(int16_t x, int16_t y, int scale);
void tft_icon_rain  (int16_t x, int16_t y, int scale);

typedef struct {
    const uint32_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t  dataSize;
} tImage;

extern const tImage sun1;
extern const tImage clouds;
extern const tImage rainy;

#ifdef __cplusplus
}

#endif // ICONS_H
