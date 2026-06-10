#pragma once
#include <GLFW/glfw3.h>
#include <simplex/string_view.hpp>

namespace spx
{

    class window
    {
        public:
            inline  window() { }
            inline ~window() { }
            GLFWwindow *handle = NULL;

            inline operator GLFWwindow*() { return this->handle; }
    };

    /// @brief  The window interface serves as the primary way to pass around a window
    ///         without knowing the underlying implementation. In the event that platform
    ///         internals are required, casting to platform window type is possible through
    ///         platform detection macros.
    ///
    ///         When a window is created, it is by default not shown to the user.
    ///         The user must manually invoke .show() to display the window.
    ///
    ///         The window interface is trivially copyable; meaning the creation and destruction
    ///         must be invoked manually. In most cases, the intent is clear about who creates
    ///         and destroys a window.
    class window_interface
    {
        public:
            inline          window_interface() = default;
            inline virtual ~window_interface() = default;

            /// @brief Creates a window.
            /// @param window_title The title to give to the window.
            /// @param width The initial starting width of the window.
            /// @param height The initial starting height of the window.
            /// @return True if creation succeeded, false otherwise.
            inline virtual bool create(spx::string_view<char> window_title, uint32_t width, uint32_t height) = 0;

            /// @brief Destroys the window and invalidates the internal handle.
            inline virtual void destroy() = 0;

            /// @brief Polls all the platform events for the window.
            inline virtual void poll_events() = 0;

            /// @brief Hides the window from the user.
            inline virtual void hide() = 0;

            /// @brief Shows the window (if it is hidden).
            inline virtual void show() = 0;

            /// @brief Maximizes the window, behavior is platform dependent.
            inline virtual void maximize() = 0;

            /// @brief Enables fullscreen-borderless mode.
            inline virtual void borderless() = 0;

            /// @brief Normalizes the window (if it is minimized or maximized).
            inline virtual void normalize() = 0;

            /// @brief Prevents resizing and maximization.
            inline virtual void lock_resizing() = 0;

            /// @brief Allows resizing and maximization.
            inline virtual void unlock_resizing() = 0;

            /// @brief Determines if the window is valid.
            /// @return True if it is.
            inline virtual bool is_valid() const = 0;

            /// @brief Determines if the window should be closed and destroyed.
            /// @return True if it should.
            inline virtual bool should_close() const = 0;

            /// @brief Returns the window's width.
            /// @return The window's width.
            inline virtual int32_t get_width() const = 0;

            /// @brief Returns the window's height.
            /// @return The window's height.
            inline virtual int32_t get_height() const = 0;
            
            /// @brief Returns the window's aspect ration.
            /// @return The window's aspect ratio.
            inline virtual real32_t get_aspect_ratio() const = 0;

            /// @brief Returns whether or not the window is visible (platform dependent).
            /// @return True if it is visible.
            inline virtual bool is_visible() const = 0;

            /// @brief Returns whether or not the window is hidden (platform dependent).
            /// @return True if it is hidden.
            inline virtual bool is_hidden() const = 0;

            /// @brief Returns whether or not the window is minimized.
            /// @return True if it is.
            inline virtual bool is_minimized() const = 0;

            /// @brief Returns whether or not the window is maximized.
            /// @return True if it is.
            inline virtual bool is_maximized() const = 0;

            /// @brief Returns whether or not the window is a normal state (not maximized/fullscreen/minimized).
            /// @return True if it is.
            inline virtual bool is_normalized() const = 0;

            /// @brief Returns if the window is actively focused (platform dependent).
            /// @return True if it is.
            inline virtual bool is_active() const = 0;

            /// @brief Returns if the window is not the active focus.
            /// @return True if it is.
            inline virtual bool is_inactive() const = 0;

    };

    /// @brief      The GLFW window interface provides a default-platform agnostic window entry
    ///             point for platforms that have not had their platform-specific window definitions
    ///             created. This is mainly for Linux platforms since their windowing systems are
    ///             not strictly defined to one API (wayland, X11, etc.).
    class glfw_window : public window_interface
    {

        public:
            inline          glfw_window() = default;
            inline virtual ~glfw_window() = default;

            inline operator GLFWwindow*() { return this->window; }

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

        private:
            GLFWwindow *window = NULL;

    };

    /// @brief  Provides a way of creating a win32 window.
    class win32_window : public window_interface
    {

        public:
            inline          win32_window() = default;
            inline virtual ~win32_window() = default;

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