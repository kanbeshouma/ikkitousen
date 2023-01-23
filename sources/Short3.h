#pragma once
#include<stdint.h>


class Short3
{
public:
    Short3() { x = 0; y = 0; z = 0; }
    inline Short3(int16_t x, int16_t y, int16_t z) { this->x = x, this->y = y, this->z = z; }
    inline Short3(const Short3& v) { this->x = v.x, this->y = v.y, this->z = v.z; }
public:
    int16_t x, y, z;

};