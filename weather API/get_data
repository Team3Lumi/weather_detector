#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"

#define WEATHER_API_URL "https://api.open-meteo.com/v1/forecast?latitude=21.0&longitude=105.75&daily=temperature_2m_max,precipitation_probability_mean,weathercode&timezone=auto"

static const char *TAG = "weather";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    return ESP_OK;
}

void get_weather_forecast()
{
    esp_http_client_config_t config = {
        .url = WEATHER_API_URL,
        .event_handler = _http_event_handler,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", status_code, content_length);

        char *buffer = malloc(content_length + 1);
        if (buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory");
            return;
        }

        esp_http_client_read(client, buffer, content_length);
        buffer[content_length] = '\0';

        // Parse JSON response
        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            ESP_LOGE(TAG, "Failed to parse JSON");
            free(buffer);
            return;
        }

        // Parse daily fields
        cJSON *daily = cJSON_GetObjectItem(json, "daily");
        if (daily) {
            cJSON *temp_max = cJSON_GetObjectItem(daily, "temperature_2m_max");
            cJSON *rain_prob = cJSON_GetObjectItem(daily, "precipitation_probability_mean");
            cJSON *weathercode = cJSON_GetObjectItem(daily, "weathercode");
            if (temp_max && rain_prob && weathercode) {
                ESP_LOGI(TAG, "--- Forecast for today ---");
                ESP_LOGI(TAG, "Temp max: %.1f C", cJSON_GetArrayItem(temp_max, 0)->valuedouble);
                ESP_LOGI(TAG, "Rain prob: %.1f %%", cJSON_GetArrayItem(rain_prob, 0)->valuedouble);
                ESP_LOGI(TAG, "Weather code: %d", cJSON_GetArrayItem(weathercode, 0)->valueint);
            }
        }

        cJSON_Delete(json);
        free(buffer);
    } else {
        ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
