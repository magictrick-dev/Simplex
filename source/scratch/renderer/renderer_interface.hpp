#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <scratch/window.hpp>

#define SIMPLEX_RENDERER_EXCEPTIONS 1
#if defined(SIMPLEX_RENDERER_EXCEPTIONS) && SIMPLEX_RENDERER_EXCEPTIONS == 1
#   include <stdexcept>
#endif

namespace spx
{

    /// @brief  Various return results for various stages of the renderer. Any value
    ///         that isn't RendererResultType_OK is required to be inspected for further
    ///         handling. Some non-OK results aren't necessarily errors.
    enum RendererResultType : uint32_t
    {
        RendererResultType_OK,
        RendererResultType_InitializationFailed,
        RendererResultType_DeinitializationFailed,
        RendererResultType_WindowInvalid,
    };

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
            initialize(spx::window window)
            {

                this->window = window;
                if (this->window.handle == NULL)
                {
                    return RendererResultType_WindowInvalid;
                }

                RendererResultType result { };
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();
                result = this->internal_initialize();
                END_CAPTURING_RENDER_EXCEPTIONS();

                spx::logger::dispatch_information_log("Renderer finished initialized.");
                return result;

            }

            inline RendererResultType 
            deinitialize()
            {

                RendererResultType result { };
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();

                result = this->internal_deinitialize();

                END_CAPTURING_RENDER_EXCEPTIONS();

                spx::logger::dispatch_information_log("Renderer finished deinitialized.");
                return result;
            }

            inline RendererResultType 
            frame_begin()
            {

                RendererResultType result { };
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();

                result = this->internal_frame_begin();

                END_CAPTURING_RENDER_EXCEPTIONS();

                return result;
            }

            inline RendererResultType 
            frame_end()
            {

                RendererResultType result { };
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();

                result = this->internal_frame_end();

                END_CAPTURING_RENDER_EXCEPTIONS();

                return result;
            }
            
            inline RendererResultType 
            render_begin()
            {

                RendererResultType result { };
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();

                result = this->internal_render_begin();

                END_CAPTURING_RENDER_EXCEPTIONS();

                return result;
            }

            inline RendererResultType 
            render_end()
            {
                
                RendererResultType result { };
                BEGIN_CAPTURING_RENDER_EXCEPTIONS();

                result = this->internal_render_end();

                END_CAPTURING_RENDER_EXCEPTIONS();

                return result;
            }

            inline spx::window get_window() { return this->window; }

        private:
            spx::window window;

        protected:
            inline virtual RendererResultType internal_initialize()     = 0;
            inline virtual RendererResultType internal_frame_begin()    = 0;
            inline virtual RendererResultType internal_render_begin()   = 0;
            inline virtual RendererResultType internal_render_end()     = 0;
            inline virtual RendererResultType internal_frame_end()      = 0;
            inline virtual RendererResultType internal_deinitialize()   = 0;
        
    };

};