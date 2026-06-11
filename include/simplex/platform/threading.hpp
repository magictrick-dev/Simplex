#pragma once
#include <utils/defs.hpp>
#include <simplex/string_view.hpp>

namespace spx::mt
{

    class this_thread
    {
        public:
            static spx::string_view<char> get_name();
            static void set_name(spx::string_view<char> name);
    };

};