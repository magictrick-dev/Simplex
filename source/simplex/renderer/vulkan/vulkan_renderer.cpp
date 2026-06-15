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

    // TODO(Chris): Actually create the instance.
    spx::vk::application_info application_info { };
    application_info.apiVersion = VK_MAKE_VERSION(1, 3, 0);
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pApplicationName = "Simplex";
    application_info.pEngineName = "SimplexVK";

    spx::vk::instance_create_info instance_create_info { };
    instance_create_info.set_extensions(required_extensions);
    instance_create_info.set_layers(required_layers);

    // Creates the instance.
    const auto result = spx::vk::create_instance(instance_create_info, NULL, this->instance);
    if (result != VK_SUCCESS) THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create vulkan instance.");

    spx::logger::dispatch_diagnostic_log("Created a vulkan instance successfully.");

}

RendererResultType spx::vk::vulkan_renderer::
internal_deinitialize() 
{

    spx::vk::destroy_instance(this->instance);

    return RendererResultType_OK;

}

void spx::vk::vulkan_renderer::
select_physical_device()
{

}

void spx::vk::vulkan_renderer::
create_logical_device()
{

    // Create logical device with required extensions.
    spx::dynamic_array<const char*> required_device_extensions;

#   if defined(__APPLE__)
        required_device_extensions.emplace_back("VK_KHR_portability_subset");
#   endif

    // TODO(Chris): Actually create the logical device.

}

void spx::vk::vulkan_renderer::
create_surface()
{

    // Fetch the window.
    spx::window_interface *window = this->get_window();

    // TODO(Chris): Actually create the surface.

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