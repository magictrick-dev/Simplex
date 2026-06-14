#include <simplex/renderer/vulkan/surface.hpp>

void spx::vk::surface::
create(spx::vk::instance instance, spx::window_interface *window)
{

    this->instance = instance;

    // Window abstraction handles surface creation for us depending on platform.
    WindowStatus result = window->create_vulkan_surface(instance, NULL, &this->handle);
    if (result != WindowStatus_OK)
    {
        switch (result)
        {
            case WindowStatus_NativeHandleUnavailable:
                THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create surface, native handle invalid.");
            case WindowStatus_VulkanSurfaceFailed:
                THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create surface, creation failed.");
            case WindowStatus_VulkanUnsupported:
                THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create surface, vulkan unsupported.");
            default:
                THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create surface, reason not captured.");
        }
    }

}

void spx::vk::surface::
destroy()
{
    vkDestroySurfaceKHR(this->instance, this->handle, NULL);
}