#include "include_dir.h"

static const char *TAG = "wifi_sta";

void
app_main(void){
    ESP_LOGI(TAG, "Starting weather station");

    // 1. Kết nối Wi-Fi
    wifi_init_sta();

    // 3. Khởi động task HTTP
    http_start_task();
}
