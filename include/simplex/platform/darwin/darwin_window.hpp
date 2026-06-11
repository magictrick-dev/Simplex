#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>
#include <simplex/platform/window.hpp>

namespace spx
{

    /// @brief  Provides a way of creating a win32 window.
    class darwin_window : public window_interface
    {

        public:
            inline          darwin_window() = default;
            inline virtual ~darwin_window() = default;

            inline virtual bool
            create(spx::string_view<char> window_title, uint32_t width, uint32_t height) override
            {

            }

            inline virtual void
            destroy() override
            {

            }

            inline virtual void
            poll_events() override
            {

            }

            inline virtual void
            hide() override
            {

            }

            inline virtual void
            show() override
            {

            }

            inline virtual void
            maximize() override
            {

            }

            inline virtual void
            borderless() override
            {

            }

            inline virtual void
            normalize() override
            {

            }

            inline virtual void
            lock_resizing() override
            {

            }
            inline virtual void
            unlock_resizing() override
            {

            }

            inline virtual bool
            is_valid() const override
            {

            }

            inline virtual bool
            should_close() const override
            {

            }
            
            inline virtual int32_t
            get_width() const override
            {

            }

            inline virtual int32_t
            get_height() const override
            {

            }

            inline virtual real32_t
            get_aspect_ratio() const override
            {

            }

            inline virtual bool
            is_visible() const override
            {

            }

            inline virtual bool
            is_hidden() const override
            {

            }

            inline virtual bool
            is_minimized() const override
            {

            }

            inline virtual bool
            is_maximized() const override
            {

            }

            inline virtual bool
            is_normalized() const override
            {

            }

            inline virtual bool
            is_active() const override
            {

            }

            inline virtual bool
            is_inactive() const override
            {

            }

    };

}