#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>
#include <simplex/platform/window.hpp>

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

            inline virtual bool
            create(spx::string_view<char> window_title, uint32_t width, uint32_t height) override
            {

                if (this->window != NULL)
                {
                    return false;
                }

                // Idempotent; safe even if the application already initialized GLFW.
                if (glfwInit() != GLFW_TRUE)
                {
                    return false;
                }

                // The title view isn't guaranteed to be null-terminated; titles longer
                // than the buffer are truncated.
                char title[256];
                size_t title_length = window_title.size();
                if (title_length > sizeof(title) - 1)
                {
                    title_length = sizeof(title) - 1;
                }
                memcpy(title, window_title.data(), title_length);
                title[title_length] = '\0';

                // No client API; presentation is provided by Vulkan/DX, not OpenGL.
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

                // Windows are created hidden per the interface contract; show() reveals them.
                glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
                glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

                this->window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, NULL, NULL);
                if (this->window == NULL)
                {
                    spx::logger::dispatch_critical_log("GLFW returned NULL on window creation.");
                }
                return this->window != NULL;

            }

            inline virtual void
            destroy() override
            {

                // glfwTerminate() is intentionally left to application shutdown since
                // other windows may still be live.
                if (this->window != NULL)
                {
                    glfwDestroyWindow(this->window);
                    this->window = NULL;
                }

            }

            inline virtual void
            poll_events() override
            {

                glfwPollEvents();

            }

            inline virtual void
            hide() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                glfwHideWindow(this->window);

            }

            inline virtual void
            show() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                glfwShowWindow(this->window);

            }

            inline virtual void
            maximize() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                glfwMaximizeWindow(this->window);

            }

            inline virtual void
            borderless() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                if (glfwGetWindowMonitor(this->window) != NULL)
                {
                    return;
                }

                // Capture the windowed geometry so normalize() can restore it.
                glfwGetWindowPos(this->window, &this->windowed_x, &this->windowed_y);
                glfwGetWindowSize(this->window, &this->windowed_width, &this->windowed_height);

                GLFWmonitor *monitor = glfwGetPrimaryMonitor();
                if (monitor == NULL)
                {
                    return;
                }

                const GLFWvidmode *mode = glfwGetVideoMode(monitor);
                if (mode == NULL)
                {
                    return;
                }

                // Matching the monitor's current video mode yields borderless fullscreen
                // rather than an exclusive mode switch.
                glfwSetWindowMonitor(this->window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

            }

            inline virtual void
            normalize() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                if (glfwGetWindowMonitor(this->window) != NULL)
                {
                    glfwSetWindowMonitor(this->window, NULL, this->windowed_x, this->windowed_y,
                            this->windowed_width, this->windowed_height, 0);
                }

                glfwRestoreWindow(this->window);

            }

            inline virtual void
            lock_resizing() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                glfwSetWindowAttrib(this->window, GLFW_RESIZABLE, GLFW_FALSE);

            }
            inline virtual void
            unlock_resizing() override
            {

                if (this->window == NULL)
                {
                    return;
                }

                glfwSetWindowAttrib(this->window, GLFW_RESIZABLE, GLFW_TRUE);

            }

            inline virtual bool
            is_valid() const override
            {

                return this->window != NULL;

            }

            inline virtual bool
            should_close() const override
            {

                // An invalid window has nothing to keep open.
                if (this->window == NULL)
                {
                    return true;
                }

                return glfwWindowShouldClose(this->window) == GLFW_TRUE;

            }

            inline virtual int32_t
            get_width() const override
            {

                if (this->window == NULL)
                {
                    return 0;
                }

                int32_t width = 0;
                int32_t height = 0;
                glfwGetWindowSize(this->window, &width, &height);
                return width;

            }

            inline virtual int32_t
            get_height() const override
            {

                if (this->window == NULL)
                {
                    return 0;
                }

                int32_t width = 0;
                int32_t height = 0;
                glfwGetWindowSize(this->window, &width, &height);
                return height;

            }

            inline virtual real32_t
            get_aspect_ratio() const override
            {

                if (this->window == NULL)
                {
                    return 0.0f;
                }

                int32_t width = 0;
                int32_t height = 0;
                glfwGetWindowSize(this->window, &width, &height);
                if (height == 0)
                {
                    return 0.0f;
                }

                return static_cast<real32_t>(width) / static_cast<real32_t>(height);

            }

            inline virtual bool
            is_visible() const override
            {

                if (this->window == NULL)
                {
                    return false;
                }

                return glfwGetWindowAttrib(this->window, GLFW_VISIBLE) == GLFW_TRUE;

            }

            inline virtual bool
            is_hidden() const override
            {

                return this->is_visible() == false;

            }

            inline virtual bool
            is_minimized() const override
            {

                if (this->window == NULL)
                {
                    return false;
                }

                return glfwGetWindowAttrib(this->window, GLFW_ICONIFIED) == GLFW_TRUE;

            }

            inline virtual bool
            is_maximized() const override
            {

                if (this->window == NULL)
                {
                    return false;
                }

                return glfwGetWindowAttrib(this->window, GLFW_MAXIMIZED) == GLFW_TRUE;

            }

            inline virtual bool
            is_normalized() const override
            {

                if (this->window == NULL)
                {
                    return false;
                }

                if (this->is_minimized() || this->is_maximized())
                {
                    return false;
                }

                // A window bound to a monitor is fullscreen/borderless, not normal.
                return glfwGetWindowMonitor(this->window) == NULL;

            }

            inline virtual bool
            is_active() const override
            {

                if (this->window == NULL)
                {
                    return false;
                }

                return glfwGetWindowAttrib(this->window, GLFW_FOCUSED) == GLFW_TRUE;

            }

            inline virtual bool
            is_inactive() const override
            {

                return this->is_active() == false;

            }

            inline virtual WindowStatus
            get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const override
            {

                if (extension_list == NULL || extension_count == NULL)
                {
                    return WindowStatus_InvalidArgument;
                }

                *extension_list = NULL;
                *extension_count = 0;

                // Requires GLFW to be initialized first; reports unsupported otherwise.
                if (glfwVulkanSupported() != GLFW_TRUE)
                {
                    return WindowStatus_VulkanUnsupported;
                }

                *extension_list = glfwGetRequiredInstanceExtensions(extension_count);
                if (*extension_list == NULL)
                {
                    return WindowStatus_VulkanUnsupported;
                }

                return WindowStatus_OK;

            }

            inline virtual WindowStatus
            create_vulkan_surface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const override
            {

                if (surface == NULL)
                {
                    return WindowStatus_InvalidArgument;
                }

                *surface = VK_NULL_HANDLE;

                if (instance == VK_NULL_HANDLE)
                {
                    return WindowStatus_InvalidArgument;
                }

                if (this->window == NULL)
                {
                    return WindowStatus_WindowInvalid;
                }

                VkResult result = glfwCreateWindowSurface(instance, this->window, allocator, surface);
                if (result != VK_SUCCESS)
                {
                    return WindowStatus_VulkanSurfaceFailed;
                }

                return WindowStatus_OK;

            }

            inline virtual WindowStatus
            get_native_handle(void **native_handle) const override
            {

                if (native_handle == NULL)
                {
                    return WindowStatus_InvalidArgument;
                }

                *native_handle = NULL;

                if (this->window == NULL)
                {
                    return WindowStatus_WindowInvalid;
                }

#               if defined(_WIN32)
                    *native_handle = static_cast<void*>(glfwGetWin32Window(this->window));
                    if (*native_handle == NULL)
                    {
                        return WindowStatus_NativeHandleUnavailable;
                    }

                    return WindowStatus_OK;
#               else
                    return WindowStatus_NativeHandleUnavailable;
#               endif

            }

        private:
            GLFWwindow *window      = NULL;
            int32_t windowed_x      = 0;
            int32_t windowed_y      = 0;
            int32_t windowed_width  = 0;
            int32_t windowed_height = 0;

    };

}