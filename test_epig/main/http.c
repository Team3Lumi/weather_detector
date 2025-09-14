
#include "http.h"
#include "spiffs_store.h"

#include "cJSON.h"

static const char *TAG = "weather";

// Map WMO weathercode -> icon đơn giản
static int icon_from_code(int code) {
    if (code == 0 || code == 1) return TFT_ICON_SUNNY;
    if (code == 2 || code == 3) return TFT_ICON_CLOUDY;
    return TFT_ICON_RAIN;
}

static void fill_day(tft_day_t *d,
                     const char *date, double tmax, double rain_prob, int wcode)
{
    if (!d) return;
    // day "YYYY-MM-DD"
    if (date) {
        strncpy(d->day, date, sizeof(d->day)-1);
        d->day[sizeof(d->day)-1] = '\0';
    } else {
        d->day[0] = '\0';
    }
    d->temp_c = (int)tmax;

    // struct của bạn trong tft_types.h dùng field nào cho phần trăm:
    // nếu là rain_pct thì sẽ đúng nghĩa; nếu là humidity_pct thì ta reuse để hiển thị % (không ảnh hưởng UI)
    #ifdef TFT_DAY_HAS_RAIN_PCT
      d->rain_pct = (int)rain_prob;
    #else
      d->humidity_pct = (int)rain_prob;
    #endif

    d->icon = icon_from_code(wcode);
}

static esp_err_t parse_json_to_days(const char *json, size_t len,
                                    tft_day_t out[FORECAST_DAYS], int *out_count)
{
    if (!json || !len || !out || !out_count) return ESP_ERR_INVALID_ARG;

    cJSON *root = cJSON_ParseWithLength(json, len);
    if (!root) return ESP_FAIL;

    cJSON *daily = cJSON_GetObjectItemCaseSensitive(root, "daily");
    if (!cJSON_IsObject(daily)) { cJSON_Delete(root); return ESP_FAIL; }

    cJSON *times       = cJSON_GetObjectItemCaseSensitive(daily, "time");
    cJSON *temp_max    = cJSON_GetObjectItemCaseSensitive(daily, "temperature_2m_max");
    cJSON *rain_prob   = cJSON_GetObjectItemCaseSensitive(daily, "precipitation_probability_mean");
    cJSON *weathercode = cJSON_GetObjectItemCaseSensitive(daily, "weathercode");
    if (!cJSON_IsArray(times) || !cJSON_IsArray(temp_max) ||
        !cJSON_IsArray(rain_prob) || !cJSON_IsArray(weathercode)) {
        cJSON_Delete(root); return ESP_FAIL;
    }

    int n = cJSON_GetArraySize(times);
    n = (n < cJSON_GetArraySize(temp_max))    ? n : cJSON_GetArraySize(temp_max);
    n = (n < cJSON_GetArraySize(rain_prob))   ? n : cJSON_GetArraySize(rain_prob);
    n = (n < cJSON_GetArraySize(weathercode)) ? n : cJSON_GetArraySize(weathercode);
    if (n > FORECAST_DAYS) n = FORECAST_DAYS;

    for (int i = 0; i < n; ++i) {
        cJSON *ti = cJSON_GetArrayItem(times, i);
        cJSON *tm = cJSON_GetArrayItem(temp_max, i);
        cJSON *rp = cJSON_GetArrayItem(rain_prob, i);
        cJSON *wc = cJSON_GetArrayItem(weathercode, i);
        const char *date = (cJSON_IsString(ti) && ti->valuestring) ? ti->valuestring : NULL;
        double tmax = cJSON_IsNumber(tm) ? tm->valuedouble : 0.0;
        double rprob= cJSON_IsNumber(rp) ? rp->valuedouble : 0.0;
        int wcode   = cJSON_IsNumber(wc) ? wc->valueint   : 0;
        fill_day(&out[i], date, tmax, rprob, wcode);
    }
    *out_count = n;
    cJSON_Delete(root);
    return (n > 0) ? ESP_OK : ESP_FAIL;
}

esp_err_t forecast_load_from_spiffs(tft_day_t out[FORECAST_DAYS], int *out_count)
{
    if (!out || !out_count) return ESP_ERR_INVALID_ARG;
    char *buf = NULL; size_t blen = 0;
    esp_err_t er = spiffs_load_json(&buf, &blen);
    if (er != ESP_OK) return er;

    er = parse_json_to_days(buf, blen, out, out_count);
    free(buf);
    return er;
}

esp_err_t forecast_fetch_and_cache(tft_day_t out[FORECAST_DAYS], int *out_count)
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
    if (!c) { ESP_LOGE(TAG, "init client fail"); return ESP_FAIL; }

    esp_http_client_set_header(c, "Accept-Encoding", "identity");
    esp_http_client_set_header(c, "User-Agent", "ESP32-IDF/WeatherStation");

    esp_err_t err = esp_http_client_open(c, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(c);
        return err;
    }

    (void)esp_http_client_fetch_headers(c);
    int status = esp_http_client_get_status_code(c);
    if (status != 200) {
        ESP_LOGE(TAG, "HTTP status %d", status);
        esp_http_client_close(c);
        esp_http_client_cleanup(c);
        return ESP_FAIL;
    }

    size_t cap = 4096, len = 0;
    char *resp = (char*) malloc(cap);
    if (!resp) { esp_http_client_close(c); esp_http_client_cleanup(c); return ESP_ERR_NO_MEM; }

    while (1) {
        char tmp[1024];
        int r = esp_http_client_read(c, tmp, sizeof(tmp));
        if (r < 0)  { free(resp); esp_http_client_close(c); esp_http_client_cleanup(c); return ESP_FAIL; }
        if (r == 0) break;
        if (len + r + 1 > cap) {
            size_t new_cap = cap * 2;
            while (new_cap < len + r + 1) new_cap *= 2;
            if (new_cap > RESP_MAX_BYTES) new_cap = RESP_MAX_BYTES;
            if (new_cap < len + r + 1) { free(resp); esp_http_client_close(c); esp_http_client_cleanup(c); return ESP_ERR_NO_MEM; }
            char *p = (char*) realloc(resp, new_cap);
            if (!p) { free(resp); esp_http_client_close(c); esp_http_client_cleanup(c); return ESP_ERR_NO_MEM; }
            resp = p; cap = new_cap;
        }
        memcpy(resp + len, tmp, r);
        len += r; resp[len] = '\0';
    }

    esp_http_client_close(c);
    esp_http_client_cleanup(c);

    if (len == 0) { free(resp); return ESP_FAIL; }

    // Lưu cache
    (void) spiffs_save_json(resp, len);

    // Nếu người gọi muốn nhận luôn mảng ngày thì parse ra
    esp_err_t perr = ESP_OK;
    if (out && out_count) {
        perr = parse_json_to_days(resp, len, out, out_count);
    }

    free(resp);
    return perr;
}

// Giữ hàm cũ để không phá code cũ
void get_weather_forecast(void) {
    (void) forecast_fetch_and_cache(NULL, NULL);
}