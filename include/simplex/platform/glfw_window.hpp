#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>
#include <simplex/platform/window.hpp>
#include <GLFW/glfw3.h>

namespace spx
{

    /// @brief The GLFW window interface provides a default-platform agnostic window entry
    ///        point for platforms that have not had their platform-specific window definitions
    ///        created. This is mainly for Linux platforms since their windowing systems are
    ///        not strictly defined to one API (wayland, X11, etc.).
    class glfw_window : public window_interface
    {

        public:
            inline          glfw_window() = default;
            inline virtual ~glfw_window() = default;

            inline operator GLFWwindow*() { return this->window; }

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
            virtual WindowStatus get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const override;
            virtual WindowStatus create_vulkan_surface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const override;
            virtual WindowStatus get_native_handle(void **native_handle) const override;

        private:
            GLFWwindow *window      = NULL;
            int32_t windowed_x      = 0;
            int32_t windowed_y      = 0;
            int32_t windowed_width  = 0;
            int32_t windowed_height = 0;

    };
            
}