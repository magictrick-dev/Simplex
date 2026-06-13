#include <simplex/renderer/renderer_interface.hpp>
#include <simplex/platform/window.hpp>
#include <utils/logging.hpp>

RendererResultType spx::renderer_interface::
initialize(spx::window_interface *window_interface)
{

    this->window = window_interface;

    SIMPLEX_CHECK_PTR(window);
    if (!this->window->is_valid())
    {
        spx::logger::dispatch_error_log("GLFW Window instance is invalid. Did you initialize it?");
        return RendererResultType_WindowInvalid;
    }

    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();
    result = this->internal_initialize();
    END_CAPTURING_RENDER_EXCEPTIONS();

    spx::logger::dispatch_information_log("Renderer finished initialized.");
    return result;


}

RendererResultType spx::renderer_interface::
deinitialize()
{

    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();

    result = this->internal_deinitialize();

    END_CAPTURING_RENDER_EXCEPTIONS();

    spx::logger::dispatch_information_log("Renderer finished deinitialized.");
    return result;
}

RendererResultType spx::renderer_interface::
frame_begin()
{

    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();

    result = this->internal_frame_begin();

    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}

RendererResultType spx::renderer_interface::
frame_end()
{

    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();

    result = this->internal_frame_end();

    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}

RendererResultType spx::renderer_interface::
render_begin()
{

    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();

    result = this->internal_render_begin();

    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}

RendererResultType spx::renderer_interface::
render_end()
{
    
    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();

    result = this->internal_render_end();

    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}