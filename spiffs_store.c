#include "spiffs_store.h"

static const char *TAG = "spiffs";

esp_err_t spiffs_init(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",     // khớp tên trong partitions.csv
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(err));
        return err;
    }

    size_t total=0, used=0;
    err = esp_spiffs_info(conf.partition_label, &total, &used);
    if (err == ESP_OK)
        ESP_LOGI(TAG, "SPIFFS total=%u, used=%u", (unsigned)total, (unsigned)used);
    return ESP_OK;
}

esp_err_t spiffs_save_json(const char *data, size_t len)
{
    if (!data || !len) return ESP_ERR_INVALID_ARG;
    FILE *f = fopen(FORECAST_PATH, "w");
    if (!f) { ESP_LOGE(TAG, "open %s fail", FORECAST_PATH); return ESP_FAIL; }
    size_t w = fwrite(data, 1, len, f);
    fclose(f);
    if (w != len) { ESP_LOGE(TAG, "write short (%u/%u)", (unsigned)w, (unsigned)len); return ESP_FAIL; }
    ESP_LOGI(TAG, "saved %s (%u bytes)", FORECAST_PATH, (unsigned)len);
    return ESP_OK;
}

esp_err_t spiffs_load_json(char **out, size_t *out_len)
{
    if (!out || !out_len) return ESP_ERR_INVALID_ARG;
    *out = NULL; *out_len = 0;

    FILE *f = fopen(FORECAST_PATH, "r");
    if (!f) { ESP_LOGW(TAG, "no cache file %s", FORECAST_PATH); return ESP_ERR_NOT_FOUND; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0) { fclose(f); return ESP_ERR_INVALID_SIZE; }
    rewind(f);

    char *buf = (char*) malloc(sz + 1);
    if (!buf) { fclose(f); return ESP_ERR_NO_MEM; }
    size_t r = fread(buf, 1, sz, f);
    fclose(f);
    buf[r] = '\0';

    if (r == 0) { free(buf); return ESP_FAIL; }
    *out = buf;
    *out_len = r;
    ESP_LOGI(TAG, "loaded %s (%u bytes)", FORECAST_PATH, (unsigned)r);
    return ESP_OK;
}