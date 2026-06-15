#pragma once
#include <utils/defs.hpp>
#define SIMPLEX_RENDERER_EXCEPTIONS 1
#if defined(SIMPLEX_RENDERER_EXCEPTIONS) && SIMPLEX_RENDERER_EXCEPTIONS == 1
#   include <stdexcept>
#endif

/// @brief  Various return results for various stages of the renderer. Any value
///         that isn't RendererResultType_OK is required to be inspected for further
///         handling. Some non-OK results aren't necessarily errors.
enum RendererResultType : uint32_t
{
    RendererResultType_OK,
    RendererResultType_WindowInvalid,
    RendererResultType_VulkanInstanceCreationFailed,
    RendererResultType_VulkanSurfaceCreationFailed,
    RendererResultType_VulkanPhysicalDeviceSelectionFailed,
    RendererResultType_VulkanLogicalDeviceCreationFailed,
};

namespace spx
{

    // NOTE(Chris): Exceptions are provided for debugging purposes and should be turned off
    //              and disabled so that they're not bogging up the performance pipeline.
    //              You're not meant to handle manual capturing of exceptions, but use the following
    //              macros that capture it. When the renderer exceptions is disabled, the code
    //              no longer evaluates.
#   if defined(SIMPLEX_RENDERER_EXCEPTIONS) && SIMPLEX_RENDERER_EXCEPTIONS == 1
        struct _renderer_exception : public std::runtime_error 
        { 
            _renderer_exception(const char *why) : runtime_error(why) 
            { 

            };
        };
#       define THROW_SIMPLEX_RENDERER_EXCEPTION(why) throw spx::_renderer_exception((why))
#       define BEGIN_CAPTURING_RENDER_EXCEPTIONS()                                              \
            try                                                                                 \
            {                                                                                   \
                do { } while(false)                                                                 
#       define END_CAPTURING_RENDER_EXCEPTIONS()                                                \
            }                                                                                   \
            catch (const _renderer_exception &e)                                                \
            {                                                                                   \
                spx::logger::dispatch_error_log("Caught rendering exception: {}", e.what());    \
            }                                                                                   \
            do { } while (false)
#   else
#       define THROW_SIMPLEX_RENDERER_EXCEPTION(why)
#       define BEGIN_CAPTURING_RENDER_EXCEPTIONS()
#       define END_CAPTURING_RENDER_EXCEPTIONS()
#   endif

};