#pragma once
#include <glfw/glfw3.h>
#include <stdexcept>
#include <utils/system/logging_manager.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/array.hpp>

enum EngineResultType
{
    EngineResultType_OK,
    EngineResultType_NotInitialized,
    EngineResultType_InitializationFailed,
};

class vulkan_renderer
{

    public:
        inline 
        vulkan_renderer()
        {

        }

        inline 
        ~vulkan_renderer()
        {

        }

        inline EngineResultType
        is_initialized() const
        {

            if (this->instance != NULL)
            {
                return EngineResultType_NotInitialized;
            }

            return EngineResultType_OK;

        }

        inline EngineResultType
        initialize(GLFWwindow *window)
        {

            try
            {
                this->create_instance();
            }
            catch (const std::runtime_error& error)
            {
                LoggingManager::DispatchError("Vulkan Error: {}", error.what());
                return EngineResultType_InitializationFailed;
            }

            return EngineResultType_OK;

        }

        inline void
        deinitialize()
        {

        }

    private:
        inline bool
        validate_instance_extension_support(spx::array_view<const char*> check_extensions)
        {

            uint32_t extension_count = 0;
            vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

            spx::dynamic_array<VkExtensionProperties> extensions(extension_count);
            vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.begin());

            for (const auto &extension : extensions)
            {
                LoggingManager::DispatchDiagnostic(
                    "Vulkan Instance Extension {} is available.",
                    extension.extensionName);
            }

            for (const auto &current_check_extension : check_extensions)
            {

                spx::string_view<char> checking(current_check_extension);

                bool has_extension = false;
                for (const auto &extension : extensions)
                {
                    spx::string_view<char> current(extension.extensionName);
                    if (current == checking)
                    {
                        has_extension = true;
                        break;
                    }
                }

                if (!has_extension)
                {
                    LoggingManager::DispatchCritical(
                        "Failed to find instance extension {}.",
                        checking.data());
                    return false;
                }

            }

            return true;

        }

        inline void 
        create_instance()
        {

            uint32_t glfw_extension_count = 0;
            const char **glfw_extensions_list = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

            spx::array_view<const char *> instance_extensions_view(glfw_extensions_list, glfw_extension_count);
            if (!validate_instance_extension_support(instance_extensions_view))
            {
                throw std::runtime_error("Failed to validate required GLFW extensions.");
            }
            
            VkApplicationInfo application_info = {};
            application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            application_info.pApplicationName = "Simplex";
            application_info.pEngineName = "Simplex";
            application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            application_info.apiVersion = VK_API_VERSION_1_4;
            
            VkInstanceCreateInfo instance_create_info = {};
            instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instance_create_info.pApplicationInfo = &application_info;           
            instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions_view.size());
            instance_create_info.ppEnabledExtensionNames = instance_extensions_view.begin();
            instance_create_info.enabledLayerCount = 0;
            instance_create_info.ppEnabledLayerNames = NULL;
            
            VkResult result = vkCreateInstance(&instance_create_info, NULL, &this->instance);
            if (result != VK_SUCCESS) throw std::runtime_error("Failed to create VkInstance.");
            
        }

    private:
        VkInstance instance = NULL;

    private:
        GLFWwindow *window;

};