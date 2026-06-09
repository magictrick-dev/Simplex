#pragma once
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/array_view.hpp>

#include <utils/logging.hpp>

namespace spx::vk
{

    class instance
    {

        public:
            inline  instance() = default;
            inline ~instance() = default;
        
            inline bool
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
