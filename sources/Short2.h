#pragma once
#include<stdint.h>


class Short2
{
public:
    Short2() { x = 0; y = 0; }
    inline Short2(int16_t x, int16_t y) { this->x = x, this->y = y; }
    inline Short2(const Short2& v) { this->x = v.x, this->y = v.y; }
public:
    int16_t x, y;

};