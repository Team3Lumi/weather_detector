#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "lcd.h"    
#include <stdint.h>
#include "icons.h"
#include "tft_display.h"
#include "Wifi.h"
#include "http.h"
#include "spiffs_store.h"
#include "buttons.h"
//static const char* TAG = "TFT_WEATHER_UI";

// ========== PIN MAP ==========
#define PIN_TFT_LED    17
#define PIN_BTN_BACK   16   // đã chỉnh đúng chiều
#define PIN_BTN_NEXT   15

static tft_day_t g_days[FORECAST_DAYS];
static int       g_count = 0;
static int       g_index = 0;

static void fetch_task(void *arg) {
    tft_day_t days[FORECAST_DAYS]; int n = 0;
    if (forecast_fetch_and_cache(days, &n) == ESP_OK && n > 0) {
        // cập nhật bộ nhớ đang hiển thị
        memcpy(g_days, days, sizeof(days));
        g_count = n;
        if (g_index == 0) tft_render_day(&g_days[0]);  // refresh nếu đang ở ngày 0
    }
    vTaskDelete(NULL);
}

static void btn_task(void *arg)
{
    QueueHandle_t q = buttons_event_queue();
    btn_event_t evt;
    while (1) {
        if (xQueueReceive(q, &evt, portMAX_DELAY)) {
            if (evt == BTN_NEXT_PRESS && g_count > 0) {
                g_index = (g_index + 1) % g_count;
                tft_render_day(&g_days[g_index]);
            } else if (evt == BTN_BACK_PRESS && g_count > 0) {
                g_index = (g_index + g_count - 1) % g_count;
                tft_render_day(&g_days[g_index]);
            }
        }
    }
}

void app_main(void)
{
    // 0) Wi-Fi + SPIFFS
    wifi_init_sta();
    spiffs_init();

    // 1) Nút (bật backlight PIN_TFT_LED = 17)
    ESP_ERROR_CHECK(buttons_init(PIN_BTN_BACK, PIN_BTN_NEXT, PIN_TFT_LED));

    // 2) TFT
    tft_init(1);               // rotation 1 (landscape). Đổi 0..3 tùy panel.

    // 3) Hiển thị cache nếu có (để không trắng khi mới boot)
    if (forecast_load_from_spiffs(g_days, &g_count) == ESP_OK && g_count > 0) {
        tft_render_day(&g_days[0]);
    } else {
        tft_clear();           // hoặc vẽ splash "No cache"
    }

    // 4) Task nút + Task cập nhật HTTP
    xTaskCreate(btn_task,   "btn_task",   2048,   NULL, 5, NULL);
    xTaskCreate(fetch_task, "fetch_task", 12*1024, NULL, 5, NULL);


}

