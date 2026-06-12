#pragma once
#if defined(__APPLE__)

#include <utils/defs.hpp>
#include <utils/logging.hpp>
#include <simplex/platform/window.hpp>
#include <simplex/static_queue.hpp>

// The header is consumed by plain C++ translation units while the
// implementation is Objective-C++; the Cocoa class types degrade to opaque
// pointers outside of Objective-C so the class layout stays identical.
#if defined(__OBJC__)
@class NSWindow;
@class NSView;
@class NSEvent;
@class CAMetalLayer;
@class SpxDarwinWindowDelegate;
#else
typedef void NSWindow;
typedef void NSView;
typedef void NSEvent;
typedef void CAMetalLayer;
typedef void SpxDarwinWindowDelegate;
#endif

namespace spx
{

    struct darwin_window_bridge;

    /// @brief  Provides a way of creating a darwin (Cocoa) window.
    ///
    ///         The window's content view is layer-hosted with a CAMetalLayer,
    ///         which is the presentation target for both Metal renderers and
    ///         Vulkan via MoltenVK (VK_EXT_metal_surface); get_metal_layer() is
    ///         the darwin analog of win32's HWND-for-DXGI accessor.
    ///
    ///         Application events are translated into window_events as
    ///         poll_events() pumps NSApplication's event queue. Default handling
    ///         (close requests, state and geometry tracking) is applied at
    ///         translation time, so draining the queue with poll_event() is
    ///         optional; unconsumed events are purely informational and the
    ///         oldest are dropped if the queue overflows.
    ///
    ///         Controllers are not pump-driven on darwin and are instead sampled
    ///         at the end of each poll_events() pump; their events will surface
    ///         through poll_event() like every other input event.
    ///
    ///         Cocoa requires that windows are created and pumped from the main
    ///         thread; create() and poll_events() must be called from it.
    class darwin_window : public window_interface
    {

        public:
            inline          darwin_window() = default;
            inline virtual ~darwin_window() = default;

            inline operator NSWindow*() const { return this->window; }

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

            /// @brief  Returns the layer Metal renderers attach to (and MoltenVK
            ///         presents through); the darwin equivalent of the HWND
            ///         required for DXGI swap chain creation on win32. The layer's
            ///         drawableSize is kept in sync with the window's backing
            ///         (pixel) size as it resizes or moves between displays.
            /// @return The metal layer, or NULL if the window is invalid.
            inline CAMetalLayer* get_metal_layer() const { return this->layer; }

            /// @brief  Returns the native window object; equivalent to
            ///         get_native_handle() without the void* indirection.
            /// @return The window, or NULL if the window is invalid.
            inline NSWindow* get_window_handle() const { return this->window; }

            /// @brief  Returns the window's content view hosting the metal layer,
            ///         as required by APIs which attach to views rather than
            ///         layers (MTKView-style integration, drag and drop, etc.).
            /// @return The content view, or NULL if the window is invalid.
            inline NSView* get_content_view() const { return this->view; }

        private:
            friend struct darwin_window_bridge;

            static bool initialize_application();

            bool translate_event(NSEvent *event);
            void handle_close_request();
            void handle_resize();
            void handle_focus(bool focused);
            void handle_miniaturize(bool miniaturized);
            void handle_backing_changed();
            uint64_t get_windowed_style_mask() const;
            void apply_windowed_style();
            void update_drawable_size();
            void poll_controllers();
            void push_event(const spx::window_event &event);
            void push_key_event(EngineKeyCode key_code, bool pressed);
            void push_mouse_event(EngineMouseCode mouse_code, bool pressed);

        private:
            /// Tracks which size-state transition was last reported so resize
            /// notifications only queue state-change events on actual
            /// transitions, not every resize tick.
            enum class size_state : uint32_t
            {
                normal,
                minimized,
                maximized,
                borderless,
            };

            static inline constexpr size_t max_events = 4096; // Surely this is enough?
            spx::static_queue<window_event, max_events> window_event_queue;

            NSWindow *window                    = NULL;
            NSView *view                        = NULL;
            CAMetalLayer *layer                 = NULL;
            SpxDarwinWindowDelegate *delegate   = NULL;

            // Cocoa frames are in screen points; the windowed geometry is saved
            // here so normalize() can restore it after borderless().
            real64_t windowed_x                 = 0.0;
            real64_t windowed_y                 = 0.0;
            real64_t windowed_width             = 0.0;
            real64_t windowed_height            = 0.0;

            size_state current_size_state       = size_state::normal;
            bool close_requested                = false;
            bool borderless_mode                = false;
            bool resizing_locked                = false;

    };

}

#endif
