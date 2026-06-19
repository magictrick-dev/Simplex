#include <utils/about.hpp>

#include <simplex/renderer/vulkan/vulkan_renderer.hpp>
#include <simplex/platform/window.hpp>
#include <simplex/dynamic_array.hpp>

RendererResultType spx::vk::vulkan_renderer::
internal_initialize()
{

    if (this->create_instance() == false)           return RendererResultType_VulkanInstanceCreationFailed;
    if (this->create_surface() == false)            return RendererResultType_VulkanSurfaceCreationFailed;
    if (this->select_physical_device() == false)    return RendererResultType_VulkanPhysicalDeviceSelectionFailed;
    if (this->create_logical_device() == false)     return RendererResultType_VulkanLogicalDeviceCreationFailed;

    return RendererResultType_OK;

}

RendererResultType spx::vk::vulkan_renderer::
internal_deinitialize() 
{

    spx::vk::destroy_device(this->device);
    if constexpr (enable_validation) spx::vk::destroy_debug_utils_messenger(this->instance, this->debug_messenger);
    spx::vk::destroy_surface(this->instance, this->surface);
    spx::vk::destroy_instance(this->instance);

    return RendererResultType_OK;

}

bool32_t spx::vk::vulkan_renderer::
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
    {
        required_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    // Verify our extensions.
    if (!spx::vk::instance_t::validate_instance_extensions(required_extensions))
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to validate instance extensions.");
        return false;
    }

    spx::logger::dispatch_diagnostic_log("Validated instance extensions successfully.");

    // Verify our layers.
    if (!spx::vk::instance_t::validate_instance_layers(required_layers))
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to validate instance layers.");
        return false;
    }
    spx::logger::dispatch_diagnostic_log("Validated instance layers successfully.");

    // Set application info.
    spx::vk::application_info_t application_info { };
    application_info.apiVersion         = VK_API_VERSION_1_4;
    application_info.engineVersion      = VK_MAKE_VERSION(SIMPLEX_ABOUT_VERSION_MAJOR,
                                                          SIMPLEX_ABOUT_VERSION_MINOR,
                                                          SIMPLEX_ABOUT_VERSION_PATCH);
    application_info.pApplicationName   = SIMPLEX_ABOUT_PROJECT_NAME;
    application_info.pEngineName        = SIMPLEX_ABOUT_ENGINE_NAME;

    // Instance creation.
    spx::vk::instance_create_info_t instance_create_info { };
    instance_create_info.set_extensions(required_extensions);
    instance_create_info.set_layers(required_layers);
    instance_create_info.pApplicationInfo = &application_info;

#   if defined(__APPLE__)
        // MoltenVK is a portability-subset driver. The loader only enumerates such drivers when the
        // instance is created with this flag set (alongside the portability-enumeration extension
        // added above). Without it, only native ICDs (e.g. kosmickrisp) appear and MoltenVK is
        // hidden entirely.
        instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#   endif

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
    if (result != VK_SUCCESS)
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create vulkan instance.");
        return false;
    }

    spx::logger::dispatch_diagnostic_log("Created a vulkan instance successfully.");

    // Stand up the persistent debug messenger for the lifetime of the instance.
    if constexpr (enable_validation)
    {
        const auto debug_result = spx::vk::create_debug_utils_messenger(
            this->instance, debug_create_info, NULL, this->debug_messenger);
        if (debug_result != VK_SUCCESS)
        {
            THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create the vulkan debug messenger.");
            return false;
        }

        spx::logger::dispatch_diagnostic_log("Created the vulkan debug messenger successfully.");
    }

    return true;

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

bool32_t spx::vk::vulkan_renderer::
select_physical_device()
{

    auto physical_devices = spx::vk::physical_device_t::get_physical_devices(this->instance);
    for (auto device : physical_devices)
    {
        // Include the driver name and Vulkan version: on Apple multiple ICDs (MoltenVK, kosmickrisp)
        // expose the same GPU under the same device name, so those are what tell them apart.
        const uint32_t version = device.get_device_api_version();
        spx::logger::dispatch_diagnostic_log("Physical device found: {} [{}] (Vulkan {}.{}.{}).",
            device.get_device_name().data(),
            device.get_device_driver_name().data(),
            VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));
    }

    // Pick the highest-scoring device (Vulkan version, then class, then VRAM). select_optimal_device
    // pre-fetches each device's properties/features/queue families at construction. Because version
    // ranks first, the 1.4-capable MoltenVK device wins over the kosmickrisp ICD (which lags at an
    // earlier version) even though both report the same M5 GPU, type, and unified memory.
    this->physical_device = spx::vk::physical_device_t::select_optimal_device(this->instance);
    if (this->physical_device.native == nullptr)
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("No suitable vulkan physical device was found.");
        return false;
    }

    const uint32_t selected_version = this->physical_device.get_device_api_version();
    spx::logger::dispatch_diagnostic_log("Selected physical device: {} ({}) [{}] (Vulkan {}.{}.{}).",
        this->physical_device.get_device_name().data(),
        this->physical_device.get_device_vendor().data(),
        this->physical_device.get_device_driver_name().data(),
        VK_API_VERSION_MAJOR(selected_version), VK_API_VERSION_MINOR(selected_version), VK_API_VERSION_PATCH(selected_version));

    // Resolve the queue families we need: one that can do graphics and one that can present to our
    // surface. Prefer a single family that does both (the common case); otherwise fall back to the
    // first family of each capability.
    const auto& families = this->physical_device.get_queue_families();
    for (uint32_t index = 0; index < families.size(); ++index)
    {
        const auto& properties = families[index].get_queue_family_properties();

        const bool32_t graphics = properties.supports_graphics();
        const bool32_t present  = properties.supports_presentation(
            this->physical_device.native, index, this->surface.native);

        if (graphics && this->graphics_queue_family_index == invalid_queue_family)
            this->graphics_queue_family_index = index;
        if (present && this->present_queue_family_index == invalid_queue_family)
            this->present_queue_family_index = index;

        if (graphics && present)
        {
            this->graphics_queue_family_index = index;
            this->present_queue_family_index  = index;
            break;
        }
    }

    if (this->graphics_queue_family_index == invalid_queue_family)
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Selected device has no graphics-capable queue family.");
        return false;
    }

    if (this->present_queue_family_index == invalid_queue_family)
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Selected device has no presentation-capable queue family.");
        return false;
    }

    spx::logger::dispatch_diagnostic_log("Queue families resolved (graphics: {}, present: {}).",
        this->graphics_queue_family_index, this->present_queue_family_index);

    return true;

}

bool32_t spx::vk::vulkan_renderer::
create_logical_device()
{

    // Device extensions: presentation needs the swapchain extension, and MoltenVK requires the
    // portability subset extension to be enabled when the device advertises it.
    spx::dynamic_array<const char*> required_device_extensions;
    required_device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // VK_KHR_portability_subset is mandatory to enable *only* on portability drivers that advertise
    // it (e.g. MoltenVK). Native drivers (e.g. kosmickrisp) don't expose it, so enabling it
    // unconditionally would fail device creation -- gate it on actual support.
    if (this->physical_device.supports_extension("VK_KHR_portability_subset"))
        required_device_extensions.emplace_back("VK_KHR_portability_subset");

    // One queue per unique family. When graphics and presentation share a family (the common case)
    // we request a single queue; otherwise we request one from each. priority is referenced by the
    // create infos, so it must outlive the create_device call below.
    const float queue_priority = 1.0f;
    spx::dynamic_array<spx::vk::device_queue_create_info_t> queue_create_infos;

    {
        spx::vk::device_queue_create_info_t queue_info { };
        queue_info.set_queue_family_index(this->graphics_queue_family_index);
        queue_info.set_queue_priorities({ &queue_priority, 1 });
        queue_create_infos.emplace_back(queue_info);
    }

    if (this->present_queue_family_index != this->graphics_queue_family_index)
    {
        spx::vk::device_queue_create_info_t queue_info { };
        queue_info.set_queue_family_index(this->present_queue_family_index);
        queue_info.set_queue_priorities({ &queue_priority, 1 });
        queue_create_infos.emplace_back(queue_info);
    }

    spx::vk::device_create_info_t device_create_info { };
    device_create_info.set_queue_create_infos(queue_create_infos);
    device_create_info.set_extensions(required_device_extensions);

    const auto result = spx::vk::create_device(this->physical_device, device_create_info, NULL, this->device);
    if (result != VK_SUCCESS)
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create the vulkan logical device.");
        return false;
    }

    spx::logger::dispatch_diagnostic_log("Created the vulkan logical device successfully.");

    // Grab the queue handles (queue 0 from each requested family). When the families coincide both
    // handles refer to the same underlying queue.
    spx::vk::get_device_queue(this->device, this->graphics_queue_family_index, 0, this->graphics_queue);
    spx::vk::get_device_queue(this->device, this->present_queue_family_index, 0, this->present_queue);

    return true;

}

bool32_t spx::vk::vulkan_renderer::
create_surface()
{

    // Fetch the window.
    spx::window_interface *window = this->get_window();

    // The window owns platform-specific surface creation; hand it our instance and let it populate
    // the surface handle.
    const WindowStatus status = window->create_vulkan_surface(
        this->instance, NULL, &this->surface.native);
    if (status != WindowStatus_OK)
    {
        THROW_SIMPLEX_RENDERER_EXCEPTION("Failed to create the vulkan presentation surface.");
        return false;
    }

    spx::logger::dispatch_diagnostic_log("Created the vulkan presentation surface successfully.");

    return true;
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