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

        uint8_t* pix = (buffer + (4 * pos.x) + (size.x * 4 * pos.y));

        pix[0] = (colour.x);
        pix[1] = (colour.y);
        pix[2] = (colour.z);
        return;
    }
};
