#pragma once
#include <utils/defs.hpp>

namespace spx
{

    struct material
    {
        uint32_t texture;
        uint32_t normal;
        uint32_t bump;
        uint32_t specular;
        real32_t Ka;
        real32_t Kd;
        real32_t Ks;
        real32_t Ke;
    };

}