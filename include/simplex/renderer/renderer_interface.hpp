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
            /// @brief Initializes the renderer and assigns it a window to bind to.
            /// @param window_interface The window interface to bind with.
            /// @return OK if initialization succeeded.
            ///
            /// Initialization is typically heavy-weighted, so if there's any I/O you
            /// want to do, it's best to dispatch asynchronously BEFORE calling this
            /// on the main thread.
            RendererResultType initialize(spx::window_interface* window_interface);

            /// @brief Deinitializes the renderer and clears any resources.
            /// @return OK if the deinitializations succeeded.
            ///
            /// Calling this isn't technically required for proper application shutdown, but
            /// if you're using Vulkan and have validation layers enabled, it will complain if
            /// you don't call this after the main-loop has exitted.
            RendererResultType deinitialize();

            /// @brief Prepares a frame for rendering.
            /// @return OK if preperation completed.
            RendererResultType frame_begin();

            /// @brief Cleans up a frame after rendering.
            /// @return OK if cleanup succeeded.
            RendererResultType frame_end();

            /// @brief Begins rendering.
            /// @return OK if rendering can proceed.
            RendererResultType render_begin();

            /// @brief Ends rendering.
            /// @return Unconditionally OK.
            RendererResultType render_end();

            /// @brief  Notifies the renderer that the window's drawable surface has changed size,
            ///         driving any resolution-dependent resources (e.g. the swapchain and its image
            ///         views) to be rebuilt against the new dimensions. Intended to be called in
            ///         response to a window resize event. Safe to call when the window has collapsed
            ///         to a zero-area surface (minimization); the implementation defers in that case.
            RendererResultType resize();

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
            inline virtual RendererResultType internal_resize()         = 0;
        
    };

};
