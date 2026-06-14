#include <simplex/renderer/vulkan/vulkan_renderer.hpp>
#include <simplex/platform/window.hpp>
#include <simplex/dynamic_array.hpp>

RendererResultType spx::vk::vulkan_renderer::
internal_initialize()
{

    this->create_instance();
    this->create_surface();
    this->select_physical_device();
    this->create_logical_device();

    // Generating a surface.

    return RendererResultType_OK;

}

void spx::vk::vulkan_renderer::
create_instance()
{

    // Fetching the window.
    spx::window_interface *window = this->get_window();

    // Getting required extensions based on the window.
    uint32_t instance_extensions_count = 0;
    const char **instance_extensions = NULL;
    window->get_vulkan_instance_extensions(&instance_extensions, &instance_extensions_count);

    // Setting required extensions based on the window.
    spx::dynamic_array<const char*> required_extensions;
    for (uint32_t index = 0; index < instance_extensions_count; ++index)
    {
        required_extensions.emplace_back(instance_extensions[index]);
    }

    // Add additional instance extensions as needed.
    required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#   if defined(__APPLE__)
        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#   endif

    // Add additional layers as we need them.
    spx::dynamic_array<const char*> required_layers;
    required_layers.emplace_back("VK_LAYER_KHRONOS_validation");

    this->instance.create(required_extensions, required_layers);

}

void spx::vk::vulkan_renderer::
select_physical_device()
{

    // Allowing the selection of the optimal device based on device scoring.
    this->physical_device = spx::vk::physical_device::get_optimal_device(this->instance, this->surface);

    spx::logger::dispatch_diagnostic_log(
        "Selected {} as the physical device.",
        this->physical_device.get_qualified_name().c_str()
    );

}

void spx::vk::vulkan_renderer::
create_logical_device()
{

    // Create logical device with required extensions.
    spx::dynamic_array<const char*> required_device_extensions;

#   if defined(__APPLE__)
        required_device_extensions.emplace_back("VK_KHR_portability_subset");
#   endif

    this->logical_device.create(this->instance, this->physical_device, required_device_extensions);

}

void spx::vk::vulkan_renderer::
create_surface()
{

    // Fetch the window.
    spx::window_interface *window = this->get_window();

    // Creating the surface.
    this->surface.create(this->instance, window);

}

RendererResultType spx::vk::vulkan_renderer::
internal_frame_begin() 
{

    return RendererResultType_OK;
}

RendererResultType spx::vk::vulkan_renderer::
internal_render_begin() 
{

    return RendererResultType_OK;
}

RendererResultType spx::vk::vulkan_renderer::
internal_render_end() 
{

    return RendererResultType_OK;
}

RendererResultType spx::vk::vulkan_renderer::
internal_frame_end() 
{

    return RendererResultType_OK;
}

RendererResultType spx::vk::vulkan_renderer::
internal_deinitialize() 
{

    this->logical_device.destroy();
    this->surface.destroy();
    this->instance.destroy();

    return RendererResultType_OK;

}