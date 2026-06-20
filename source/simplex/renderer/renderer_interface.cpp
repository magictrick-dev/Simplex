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

    // NOTE(Chris): Since exceptions will not alter this value, we prematurely set it to an invalid
    //              exception value case which indicates that IF exceptions are enabled and they ARE
    //              caught, the exception information will be output case of WHAT happened and still
    //              have a valid exit-case.
    RendererResultType result = RendererResultType_RendererExceptionState;

    BEGIN_CAPTURING_RENDER_EXCEPTIONS();
    result = this->internal_initialize();
    END_CAPTURING_RENDER_EXCEPTIONS();


    if (result == RendererResultType_OK)
        spx::logger::dispatch_information_log("Renderer finished initialization successfully.");
    else
        spx::logger::dispatch_error_log("Renderer failed initialization.");

    return result;


}

RendererResultType spx::renderer_interface::
deinitialize()
{

    RendererResultType result = RendererResultType_RendererExceptionState;

    BEGIN_CAPTURING_RENDER_EXCEPTIONS();
    result = this->internal_deinitialize();
    END_CAPTURING_RENDER_EXCEPTIONS();

    spx::logger::dispatch_information_log("Renderer finished deinitialized.");
    return result;
}

RendererResultType spx::renderer_interface::
frame_begin()
{

    RendererResultType result = RendererResultType_RendererExceptionState;
    
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();
    result = this->internal_frame_begin();
    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}

RendererResultType spx::renderer_interface::
frame_end()
{

    RendererResultType result = RendererResultType_RendererExceptionState;

    BEGIN_CAPTURING_RENDER_EXCEPTIONS();
    result = this->internal_frame_end();
    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}

RendererResultType spx::renderer_interface::
render_begin()
{

    RendererResultType result = RendererResultType_RendererExceptionState;

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

RendererResultType spx::renderer_interface::
resize()
{

    RendererResultType result { };
    BEGIN_CAPTURING_RENDER_EXCEPTIONS();

    result = this->internal_resize();

    END_CAPTURING_RENDER_EXCEPTIONS();

    return result;
}