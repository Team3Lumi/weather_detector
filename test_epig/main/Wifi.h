#ifndef WIFI_H
#define WIFI_H

// Tên mạng Wi-Fi (SSID)
#define WIFI_SSID      "KTTH 2"

// Mật khẩu Wi-Fi
#define WIFI_PASS      "0983014568"

#include "esp_err.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"

void wifi_init_sta(void);

#endif // WIFI_H
