// buttons.c
#include "buttons.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "buttons";

#define DEBOUNCE_MS  180  // chống dội

static QueueHandle_t s_btn_q = NULL;
static gpio_num_t s_pin_back = -1, s_pin_next = -1, s_pin_bl = -1;
static TickType_t s_last_back = 0, s_last_next = 0;

static inline bool _debounce_ok(TickType_t *last_tick, TickType_t now)
{
    if ((now - *last_tick) >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
        *last_tick = now;
        return true;
    }
    return false;
}

static void IRAM_ATTR btn_isr(void *arg)
{
    int pin = (int) arg;
    BaseType_t hp_task_woken = pdFALSE;
    TickType_t now = xTaskGetTickCountFromISR();

    if (pin == s_pin_back) {
        if (_debounce_ok(&s_last_back, now)) {
            btn_event_t evt = BTN_BACK_PRESS;
            xQueueSendFromISR(s_btn_q, &evt, &hp_task_woken);
        }
    } else if (pin == s_pin_next) {
        if (_debounce_ok(&s_last_next, now)) {
            btn_event_t evt = BTN_NEXT_PRESS;
            xQueueSendFromISR(s_btn_q, &evt, &hp_task_woken);
        }
    }

    if (hp_task_woken) portYIELD_FROM_ISR();
}

esp_err_t buttons_init(gpio_num_t pin_back, gpio_num_t pin_next, gpio_num_t pin_bl)
{
    s_pin_back = pin_back;
    s_pin_next = pin_next;
    s_pin_bl   = pin_bl;

    // Queue sự kiện
    if (!s_btn_q) s_btn_q = xQueueCreate(8, sizeof(btn_event_t));
    if (!s_btn_q) return ESP_ERR_NO_MEM;

    // Backlight
    if (s_pin_bl >= 0) {
        gpio_config_t o = {
            .pin_bit_mask = 1ULL << s_pin_bl,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = 0, .pull_down_en = 0, .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&o));
        gpio_set_level(s_pin_bl, 1); // Active HIGH
    }

    // Buttons: input pull-up, ngắt cạnh xuống (active LOW)
    gpio_config_t i = {
        .pin_bit_mask = (1ULL << s_pin_back) | (1ULL << s_pin_next),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1, .pull_down_en = 0,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&i));

    // ISR
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(s_pin_back, btn_isr, (void*) s_pin_back));
    ESP_ERROR_CHECK(gpio_isr_handler_add(s_pin_next, btn_isr, (void*) s_pin_next));

    ESP_LOGI(TAG, "Buttons ready: BACK=%d, NEXT=%d, BL=%d", s_pin_back, s_pin_next, s_pin_bl);
    return ESP_OK;
}

void buttons_set_backlight(bool on)
{
    if (s_pin_bl >= 0) gpio_set_level(s_pin_bl, on ? 1 : 0);
}

QueueHandle_t buttons_event_queue(void)
{
    return s_btn_q;
}

