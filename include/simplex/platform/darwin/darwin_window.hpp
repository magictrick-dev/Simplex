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

            virtual bool create(spx::string_view<char> window_title, uint32_t width, uint32_t height) override;
            virtual void destroy() override;
            virtual void poll_events() override;
            virtual void hide() override;
            virtual void show() override;
            virtual void maximize() override;
            virtual void borderless() override;
            virtual void normalize() override;
            virtual void lock_resizing() override;
            virtual void unlock_resizing() override;
            virtual bool is_valid() const override;
            virtual bool should_close() const override;
            virtual int32_t get_width() const override;
            virtual int32_t get_height() const override;
            virtual real32_t get_aspect_ratio() const override;
            virtual bool is_visible() const override;
            virtual bool is_hidden() const override;
            virtual bool is_minimized() const override;
            virtual bool is_maximized() const override;
            virtual bool is_normalized() const override;
            virtual bool is_active() const override;
            virtual bool is_inactive() const override;

    };

}