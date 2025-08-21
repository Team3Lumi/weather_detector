#ifndef SPIFFS_STORE_H
#define SPIFFS_STORE_H

#include "esp_err.h"
#include <stddef.h>
#include "spiffs_store.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <stdio.h>
#include <stdlib.h>


#define FORECAST_PATH "/spiffs/forecast.json"

esp_err_t spiffs_init(void);
esp_err_t spiffs_save_json(const char *data, size_t len);     // lưu /spiffs/forecast.json
esp_err_t spiffs_load_json(char **out, size_t *out_len);      // đọc file -> malloc buffer (nhớ free)

#endif