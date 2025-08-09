
#include "http.h"



static const char *TAG = "weather";

// static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
// {
//     return ESP_OK;
// }

#define RESP_MAX_BYTES   (64 * 1024)

void get_weather_forecast(void)
{
    const char *URL = WEATHER_API_URL;

    esp_http_client_config_t cfg = {
        .url               = URL,
        .method            = HTTP_METHOD_GET,
        .timeout_ms        = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size       = 2048,
        .buffer_size_tx    = 1024,
    };

    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) { ESP_LOGE(TAG, "init client fail"); return; }

    // Yêu cầu trả plain (không gzip) để JSON là text
    esp_http_client_set_header(c, "Accept-Encoding", "identity");
    esp_http_client_set_header(c, "User-Agent", "ESP32-IDF/WeatherStation");

    // QUAN TRỌNG: dùng open + fetch_headers + read (đừng chỉ perform rồi read)
    esp_err_t err = esp_http_client_open(c, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(c);
        return;
    }

    int64_t hdr_len = esp_http_client_fetch_headers(c); // có thể -1
    int status = esp_http_client_get_status_code(c);
    ESP_LOGI(TAG, "HTTP GET Status = %d, header_len = %lld", status, hdr_len);

    // Đọc body theo chunk
    size_t cap = 4096, len = 0;
    char *resp = (char*) malloc(cap);
    if (!resp) {
        ESP_LOGE(TAG, "OOM: malloc");
        esp_http_client_close(c);
        esp_http_client_cleanup(c);
        return;
    }

    while (1) {
        char tmp[1024];
        int r = esp_http_client_read(c, tmp, sizeof(tmp));
        if (r < 0) {
            ESP_LOGE(TAG, "read error");
            free(resp);
            esp_http_client_close(c);
            esp_http_client_cleanup(c);
            return;
        }
        if (r == 0) break; // hết dữ liệu

        if (len + r + 1 > cap) {
            size_t new_cap = cap * 2;
            while (new_cap < len + r + 1) new_cap *= 2;
            if (new_cap > RESP_MAX_BYTES) new_cap = RESP_MAX_BYTES;
            if (new_cap < len + r + 1) {
                ESP_LOGE(TAG, "Response too large (> %d bytes)", RESP_MAX_BYTES);
                free(resp);
                esp_http_client_close(c);
                esp_http_client_cleanup(c);
                return;
            }
            char *p = (char*) realloc(resp, new_cap);
            if (!p) {
                ESP_LOGE(TAG, "OOM: realloc");
                free(resp);
                esp_http_client_close(c);
                esp_http_client_cleanup(c);
                return;
            }
            resp = p; cap = new_cap;
        }
        memcpy(resp + len, tmp, r);
        len += r; resp[len] = '\0';
    }

    esp_http_client_close(c);
    esp_http_client_cleanup(c);

    ESP_LOGI(TAG, "Total bytes: %u", (unsigned)len);
    ESP_LOGI(TAG, "Peek: %.*s", (int)((len > 120) ? 120 : len), resp);

    if (len == 0) {  // không có body → thoát sớm
        ESP_LOGE(TAG, "Empty body");
        free(resp);
        return;
    }

    // Parse JSON an toàn theo độ dài
    cJSON *json = cJSON_ParseWithLength(resp, len);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        free(resp);
        return;
    }

    cJSON *daily = cJSON_GetObjectItemCaseSensitive(json, "daily");
    if (cJSON_IsObject(daily)) {
        cJSON *temp_max    = cJSON_GetObjectItemCaseSensitive(daily, "temperature_2m_max");
        cJSON *rain_prob   = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_probability_mean");
        cJSON *weathercode = cJSON_GetObjectItemCaseSensitive(daily, "weathercode");
        if (cJSON_IsArray(temp_max) && cJSON_IsArray(rain_prob) && cJSON_IsArray(weathercode)) {
            cJSON *t0 = cJSON_GetArrayItem(temp_max, 0);
            cJSON *r0 = cJSON_GetArrayItem(rain_prob, 0);
            cJSON *w0 = cJSON_GetArrayItem(weathercode, 0);
            if (cJSON_IsNumber(t0) && cJSON_IsNumber(r0) && cJSON_IsNumber(w0)) {
                ESP_LOGI(TAG, "--- Forecast (day 0) ---");
                ESP_LOGI(TAG, "Temp max: %.1f C", t0->valuedouble);
                ESP_LOGI(TAG, "Rain prob: %.1f %%", r0->valuedouble);
                ESP_LOGI(TAG, "Weather code: %d", w0->valueint);
            } else {
                ESP_LOGW(TAG, "Unexpected types in arrays[0]");
            }
        } else {
            ESP_LOGW(TAG, "Missing arrays in 'daily'");
        }
    } else {
        ESP_LOGW(TAG, "Missing 'daily' object");
    }

    cJSON_Delete(json);
    free(resp);
}
