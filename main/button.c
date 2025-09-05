#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// ==== Include các module khác ====
#include "wifi.h"        // module WiFi bạn đã viết
#include "weather.h"     // module Weather API bạn đã viết
// #include "ui.h"       // UI: CHƯA XONG, để trống

#define BTN_NEXT   15
#define BTN_BACK   16

static const char *TAG = "BUTTON_CTRL";

int currentDay = 0;

// ===== Hàm xử lý nút =====
void button_task(void *pvParameters) {
    // Config nút
    gpio_set_direction(BTN_NEXT, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN_NEXT);
    gpio_set_direction(BTN_BACK, GPIO_MODE_INPUT);
    gpio_pullup_en(BTN_BACK);

    ESP_LOGI(TAG, "Button task started");

    while (1) {
        if (gpio_get_level(BTN_NEXT) == 0) {
            currentDay = (currentDay + 1) % 7;

            // 🟢 Gọi API lấy dữ liệu mới
            get_weather_forecast();

            // 🟢 Sau đó gọi UI (chưa viết, để trống)
            // ui_drawWeather(currentDay);

            ESP_LOGI(TAG, "NEXT -> Ngày %d", currentDay);
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
        }

        if (gpio_get_level(BTN_BACK) == 0) {
            currentDay = (currentDay - 1 + 7) % 7;

            // 🟢 Gọi API lấy dữ liệu mới
            get_weather_forecast();

            // 🟢 Sau đó gọi UI (chưa viết, để trống)
            // ui_drawWeather(currentDay);

            ESP_LOGI(TAG, "BACK -> Ngày %d", currentDay);
            vTaskDelay(pdMS_TO_TICKS(300)); // debounce
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ===== Hàm main khởi động toàn hệ thống =====
void app_main(void) {
    ESP_LOGI(TAG, "=== WEATHER STATION START ===");

    // 1. Kết nối WiFi
    wifi_init_sta();

    // 2. Lấy dữ liệu dự báo ban đầu
    get_weather_forecast();

    // 3. Khởi động task nút bấm
    xTaskCreate(button_task, "button_task", 4096, NULL, 5, NULL);

    // 🟢 UI khởi tạo ban đầu (chưa viết, để trống)
    // ui_drawWeather(currentDay);
}
