#pragma once
#include <utils/defs.hpp>

enum RenderFormatType
{
    RenderFormatType_RGBSingleBuffer,
    RenderFormatType_RGBDoubleBuffer,
};

class IRenderEngine
{
    public:
        inline IRenderEngine(int32_t width, int32_t height, RenderFormatType type)
            : framebuffer_width(width), framebuffer_height(height), framebuffer_format(type)
        {

        }

        virtual inline bool initialize() = 0;
        virtual inline bool shutdown() = 0;

        inline int32_t get_width() { return this->framebuffer_width; }
        inline int32_t get_height() { return this->framebuffer_height; }
        inline real32_t get_aspect_ratio() { return (real32_t)this->framebuffer_width / (real32_t)this->framebuffer_height; }
        inline RenderFormatType get_render_format() { return this->framebuffer_format; }

    private:
        int32_t framebuffer_width;
        int32_t framebuffer_height;
        RenderFormatType framebuffer_format;

};
