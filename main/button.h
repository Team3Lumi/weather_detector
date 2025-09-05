#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"

// Pin mapping (match code mẫu bạn gửi trước)
#define BTN_NEXT   15
#define BTN_BACK   16

void button_task(void *pvParameters);

#endif // BUTTON_H
