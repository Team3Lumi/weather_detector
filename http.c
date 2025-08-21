
#include "http.h"
#include "spiffs_store.h"

static const char *TAG = "weather";

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

    // Mở kết nối và đọc theo stream (ổn định với chunked)
    esp_err_t err = esp_http_client_open(c, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(c);
        return;
    }

    (void)esp_http_client_fetch_headers(c);
    int status = esp_http_client_get_status_code(c);
    ESP_LOGI(TAG, "HTTP GET Status = %d", status);

    // Đọc body
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
    // (Tuỳ chọn) Lưu JSON vào SPIFFS để khởi động sau đọc ra ngay:
    spiffs_save_json(resp, len);

    if (len == 0) {
        ESP_LOGE(TAG, "Empty body");
        free(resp);
        return;
    }

    // Parse JSON đủ 7 ngày
    cJSON *json = cJSON_ParseWithLength(resp, len);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        free(resp);
        return;
    }

    cJSON *daily = cJSON_GetObjectItemCaseSensitive(json, "daily");
    if (!cJSON_IsObject(daily)) {
        ESP_LOGW(TAG, "Missing 'daily' object");
        cJSON_Delete(json); free(resp);
        return;
    }

    cJSON *times      = cJSON_GetObjectItemCaseSensitive(daily, "time");
    cJSON *temp_max   = cJSON_GetObjectItemCaseSensitive(daily, "temperature_2m_max");
    cJSON *rain_prob  = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_probability_mean");
    cJSON *weathercode= cJSON_GetObjectItemCaseSensitive(daily, "weathercode");

    if (!cJSON_IsArray(times) || !cJSON_IsArray(temp_max) ||
        !cJSON_IsArray(rain_prob) || !cJSON_IsArray(weathercode)) {
        ESP_LOGW(TAG, "Missing expected arrays in 'daily'");
        cJSON_Delete(json); free(resp);
        return;
    }

    int n = cJSON_GetArraySize(times);
    // đảm bảo đồng bộ các mảng
    n = (n < cJSON_GetArraySize(temp_max))    ? n : cJSON_GetArraySize(temp_max);
    n = (n < cJSON_GetArraySize(rain_prob))   ? n : cJSON_GetArraySize(rain_prob);
    n = (n < cJSON_GetArraySize(weathercode)) ? n : cJSON_GetArraySize(weathercode);
    if (n > FORECAST_DAYS) n = FORECAST_DAYS;

    ESP_LOGI(TAG, "=== 7-day forecast (up to %d days) ===", n);
    for (int i = 0; i < n; ++i) {
        cJSON *ti = cJSON_GetArrayItem(times, i);
        cJSON *tm = cJSON_GetArrayItem(temp_max, i);
        cJSON *rp = cJSON_GetArrayItem(rain_prob, i);
        cJSON *wc = cJSON_GetArrayItem(weathercode, i);

        const char *date = (cJSON_IsString(ti) && ti->valuestring) ? ti->valuestring : "N/A";
        double tmax = cJSON_IsNumber(tm) ? tm->valuedouble : 0.0;
        double rprob= cJSON_IsNumber(rp) ? rp->valuedouble : 0.0;
        int wcode   = cJSON_IsNumber(wc) ? wc->valueint   : 0;

        ESP_LOGI(TAG, "[%d] %s | Tmax=%.1f C | Rain=%.1f %% | code=%d",
                 i, date, tmax, rprob, wcode);

        // TODO: nếu cần, copy dữ liệu vào struct/array để đẩy ra TFT hay lưu file nhị phân
    }

    cJSON_Delete(json);
    free(resp);
}