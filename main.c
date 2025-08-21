#include "http.h"
#include "Wifi.h"
#include "spiffs_store.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MAIN";
static void fetch_task(void *arg) {
    get_weather_forecast();
    vTaskDelete(NULL);
}

void
app_main(void){
    ESP_LOGI(TAG, "Starting weather station");

    // 1. Kết nối Wi-Fi
    wifi_init_sta();

    if (spiffs_init() == ESP_OK) {
        // 3) Thử đọc cache và hiển thị ngay
        char *cached = NULL; size_t clen = 0;
        if (spiffs_load_json(&cached, &clen) == ESP_OK) {
            cJSON *json = cJSON_ParseWithLength(cached, clen);
            if (json) {
                // TODO: rút trích daily & vẽ TFT ngay lập tức
                cJSON_Delete(json);
            }
            free(cached);
        }
    }

    // 4) Tạo task cập nhật mới từ Internet (stack 10–12 KB)
    xTaskCreate(fetch_task, "fetch", 12*1024, NULL, 5, NULL);

}