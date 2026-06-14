#pragma once
#include <utils/defs.hpp>
#include <simplex/platform/window.hpp>
#include <simplex/renderer/renderer_utils.hpp>

namespace spx
{

    /// @brief      Provides the functionality necessary to interface with a specific renderer.
    ///
    ///             The renderer interface provides a front-end API which allows for layered inspection
    ///             of each phase of the rendering life-cycle. The interface itself performs and tracks
    ///             metrics and debugging information that the internal implementation can inspect.
    ///             In addition to that, the exception interface can catch and view the outputs. 
    class renderer_interface
    {

        public:
            inline          renderer_interface() = default;
            inline virtual ~renderer_interface() = default;

        public:
            RendererResultType initialize(spx::window_interface* window_interface);
            RendererResultType deinitialize();
            RendererResultType frame_begin();
            RendererResultType frame_end();
            RendererResultType render_begin();
            RendererResultType render_end();

            inline spx::window_interface* get_window() { return this->window; }

        private:
            spx::window_interface* window = NULL;

        protected:
            inline virtual RendererResultType internal_initialize()     = 0;
            inline virtual RendererResultType internal_frame_begin()    = 0;
            inline virtual RendererResultType internal_render_begin()   = 0;
            inline virtual RendererResultType internal_render_end()     = 0;
            inline virtual RendererResultType internal_frame_end()      = 0;
            inline virtual RendererResultType internal_deinitialize()   = 0;
        
    };

};
