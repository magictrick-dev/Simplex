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

RendererResultType spx::vk::vulkan_renderer::
internal_deinitialize() 
{

    if constexpr (enable_validation) spx::vk::destroy_debug_utils_messenger(this->instance, this->debug_messenger);
    spx::vk::destroy_instance(this->instance);

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
    if constexpr (enable_validation)
        required_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#   if defined(__APPLE__)
        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#   endif

    // Add additional layers as we need them.
    spx::dynamic_array<const char*> required_layers;
    if constexpr (enable_validation)
        required_layers.emplace_back("VK_LAYER_KHRONOS_validation");

    // Verify our extensions.
    if (!spx::vk::instance_t::validate_instance_extensions(required_extensions))
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to validate instance extensions.");
    spx::logger::dispatch_diagnostic_log("Validated instance extensions successfully.");

    // Verify our layers.
    if (!spx::vk::instance_t::validate_instance_layers(required_layers))
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to validate instance layers.");
    spx::logger::dispatch_diagnostic_log("Validated instance layers successfully.");

    // Set application info.
    spx::vk::application_info_t application_info { };
    application_info.apiVersion         = VK_MAKE_VERSION(1, 3, 0);
    application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    application_info.pApplicationName   = "Simplex";
    application_info.pEngineName        = "SimplexVK";

    // Instance creation.
    spx::vk::instance_create_info_t instance_create_info { };
    instance_create_info.set_extensions(required_extensions);
    instance_create_info.set_layers(required_layers);

    // When validation is enabled, describe the debug messenger up front and chain it into the
    // instance create info's pNext so the vkCreateInstance/vkDestroyInstance calls themselves are
    // covered, then reuse the same description to build the persistent messenger once the instance
    // exists. The whole path is dispatched at compile time off enable_validation.
    spx::vk::debug_utils_messenger_create_info_t debug_create_info { };
    if constexpr (enable_validation)
    {
        debug_create_info.set_message_severity(
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
        debug_create_info.set_message_type(
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
        debug_create_info.set_user_callback(&vulkan_renderer::debug_callback);
        debug_create_info.set_user_data(this);

        const VkDebugUtilsMessengerCreateInfoEXT& native_debug_info = debug_create_info;
        instance_create_info.pNext = &native_debug_info;
    }

    // Creates the instance.
    const auto result = spx::vk::create_instance(instance_create_info, NULL, this->instance);
    if (result != VK_SUCCESS) THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create vulkan instance.");

    spx::logger::dispatch_diagnostic_log("Created a vulkan instance successfully.");

    // Stand up the persistent debug messenger for the lifetime of the instance.
    if constexpr (enable_validation)
    {
        const auto debug_result = spx::vk::create_debug_utils_messenger(
            this->instance, debug_create_info, NULL, this->debug_messenger);
        if (debug_result != VK_SUCCESS)
            THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create the vulkan debug messenger.");

        spx::logger::dispatch_diagnostic_log("Created the vulkan debug messenger successfully.");
    }

}

VKAPI_ATTR VkBool32 VKAPI_CALL spx::vk::vulkan_renderer::
debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*                                       user_data)
{

    switch (message_severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: spx::logger::dispatch_diagnostic_log(callback_data->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: spx::logger::dispatch_information_log(callback_data->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: spx::logger::dispatch_warning_log(callback_data->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: spx::logger::dispatch_error_log(callback_data->pMessage); break;
        default :spx::logger::dispatch_error_log(callback_data->pMessage);
    }

    return VK_FALSE;

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