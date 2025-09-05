#pragma once
#include "tft_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Ghi chú pin (theo bộ test ST7735S của bạn):
// CS=11, RST=18, DC=8, MOSI=9, SCLK=10, LED=17

// Khởi tạo màn hình, trả 0 nếu OK.
int tft_init(void);

// Xóa màn hình về đen.
void tft_clear(void);

// Vẽ 1 “card” hiển thị cho 1 ngày.
void tft_render_day(const tft_day_t* d);

// Đổi rotation (0..3).
void tft_set_rotation(int rot);

#ifdef __cplusplus
}
#endif
