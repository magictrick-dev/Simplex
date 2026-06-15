#pragma once
#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <vulkan/vulkan.h>

#include <simplex/array_view.hpp>
#include <simplex/string_view.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/dynamic_string.hpp>

#include <simplex/renderer/renderer_utils.hpp>
#include <simplex/renderer/vulkan/structures.hpp>

#include <type_traits>
#include <cstddef>

namespace spx::vk
{

    /// @brief Mixin generator for vulkan handles.
    template <typename derived_t, typename native_t> struct vk_handle_ext { };

    /// @brief Wraps vulkan handles up.
    /// @tparam handle_type_t The handle type.
    ///
    /// We are wrapping vulkan handles with potential for mix-in helpers that can
    /// help shortcut some of the annoying boilerplate code that vulkan has.
    template <typename handle_type_t>
    struct vk_handle : vk_handle_ext<vk_handle<handle_type_t>, handle_type_t>
    {

        using native_type_t     = handle_type_t;
        native_type_t native    = { nullptr };

        inline operator native_type_t&()                { return this->native; }
        inline operator native_type_t const &() const   { return this->native; }

    };

    // ---------------------------------------------------------------------------------------------
    // Using statements.
    // ---------------------------------------------------------------------------------------------

    using instance_t        = vk_handle<VkInstance>;
    using physical_device_t = vk_handle<VkPhysicalDevice>;

    // ---------------------------------------------------------------------------------------------
    // Handle mixins.
    // ---------------------------------------------------------------------------------------------

    // @brief VkInstance mixin extensions.
    template <typename derived_t>
    struct vk_handle_ext<derived_t, VkInstance>
    {

        /// @brief Validates a list of instance extensions from the list of available extensions.
        /// @param requested_extensions The list of extensions to check.
        /// @return True if all extensions are available, false otherwise.
        static inline bool32_t
        validate_instance_extensions(spx::array_view<const char*> requested_extensions)
        {

            const auto available_extensions = get_available_instance_extensions();
            for (spx::string_view<char> requested_extension : requested_extensions)
            {

                bool32_t found = false;
                for (spx::string_view<char> available_extension : available_extensions)
                {

                    if (requested_extension == available_extension)
                    {
                        found = true;
                        break;
                    }

                }

                if (found == false) return false;

            }

            return true;

        }

        /// @brief Validates a list of instance layers from the list of available layers.
        /// @param requested_layers The lsit of layers to check.
        /// @return True if all extensions are available, false otherwise.
        static inline bool32_t
        validate_instance_layers(spx::array_view<const char*> requested_layers)
        {

            const auto available_layers = get_available_instance_layers();
            for (spx::string_view<char> requested_layer : requested_layers)
            {

                bool32_t found = false;
                for (spx::string_view<char> available_layer : available_layers)
                {

                    if (requested_layer == available_layer)
                    {
                        found = true;
                        break;
                    }

                }

                if (found == false) return false;

            }

            return true;

        }

        /// @brief Returns a list of available instance extensions.
        static inline spx::dynamic_array<spx::dynamic_string<char>>
        get_available_instance_extensions()
        {

            uint32_t extension_count = 0;
            vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

            spx::dynamic_array<VkExtensionProperties> extensions(extension_count);
            vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions.begin());

            spx::dynamic_array<spx::dynamic_string<char>> extension_names;
            for (auto extension : extensions) 
            {
                extension_names.emplace_back(extension.extensionName);
            }

            return std::move(extension_names);

        }

        /// @brief Returns a list of available instance layers.
        static inline spx::dynamic_array<spx::dynamic_string<char>>
        get_available_instance_layers()
        {

            uint32_t layers_count = 0;
            vkEnumerateInstanceLayerProperties(&layers_count, NULL);

            spx::dynamic_array<VkLayerProperties> layer_properties(layers_count);
            vkEnumerateInstanceLayerProperties(&layers_count, layer_properties.begin());

            spx::dynamic_array<spx::dynamic_string<char>> layers;
            for (const auto& layer_property : layer_properties)
            {
                layers.emplace_back(layer_property.layerName);
            }

            return std::move(layers);

        }

    };

    /// @brief VkPhysicalDevice mixin.
    template <typename derived_t>
    struct vk_handle_ext<derived_t, VkPhysicalDevice>
    {

    };

}