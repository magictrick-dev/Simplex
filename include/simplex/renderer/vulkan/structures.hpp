#pragma once
#include <utils/defs.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>
#include <utils/logging.hpp>

namespace spx::vk
{

    /// @brief A mixin extension strategy for specializations of certain vulkan structures.
    template <typename derived_t, typename native_t> struct vk_struct_ext { };

    /// @brief Provides a base structure for vulkan structures with .sType
    /// @tparam structure_type_t The type of Vulkan structure.
    /// @tparam structure_type The structure type.
    ///
    /// In most cases, you won't directly create a structure with this template, as
    /// they're aliased below with using statements. Overloads for the native type
    /// conversion to the actual vulkan calls are provided for you.
    template <typename structure_type_t, VkStructureType structure_type>
    struct vk_struct_wt : vk_struct_ext<vk_struct_wt<structure_type_t, structure_type>, structure_type_t>
    {

        using native_type_t     = structure_type_t;
        native_type_t native    = { .sType = structure_type };

        inline vk_struct_wt& set(const native_type_t &values) { native = values; native.sType = structure_type; };

        inline operator native_type_t&()                { return this->native;  }
        inline operator native_type_t*()                { return &this->native; }
        inline operator native_type_t const&() const    { return this->native;  }
        inline operator native_type_t const*() const    { return &this->native; }

    };

    /// @brief Provides a base structure for vulkan structures without an .sType.
    /// @tparam structure_type_t The type of Vulkan structure.
    ///
    /// In most cases, you won't directly create a structure with this template, as
    /// they're aliased below with using statements. Overloads for the native type
    /// conversion to the actual vulkan calls are provided for you.
    template <typename structure_type_t>
    struct vk_struct_wot : vk_struct_ext<vk_struct_wot<structure_type_t>, structure_type_t>
    {

        using native_type_t     = structure_type_t;
        native_type_t native    = { };

        inline vk_struct_wt& set(const native_type_t &values) { native = values; };

        inline operator native_type_t&()                { return this->native;  }
        inline operator native_type_t*()                { return &this->native; }
        inline operator native_type_t const&() const    { return this->native;  }
        inline operator native_type_t const*() const    { return &this->native; }

    };

    // Using statements.
    using instance_create_info                  = vk_struct_wt<VkInstanceCreateInfo,                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO>;
    using physical_device_properties            = vk_struct_wt<VkPhysicalDeviceProperties2,         VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2>;
    using physical_device_driver_properties     = vk_struct_wt<VkPhysicalDeviceDriverProperties,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES>;
    using physical_device_memory_properties     = vk_struct_wt<VkPhysicalDeviceMemoryProperties2,   VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2>;
    using physical_device_10_features           = vk_struct_wt<VkPhysicalDeviceFeatures2,           VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2>;
    using physical_device_11_features           = vk_struct_wt<VkPhysicalDeviceVulkan11Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES>;
    using physical_device_12_features           = vk_struct_wt<VkPhysicalDeviceVulkan12Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES>;
    using physical_device_13_features           = vk_struct_wt<VkPhysicalDeviceVulkan13Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES>;
    using physical_device_14_features           = vk_struct_wt<VkPhysicalDeviceVulkan14Features,    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES>;

    using application_info                      = vk_struct_wot<VkApplicationInfo>;

    template <typename derived_t>
    struct vk_struct_ext<derived_t, VkInstanceCreateInfo>
    {

        private:
            inline derived_t& self() 
            { 
                return static_cast<derived_t*>(*this); 
            }

        public:
            inline derived_t& set_next(const void* next) { self().native.pNext = next; return *this; };
            inline derived_t& set_flags(VkInstanceCreateFlags flags) { self().native.flags = flags; return *this; }

    };

}