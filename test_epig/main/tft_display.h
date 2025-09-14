#pragma once
#include "tft_types.h"
#include <stdint.h>
#include "icons.h"



#ifdef __cplusplus
extern "C" {
#endif

// Ghi chú pin (theo bộ test ST7735S của bạn):
// CS=11, RST=18, DC=8, MOSI=9, SCLK=10, LED=17

int  tft_init(uint8_t rotation);   // 0..3
void tft_set_rotation(uint8_t rotation);
void tft_clear(void);
void tft_render_day(const tft_day_t *d);

#ifdef __cplusplus
}
#endif
