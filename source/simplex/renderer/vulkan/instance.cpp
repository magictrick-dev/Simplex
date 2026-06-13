#include <simplex/renderer/vulkan/instance.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data)
{
    
    switch (severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        {
            spx::logger::dispatch_diagnostic_log("{}", callback_data->pMessage);
        } break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        {
            spx::logger::dispatch_information_log("{}" , callback_data->pMessage);
        } break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        {
            spx::logger::dispatch_critical_log("{}", callback_data->pMessage);
        } break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        {
            spx::logger::dispatch_error_log("{}", callback_data->pMessage);
        } break;

        default:
        {
            spx::logger::dispatch_information_log("{}", callback_data->pMessage);
        } break;
    }

    return VK_FALSE;
    
}

inline VkResult 
CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger) 
{

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

}

inline void 
DestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}


bool spx::vk::instance::
create(spx::array_view<const char *> required_extensions, spx::array_view<const char *> required_layers)
{
    
    // Get the layers.
    uint32_t layers_count = 0;
    vkEnumerateInstanceLayerProperties(&layers_count, NULL);

    spx::dynamic_array<VkLayerProperties> layer_properties(layers_count);
    vkEnumerateInstanceLayerProperties(&layers_count, layer_properties.begin());

    for (spx::string_view<char> required_layer : required_layers)
    {

        bool layer_exists = false;
        for (const auto &layer : layer_properties)
        {
            spx::string_view<char> current_layer(layer.layerName);
            if (current_layer == required_layer)
            {
                layer_exists = true;
                break;
            }
        }

        if (layer_exists == false)
        {
            throw std::runtime_error("Failed to find required validation layer!");
        }

    }
    
    // Get extensions.
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

    spx::dynamic_array<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.begin());

    // NOTE(Chris): Prints out available extensions.
    for (auto extension : extensions)
    {
        spx::logger::dispatch_diagnostic_log(
            "Available Vulkan instance extension: {}",
            extension.extensionName);
    }

    // NOTE(Chris): Now we actually validate the extensions.
    for (auto required_extension : required_extensions)
    {

        spx::string_view<char> check { required_extension };

        bool extension_exists = false;
        for (const auto &current : extensions)
        {
            
            spx::string_view<char> against { current.extensionName };
            if (against == check)
            {
                extension_exists = true;
                break;
            }
            
        }

        if (!extension_exists) 
        {
            spx::logger::dispatch_error_log(
                "Failed to find required instance extension: {}",
                required_extension
            );
            throw std::runtime_error("Failed to create Vulkan instance, missing required instance extension.");
            return false;
        }

    }

    // Setting up application info.
    this->application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    this->application_info.apiVersion = VK_API_VERSION_1_4;
    
    // Setting up instance create info.
    this->instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    this->instance_create_info.pApplicationInfo = &application_info;
    this->instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    this->instance_create_info.ppEnabledExtensionNames = required_extensions.begin();
    this->instance_create_info.enabledLayerCount = static_cast<uint32_t>(required_layers.size());
    this->instance_create_info.ppEnabledLayerNames = required_layers.begin();

#               if defined(__APPLE__)
        this->instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#               endif
    
    // Create the instance.
    VkResult result = vkCreateInstance(&instance_create_info, NULL, &this->handle);
    if (result != VK_SUCCESS)
    {
        spx::logger::dispatch_error_log("Failed to initialize vulkan instance (VkResult: {}).", static_cast<int>(result));
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = nullptr;

    result = CreateDebugUtilsMessengerEXT(this->handle, &create_info, NULL, &this->debug_messenger);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vulkan debug messenger.");
    }

    return true;
    
}

void spx::vk::instance::
destroy()
{

    DestroyDebugUtilsMessengerEXT(this->handle, this->debug_messenger, NULL);
    vkDestroyInstance(this->handle, NULL);
    this->debug_messenger = NULL;
    this->handle = NULL;

}

bool spx::vk::instance::
is_valid() const 
{ 
    return this->handle != NULL; 
}
