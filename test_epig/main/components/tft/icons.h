#ifndef ICONS_H
#define ICONS_H

#include <stdint.h>

#define TRANSPARENT_COLOR 0x4389

typedef struct {
    const uint32_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t  dataSize;
} tImage;

extern const tImage sun;
extern const tImage cloud;
extern const tImage rainy;


#endif // ICONS_H