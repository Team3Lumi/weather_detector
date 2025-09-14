// buttons.h
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_BACK_PRESS,
    BTN_NEXT_PRESS,
} btn_event_t;

/**
 * Khởi tạo 2 nút (active LOW, bật pull-up) và backlight
 * - pin_back, pin_next: GPIO nút
 * - pin_bl: GPIO backlight (điền -1 nếu không dùng)
 */
esp_err_t buttons_init(gpio_num_t pin_back, gpio_num_t pin_next, gpio_num_t pin_bl);

/** Bật/tắt backlight (true=ON, false=OFF) */
void buttons_set_backlight(bool on);

/** Lấy queue sự kiện để xQueueReceive trong app */
QueueHandle_t buttons_event_queue(void);

#ifdef __cplusplus
}
#endif
