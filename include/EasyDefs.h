#pragma once

#include <stdint.h>

struct vec3{
    int x;
    int y;
    int z;
};

struct vec2{
    int x;
    int y;
};

struct PixelImage
{
    vec2 size;
    uint8_t* buffer;

    inline void SetPixel(vec2 pos, vec3 colour){
        if(!buffer){
            return;
        }

        if(pos.x >= size.x || pos.y >= size.y){
            return;
        }

        uint32_t* pix = (uint32_t*)(buffer + (4 * pos.x) + (size.x * 4 * pos.y));

        *pix = (colour.x << 24);
        *pix |= (colour.z << 16);
        *pix |= (colour.y << 8);
        return;
    }
};
