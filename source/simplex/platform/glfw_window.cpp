#include <simplex/platform/glfw_window.hpp>

bool spx::glfw_window::
create(spx::string_view<char> window_title, uint32_t width, uint32_t height) 
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

void spx::glfw_window::
destroy()
{

    // glfwTerminate() is intentionally left to application shutdown since
    // other windows may still be live.
    if (this->window != NULL)
    {
        glfwDestroyWindow(this->window);
        this->window = NULL;
    }

}

void spx::glfw_window::
poll_events() 
{

    glfwPollEvents();

}

void spx::glfw_window::
hide() 
{

    if (this->window == NULL)
    {
        return;
    }

    glfwHideWindow(this->window);

}

void spx::glfw_window::
show() 
{

    if (this->window == NULL)
    {
        return;
    }

    glfwShowWindow(this->window);

}

void spx::glfw_window::
maximize() 
{

    if (this->window == NULL)
    {
        return;
    }

    glfwMaximizeWindow(this->window);

}

void spx::glfw_window::
borderless() 
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

void spx::glfw_window::
normalize() 
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

void spx::glfw_window::
lock_resizing() 
{

    if (this->window == NULL)
    {
        return;
    }

    glfwSetWindowAttrib(this->window, GLFW_RESIZABLE, GLFW_FALSE);

}
void spx::glfw_window::
unlock_resizing() 
{

    if (this->window == NULL)
    {
        return;
    }

    glfwSetWindowAttrib(this->window, GLFW_RESIZABLE, GLFW_TRUE);

}

bool spx::glfw_window::
is_valid() const 
{

    return this->window != NULL;

}

bool spx::glfw_window::
should_close() const 
{

    // An invalid window has nothing to keep open.
    if (this->window == NULL)
    {
        return true;
    }

    return glfwWindowShouldClose(this->window) == GLFW_TRUE;

}

int32_t spx::glfw_window::
get_width() const 
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

int32_t spx::glfw_window::
get_height() const 
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

real32_t spx::glfw_window::
get_aspect_ratio() const 
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

bool spx::glfw_window::
is_visible() const 
{

    if (this->window == NULL)
    {
        return false;
    }

    return glfwGetWindowAttrib(this->window, GLFW_VISIBLE) == GLFW_TRUE;

}

bool spx::glfw_window::
is_hidden() const 
{

    return this->is_visible() == false;

}

bool spx::glfw_window::
is_minimized() const 
{

    if (this->window == NULL)
    {
        return false;
    }

    return glfwGetWindowAttrib(this->window, GLFW_ICONIFIED) == GLFW_TRUE;

}

bool spx::glfw_window::
is_maximized() const 
{

    if (this->window == NULL)
    {
        return false;
    }

    return glfwGetWindowAttrib(this->window, GLFW_MAXIMIZED) == GLFW_TRUE;

}

bool spx::glfw_window::
is_normalized() const 
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

bool spx::glfw_window::
is_active() const 
{

    if (this->window == NULL)
    {
        return false;
    }

    return glfwGetWindowAttrib(this->window, GLFW_FOCUSED) == GLFW_TRUE;

}

bool spx::glfw_window::
is_inactive() const 
{

    return this->is_active() == false;

}

WindowStatus spx::glfw_window::
get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const 
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

WindowStatus spx::glfw_window::
create_vulkan_surface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const 
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

WindowStatus spx::glfw_window::
get_native_handle(void **native_handle) const 
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
