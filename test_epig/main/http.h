#ifndef HTTP_H
#define HTTP_H

#include "esp_err.h"
#include "esp_http_client.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "tft_types.h"

#define WEATHER_API_URL "https://api.open-meteo.com/v1/forecast?latitude=21.0&longitude=105.75&daily=temperature_2m_max,precipitation_probability_mean,weathercode&timezone=auto"

#ifndef FORECAST_DAYS
#define FORECAST_DAYS 7
#endif

#define RESP_MAX_BYTES   (64 * 1024)

// Tải JSON từ API, lưu SPIFFS (/spiffs/forecast.json) và (tuỳ chọn) trả về mảng 7 ngày
esp_err_t forecast_fetch_and_cache(tft_day_t out[FORECAST_DAYS], int *out_count);

// Đọc JSON đã lưu trong SPIFFS và parse ra mảng ngày
esp_err_t forecast_load_from_spiffs(tft_day_t out[FORECAST_DAYS], int *out_count);

// Giữ lại hàm cũ cho ai còn gọi:
void get_weather_forecast(void);  // wrapper -> forecast_fetch_and_cache(NULL, NULL)


#endif