#ifndef HTTP_H
#define HTTP_H

#include "esp_err.h"
#include "esp_http_client.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"

#define WEATHER_API_URL "https://api.open-meteo.com/v1/forecast?latitude=21.0&longitude=105.75&daily=temperature_2m_max,precipitation_probability_mean,weathercode&timezone=auto"

void get_weather_forecast(void);

#endif
