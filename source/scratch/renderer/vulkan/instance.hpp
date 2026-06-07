#pragma once
#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/array_view.hpp>

#include <utils/system/logging_manager.hpp>

namespace spx::vk
{

    class vulkan_instance
    {

        public:
            inline  vulkan_instance() = default;
            inline ~vulkan_instance() = default;
        
            inline bool
            create(spx::array_view<const char *> required_extensions)
            {
                
                uint32_t extension_count = 0;
                vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

                spx::dynamic_array<VkExtensionProperties> extensions;
                extensions.reserve_to(extension_count);
                vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.begin());

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
                        LoggingManager::DispatchCritical(
                            "Failed to find required instance extension: {}",
                            required_extension
                        );
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
                this->instance_create_info.enabledLayerCount = 0;
                this->instance_create_info.ppEnabledLayerNames = NULL;
                
                // Create the instance.
                VkResult result = vkCreateInstance(&instance_create_info, NULL, &this->instance);
                if (result != VK_SUCCESS)
                {
                    LoggingManager::DispatchCritical("Failed to initialize vulkan instance.");
                    return false;
                }

                return true;
                
            }

            inline void
            destroy()
            {

                vkDestroyInstance(this->instance, NULL);
                this->instance = NULL;

            }

            inline bool is_valid() const { return this->instance != NULL; }
            inline operator VkInstance() { return this->instance; }
        
        public:
            VkInstance instance = NULL;
            VkApplicationInfo application_info = {};
            VkInstanceCreateInfo instance_create_info = {};


    };

}
