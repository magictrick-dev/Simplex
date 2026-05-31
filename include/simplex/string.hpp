#pragma once
#include <utils/defs.hpp>

namespace spx
{

    class string
    {

        public:
            inline string() = delete;
            inline string(const char *string) : buffer(string), count(strlen(string)) { }
            inline ~string() = default;

            inline const char* c_str() const { return this->buffer; }
            inline size_t length() const { return this->count; }

        private:
            const char *buffer;
            size_t count;

    };

};
