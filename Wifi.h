#ifndef WIFI_H
#define WIFI_H

// Tên mạng Wi-Fi (SSID)
#define WIFI_SSID      "TP-LINK_F302"

// Mật khẩu Wi-Fi
#define WIFI_PASS      ""

#include "esp_err.h"

void wifi_init_sta(void);

#endif // WIFI_H
