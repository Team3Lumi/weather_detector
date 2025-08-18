#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "esp_spiffs.h"
#include "cJSON.h"

static const char *TAG = "SPIFFS_WEATHER";

#define MAX_FILE_PATH 64
#define CLEANUP_DAYS  5

// Hàm mount SPIFFS
void spiffs_init(void) {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 10,
      .format_if_mount_failed = true
    };

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));
    size_t total = 0, used = 0;
    ESP_ERROR_CHECK(esp_spiffs_info(NULL, &total, &used));
    ESP_LOGI(TAG, "SPIFFS mounted, total=%d, used=%d", total, used);
}

// Lưu dữ liệu 1 ngày (JSON)
esp_err_t save_day_data(const char *date, const char *json_data, time_t now) {
    char path[MAX_FILE_PATH];
    snprintf(path, sizeof(path), "/spiffs/day_%s.json", date);

    // Parse JSON để thêm last_access
    cJSON *root = cJSON_Parse(json_data);
    if (!root) return ESP_FAIL;
    cJSON_AddNumberToObject(root, "last_access", (double)now);

    char *final_str = cJSON_Print(root);

    FILE *f = fopen(path, "w");
    if (!f) {
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    fprintf(f, "%s", final_str);
    fclose(f);

    ESP_LOGI(TAG, "Saved data for %s", date);
    cJSON_free(final_str);
    cJSON_Delete(root);
    return ESP_OK;
}

// Đọc dữ liệu của ngày
char* load_day_data(const char *date, time_t now) {
    char path[MAX_FILE_PATH];
    snprintf(path, sizeof(path), "/spiffs/day_%s.json", date);

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc(size+1);
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    // Parse JSON để update last_access
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        cJSON_ReplaceItemInObject(root, "last_access", cJSON_CreateNumber((double)now));
        char *new_str = cJSON_Print(root);

        // Ghi lại file
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "%s", new_str);
            fclose(f);
        }

        cJSON_free(new_str);
        cJSON_Delete(root);
    }

    return buf; // Trả về dữ liệu cũ (người dùng free)
}

// Cleanup: xóa file >5 ngày không truy cập
void cleanup_old_data(time_t now) {
    DIR *dir = opendir("/spiffs");
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, "day_")) {
            char path[MAX_FILE_PATH];
            snprintf(path, sizeof(path), "/spiffs/%s", ent->d_name);

            FILE *f = fopen(path, "r");
            if (!f) continue;

            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            rewind(f);

            char *buf = malloc(size+1);
            fread(buf, 1, size, f);
            buf[size] = '\0';
            fclose(f);

            cJSON *root = cJSON_Parse(buf);
            if (root) {
                cJSON *last = cJSON_GetObjectItem(root, "last_access");
                if (cJSON_IsNumber(last)) {
                    double last_time = last->valuedouble;
                    double diff_days = (now - last_time) / 86400.0;

                    if (diff_days > CLEANUP_DAYS) {
                        remove(path);
                        ESP_LOGW(TAG, "Deleted old data: %s", path);
                    }
                }
                cJSON_Delete(root);
            }
            free(buf);
        }
    }
    closedir(dir);
}

//main
void app_main(void) {
    spiffs_init();

    time_t now = 1723968000; // ví dụ: timestamp hiện tại

    // Giả sử có JSON trả về từ API
    const char *json_mock = "{\"temp\":33, \"rain\":40}";
    save_day_data("20250818", json_mock, now);

    // Load lại
    char *data = load_day_data("20250818", now + 86400);
    if (data) {
        ESP_LOGI(TAG, "Read: %s", data);
        free(data);
    }

    // Cleanup (sau 6 ngày)
    cleanup_old_data(now + 6*86400);
}
