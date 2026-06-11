#pragma once
#if defined(_WIN32)
#   include <windows.h>

#include <utils/defs.hpp>
#include <utils/logging.hpp>
#include <simplex/platform/window.hpp>
#include <simplex/static_queue.hpp>

#if !defined(WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#   define NOMINMAX
#endif

namespace spx
{

    /// @brief  Provides a way of creating a win32 window.
    ///
    ///         The window procedure translates win32 messages into window_events
    ///         as poll_events() pumps the thread's message queue. Default handling
    ///         (close requests, state and geometry tracking) is applied at
    ///         translation time, so draining the queue with poll_event() is
    ///         optional; unconsumed events are purely informational and the oldest
    ///         are dropped if the queue overflows.
    ///
    ///         Controllers are not message-driven on win32 and are instead sampled
    ///         at the end of each poll_events() pump; their events will surface
    ///         through poll_event() like every other input event.
    class win32_window : public window_interface
    {

        public:
            inline          win32_window() = default;
            inline virtual ~win32_window() = default;

            inline operator HWND() const { return this->window; }

            virtual bool create(spx::string_view<char> window_title, uint32_t width, uint32_t height) override;
            virtual void destroy() override;

            virtual void poll_events() override;
            virtual bool poll_event(spx::window_event *event) override;

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
            virtual WindowStatus get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const override;
            virtual WindowStatus create_vulkan_surface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const override;
            virtual WindowStatus get_native_handle(void **native_handle) const override;

            /// @brief  Returns the native window handle required for DXGI (DX11/DX12)
            ///         swap chain creation; equivalent to get_native_handle() without
            ///         the void* indirection.
            /// @return The window handle, or NULL if the window is invalid.
            inline HWND get_window_handle() const { return this->window; }

            /// @brief  Returns the module instance the window was created against,
            ///         as required by VkWin32SurfaceCreateInfoKHR.
            /// @return The instance handle, or NULL if the window is invalid.
            inline HINSTANCE get_instance_handle() const { return this->instance; }

        private:
            static bool register_window_class(HINSTANCE instance);
            static LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

            LRESULT handle_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
            DWORD get_windowed_style() const;
            void apply_windowed_style();
            void poll_controllers();
            void push_event(const spx::window_event &event);
            void push_key_event(EngineKeyCode key_code, bool pressed);
            void push_mouse_event(EngineMouseCode mouse_code, bool pressed);

        private:
            /// Tracks which size-state transition was last reported so WM_SIZE only
            /// queues state-change events on actual transitions, not every resize tick.
            enum class size_state : uint32_t
            {
                normal,
                minimized,
                maximized,
                borderless,
            };

            static inline constexpr size_t max_events = 4096; // Surely this is enough?
            spx::static_queue<window_event, max_events> window_event_queue;

            HWND window                         = NULL;
            HINSTANCE instance                  = NULL;
            WINDOWPLACEMENT windowed_placement  = {};
            size_state current_size_state       = size_state::normal;
            bool close_requested                = false;
            bool borderless_mode                = false;
            bool resizing_locked                = false;

    };

}

#endif