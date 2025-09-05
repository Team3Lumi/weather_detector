#include "button.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi_module.h"
#include "api_module.h"
#include "tft_display.h"

extern int currentDay;
extern tft_day_t weather_forecast[7];  // dữ liệu từ module API

// Hàm khởi tạo nút
static void button_init()
{
    gpio_reset_pin(BTN_NEXT);
    gpio_set_direction(BTN_NEXT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_NEXT, GPIO_PULLUP_ONLY);

    gpio_reset_pin(BTN_BACK);
    gpio_set_direction(BTN_BACK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_BACK, GPIO_PULLUP_ONLY);
}

// Task quét nút
void button_task(void *pvParameters)
{
    button_init();

    while (1) {
        if (gpio_get_level(BTN_NEXT) == 0) {  
            currentDay = (currentDay + 1) % 7;
            get_weather_forecast();  // gọi API cập nhật
            tft_render_day(&weather_forecast[currentDay]); // gọi UI
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
        }

        if (gpio_get_level(BTN_BACK) == 0) {  
            currentDay = (currentDay - 1 + 7) % 7;
            get_weather_forecast();  
            tft_render_day(&weather_forecast[currentDay]);
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // polling delay
    }
}
