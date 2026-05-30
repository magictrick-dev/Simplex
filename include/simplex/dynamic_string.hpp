#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>

namespace spx
{

    // TODO(Chris): Incomplete implementation.
    class dynamic_string
    {

        public:
            inline dynamic_string() = default;
            inline ~dynamic_string() = default;

        private:

        private:
            char *elements;
            size_t count;

    };

};