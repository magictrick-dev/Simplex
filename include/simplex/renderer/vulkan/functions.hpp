#pragma once
#include <utils/defs.hpp>

#include <vulkan/vulkan.h>

#include <simplex/renderer/vulkan/structures.hpp>
#include <simplex/renderer/vulkan/handles.hpp>

namespace spx::vk
{

    /// @brief Thin, zero-overhead wrappers over the raw Vulkan entry points that accept the
    ///        wrapped spx::vk types directly.
    ///
    /// Each wrapper takes its arguments by reference and forwards them straight to the native
    /// call. The wrapped types convert to their native Vulkan structs/handles through a
    /// reinterpret_cast (the layout guards in structures.hpp enforce that this is valid).

    /// @brief Wraps vkCreateInstance, taking a wrapped create-info and writing into a wrapped handle.
    /// @param create_info  The instance create info.
    /// @param allocator    Optional allocation callbacks (may be nullptr).
    /// @param out_instance The handle to populate on success.
    /// @return The native VkResult from vkCreateInstance.
    inline VkResult
    create_instance(const instance_create_info_t& create_info,
                    const VkAllocationCallbacks* allocator,
                    instance_t& out_instance)
    {
        const VkInstanceCreateInfo& native_create_info = create_info;
        return vkCreateInstance(&native_create_info, allocator, &out_instance.native);
    }

    /// @brief Wraps vkDestroyInstance, taking a wrapped instance handle.
    /// @param inst      The instance to destroy.
    /// @param allocator Optional allocation callbacks (may be nullptr). Must match what was passed to create_instance.
    inline void
    destroy_instance(instance_t& inst, const VkAllocationCallbacks* allocator = nullptr)
    {
        vkDestroyInstance(inst.native, allocator);
        inst.native = nullptr;
    }

    /// @brief Wraps vkEnumeratePhysicalDevices.
    ///
    /// Follows the native two-call pattern: pass out_devices == nullptr to query the count, then
    /// call again with a buffer sized to that count. The physical_device wrapper is layout-identical
    /// to the raw VkPhysicalDevice (see the guards in handles.hpp), so the array of wrappers is
    /// written into directly without a copy.
    /// @param inst        The instance to enumerate over.
    /// @param count       In/out: capacity of out_devices, and the number actually written.
    /// @param out_devices Buffer of physical_device wrappers, or nullptr to only query the count.
    /// @return The native VkResult from vkEnumeratePhysicalDevices.
    inline VkResult
    enumerate_physical_devices(instance_t& inst, uint32_t* count, physical_device_t* out_devices)
    {
        return vkEnumeratePhysicalDevices(inst.native, count,
            out_devices ? &out_devices->native : nullptr);
    }

    /// @brief Wraps vkGetPhysicalDeviceProperties2, writing into a wrapped properties struct.
    ///
    /// Any version-specific property structs (physical_device_11/12/13/14_properties) chained onto
    /// out_properties via set_next are filled by the driver in the same call.
    /// @param device         The physical device to query.
    /// @param out_properties The properties struct (and its pNext chain) to populate.
    inline void
    get_physical_device_properties(physical_device_t& device, physical_device_10_properties_t& out_properties)
    {
        VkPhysicalDeviceProperties2& native_properties = out_properties;
        vkGetPhysicalDeviceProperties2(device.native, &native_properties);
    }

    /// @brief Wraps vkGetPhysicalDeviceFeatures2, writing into a wrapped features struct.
    ///
    /// Any version-specific feature structs (physical_device_11/12/13/14_features) chained onto
    /// out_features via set_next are filled by the driver in the same call.
    /// @param device       The physical device to query.
    /// @param out_features The features struct (and its pNext chain) to populate.
    inline void
    get_physical_device_features(physical_device_t& device, physical_device_10_features_t& out_features)
    {
        VkPhysicalDeviceFeatures2& native_features = out_features;
        vkGetPhysicalDeviceFeatures2(device.native, &native_features);
    }

    /// @brief Wraps vkCreateDebugUtilsMessengerEXT.
    ///
    /// This is an instance extension entry point and is not statically exported by the loader, so it
    /// is resolved through vkGetInstanceProcAddr at call time. If the VK_EXT_debug_utils extension is
    /// not enabled/present the lookup fails and VK_ERROR_EXTENSION_NOT_PRESENT is returned.
    /// @param inst          The instance the messenger is created against.
    /// @param create_info   The messenger create info (severity/type/callback).
    /// @param allocator     Optional allocation callbacks (may be nullptr).
    /// @param out_messenger The handle to populate on success.
    /// @return VK_ERROR_EXTENSION_NOT_PRESENT if the entry point is unavailable, otherwise the
    ///         native VkResult from vkCreateDebugUtilsMessengerEXT.
    inline VkResult
    create_debug_utils_messenger(instance_t& inst,
                                 const debug_utils_messenger_create_info_t& create_info,
                                 const VkAllocationCallbacks* allocator,
                                 debug_utils_messenger_t& out_messenger)
    {
        auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            inst.native, "vkCreateDebugUtilsMessengerEXT");
        if (fn == nullptr) return VK_ERROR_EXTENSION_NOT_PRESENT;

        const VkDebugUtilsMessengerCreateInfoEXT& native_create_info = create_info;
        return fn(inst.native, &native_create_info, allocator, &out_messenger.native);
    }

    /// @brief Wraps vkDestroyDebugUtilsMessengerEXT.
    ///
    /// Resolved through vkGetInstanceProcAddr like its create counterpart. Safe to call on a null
    /// messenger; the handle is nulled afterward either way.
    /// @param inst      The instance the messenger was created against.
    /// @param messenger The messenger to destroy.
    /// @param allocator Optional allocation callbacks (must match create_debug_utils_messenger).
    inline void
    destroy_debug_utils_messenger(instance_t& inst,
                                  debug_utils_messenger_t& messenger,
                                  const VkAllocationCallbacks* allocator = nullptr)
    {
        if (messenger.native != nullptr)
        {
            auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                inst.native, "vkDestroyDebugUtilsMessengerEXT");
            if (fn != nullptr) fn(inst.native, messenger.native, allocator);
        }
        messenger.native = nullptr;
    }

}
