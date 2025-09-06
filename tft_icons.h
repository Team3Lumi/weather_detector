#ifndef ICONS_H
#define ICONS_H

#include <stdint.h>

typedef struct {
    const uint32_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t  dataSize;
} tImage;

extern const tImage sun1;
extern const tImage clouds;
extern const tImage rainy;

#endif // ICONS_H
