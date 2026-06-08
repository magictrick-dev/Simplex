#pragma once
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/array_view.hpp>

#include <utils/logging.hpp>

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

                // NOTE(Chris): Use the sizing constructor so that count is set; reserve_to()
                //              only grows capacity and would leave the array logically empty.
                spx::dynamic_array<VkExtensionProperties> extensions(extension_count);
                vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.begin());

                // NOTE(Chris): MoltenVK on macOS is a portability driver. We must request the
                //              portability enumeration extension (and set the matching flag below)
                //              or vkCreateInstance returns VK_ERROR_INCOMPATIBLE_DRIVER.
                spx::dynamic_array<const char *> enabled_extensions;
                for (auto required_extension : required_extensions)
                {
                    enabled_extensions.push_back(required_extension);
                }

#               if defined(__APPLE__)
                    enabled_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#               endif

                // NOTE(Chris): Prints out available extensions.
                for (auto extension : extensions)
                {
                    spx::logger::dispatch_diagnostic_log(
                        "Available Vulkan instance extension: {}",
                        extension.extensionName);
                }

                for (auto required_extension : enabled_extensions)
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
                this->application_info.apiVersion = VK_API_VERSION_1_3;
                
                // Setting up instance create info.
                this->instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                this->instance_create_info.pApplicationInfo = &application_info;
                this->instance_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
                this->instance_create_info.ppEnabledExtensionNames = enabled_extensions.begin();
                this->instance_create_info.enabledLayerCount = 0;
                this->instance_create_info.ppEnabledLayerNames = NULL;

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

                return true;
                
            }

            inline void
            destroy()
            {

                vkDestroyInstance(this->handle, NULL);
                this->handle= NULL;

            }

            inline bool 
            is_valid() const 
            { 
                return this->handle != NULL; 
            }

            inline 
            operator VkInstance() const 
            { 
                return this->handle; 
            }
        
        public:
            VkInstance handle = NULL;
            VkApplicationInfo application_info = {};
            VkInstanceCreateInfo instance_create_info = {};


    };

}
