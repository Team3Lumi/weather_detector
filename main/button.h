#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"

// Định nghĩa chân nút (match với code mẫu bạn gửi)
#define BTN_NEXT   15
#define BTN_BACK   16

// Hàm khởi động task nút bấm
void button_task(void *pvParameters);

#endif // BUTTON_H
