set(TFT_SRCS
    #"components/tft/cloud.c"
    #"components/tft/rainy.c"
    #"components/tft/sun.c"
    "components/tft/icons.c"   
    "components/tft/GUI.c"
    "components/tft/lcd.c"
    # nếu có thêm tft_display.c / tft_icons.c mà bạn muốn build cùng main,
    # liệt kê vào đây hoặc để cạnh main.c
)

idf_component_register(
    SRCS
        "main.c"
        "wifi.c"
        "http.c"
        "spiffs_store.c"
        "tft_display.c"
        "buttons.c"
        ${TFT_SRCS}
    INCLUDE_DIRS
        "."
        "components/tft"         # để #include "lcd.h" ... hoạt động
    REQUIRES
        esp_http_client spiffs driver nvs_flash esp_netif esp_wifi mbedtls
    PRIV_REQUIRES
        json
)
