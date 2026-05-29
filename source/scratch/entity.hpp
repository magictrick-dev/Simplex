#pragma once
#include <utils/defs.hpp>

union entity_t
{

    uint64_t handle;

    struct
    {

        uint32_t identifier;
        uint16_t generation;
        uint16_t flags;

    };

    inline operator uint64_t()                          { return handle;                        }
    inline entity_t& operator=(uint64_t handle)         { this->handle = handle; return *this;  }
    inline bool operator==(const entity_t&other) const  { return this->handle == other.handle;  }
    inline bool operator!=(const entity_t &other) const { return !(*this == other);             }

};
