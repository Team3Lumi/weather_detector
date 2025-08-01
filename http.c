#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"


static const char* TAG = "HTTP_EXAMPLE";
#define GET_URL "http://httpbin.org/get"

static void http_get_task(void *pvParameters)
{
    esp_http_client_config_t config = {
        .url = GET_URL,                    // Địa chỉ URL API
        .method = HTTP_METHOD_GET,          // Phương thức GET
        .timeout_ms = 5000,                 // Timeout cho request
    };

    // Khởi tạo client với cấu hình trên
    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        vTaskDelete(NULL);
        return;
    }

    // Thực hiện HTTP GET
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        // Nếu thành công, lấy status code và độ dài nội dung
        int status_code = esp_http_client_get_status_code(client);
        int content_len = esp_http_client_get_content_length(client);
        
        ESP_LOGI(TAG, "HTTP GET Status = %d, Content Length = %d", status_code, content_len);

        // Đọc nội dung trả về (nếu có)
        if (content_len > 0 && content_len < 1024) {
            char *buffer = malloc(content_len + 1);
            if (buffer) {
                int read_len = esp_http_client_read_response(client, buffer, content_len);
                if (read_len >= 0) {
                    buffer[read_len] = '\0';  // Đảm bảo kết thúc chuỗi
                    ESP_LOGI(TAG, "Response data: %s", buffer);
                }
                free(buffer);  // Giải phóng bộ nhớ sau khi sử dụng
            }
        }
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // Dọn dẹp tài nguyên của client
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);  // Xóa task sau khi hoàn thành
}

// Hàm khởi động task HTTP GET
void http_start_task(void)
{
    xTaskCreate(http_get_task, "http_get_task", 8 * 1024, NULL, 5, NULL);
}