#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>
#define SIMPLEX_RENDERER_EXCEPTIONS 1
#if defined(SIMPLEX_RENDERER_EXCEPTIONS) && SIMPLEX_RENDERER_EXCEPTIONS == 1
#   include <stdexcept>
#endif

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
#       define BEGIN_CAPTURING_RENDER_EXCEPTIONS() try {
#       define END_CAPTURING_RENDER_EXCEPTIONS()                                                \
            } catch (const _renderer_exception &e) {                                            \
                spx::logger::dispatch_error_log("Caught rendering exception: {}", e.what());    \
            }
#   else
#       define THROW_SIMPLEX_RENDERER_EXCEPTION(why)
#       define BEGIN_CAPTURING_RENDER_EXCEPTIONS()
#       define END_CAPTURING_RENDER_EXCEPTIONS()
#   endif

    /// @brief  Various return results for various stages of the renderer. Any value
    ///         that isn't RendererResultType_OK is required to be inspected for further
    ///         handling. Some non-OK results aren't necessarily errors.
    enum RendererResultType : uint32_t
    {
        RendererResultType_OK,
        RendererResultType_InitializationFailed,
        RendererResultType_DeinitializationFailed,
    };

    /// @brief 
    enum RendererStepType : uint32_t
    {
        RendererStepType_Uninitialized,
        RendererStepType_Initialization,
        RendererStepType_FrameBegin,
        RendererStepType_RenderBegin,
        RendererStepType_RenderEnd,
        RendererStepType_FrameEnd,
        RendererStepType_Deinitialization,
    };

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
            inline RendererResultType 
            initialize()
            {

                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                const RendererResultType result = this->internal_initialize();
                END_CAPTURING_RENDER_EXCEPTIONS();

            }

            inline RendererResultType 
            deinitialize()
            {

                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                const RendererResultType result = this->internal_deinitialize();
                END_CAPTURING_RENDER_EXCEPTIONS();

            }

            inline RendererResultType 
            frame_begin()
            {

                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                const RendererResultType result = this->internal_frame_begin();
                END_CAPTURING_RENDER_EXCEPTIONS();

            }

            inline RendererResultType 
            frame_end()
            {

                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                const RendererResultType result = this->internal_frame_end();
                END_CAPTURING_RENDER_EXCEPTIONS();

            }
            
            inline RendererResultType 
            render_begin()
            {

                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                const RendererResultType result = this->internal_render_begin();
                END_CAPTURING_RENDER_EXCEPTIONS();

            }

            inline RendererResultType 
            render_end()
            {
                
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                const RendererResultType result = this->internal_render_end();
                END_CAPTURING_RENDER_EXCEPTIONS();

            }

        public:

        protected:
            inline virtual RendererResultType internal_initialize()     = 0;
            inline virtual RendererResultType internal_frame_begin()    = 0;
            inline virtual RendererResultType internal_render_begin()   = 0;
            inline virtual RendererResultType internal_render_end()     = 0;
            inline virtual RendererResultType internal_frame_end()      = 0;
            inline virtual RendererResultType internal_deinitialize()   = 0;
        
    };

};