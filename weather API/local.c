#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "cJSON.h"

#define STORAGE_PATH "/sdcard/weather"
static const char *TAG = "SD_SAVE";

// Hàm lưu dữ liệu 1 ngày vào file
void save_day_weather(const char *date,
                      double temp_max,
                      double precipitation,
                      int weathercode,
                      struct tm today)
{
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s.txt", STORAGE_PATH, date);

    // Tạo JSON
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "date", date);
    cJSON_AddNumberToObject(root, "temperature_max", temp_max);
    cJSON_AddNumberToObject(root, "precipitation", precipitation);
    cJSON_AddNumberToObject(root, "weathercode", weathercode);

    char today_str[16];
    strftime(today_str, sizeof(today_str), "%Y-%m-%d", &today);
    cJSON_AddStringToObject(root, "last_access", today_str);

    // Chuyển JSON thành string
    char *json_str = cJSON_Print(root);

    // Ghi xuống file
    FILE *f = fopen(filepath, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Không thể mở file %s để ghi", filepath);
        cJSON_Delete(root);
        return;
    }
    fprintf(f, "%s\n", json_str);
    fclose(f);

    ESP_LOGI(TAG, "Đã lưu dữ liệu thời tiết vào %s", filepath);

    // Giải phóng
    free(json_str);
    cJSON_Delete(root);
}

// Hàm giả lập lấy từ API → lưu 7 ngày
void save_weather_from_api_example()
{
    // Ví dụ dữ liệu từ Open-Meteo API
    const char *dates[7] = {
        "2025-08-18", "2025-08-19", "2025-08-20",
        "2025-08-21", "2025-08-22", "2025-08-23", "2025-08-24"
    };
    double temps[7] = {30, 31, 32, 33, 34, 35, 36};
    double rains[7] = {20, 10, 0, 5, 50, 70, 40};
    int codes[7] = {2, 3, 1, 0, 2, 2, 3};

    // Lấy ngày hiện tại
    time_t now = time(NULL);
    struct tm today;
    localtime_r(&now, &today);

    for (int i = 0; i < 7; i++) {
        save_day_weather(dates[i], temps[i], rains[i], codes[i], today);
    }
}
