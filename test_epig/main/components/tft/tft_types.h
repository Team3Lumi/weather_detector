#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TFT_ICON_SUNNY = 0,
  TFT_ICON_CLOUDY = 1, 
  TFT_ICON_RAIN = 2
} tft_icon_t;

typedef struct {
  char day[11];     
  tft_icon_t  icon;    // SUNNY/CLOUDY/RAIN
  int temp_c;          
  int humidity_pct;    
} tft_day_t;

#ifdef __cplusplus
}
#endif
