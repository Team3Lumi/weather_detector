#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TFT_ICON_SUNNY = 0,
  TFT_ICON_CLOUDY,
  TFT_ICON_RAIN
} tft_icon_t;

typedef struct {
  const char* day;     
  tft_icon_t  icon;    // SUNNY/CLOUDY/RAIN
  int temp_c;          
  int humidity_pct;    
} tft_day_t;

#ifdef __cplusplus
}
#endif
