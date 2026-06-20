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

    /// @brief Wraps vkDestroySurfaceKHR. Null-safe; the handle is nulled afterward either way.
    /// @param inst      The instance the surface was created against.
    /// @param surface   The surface to destroy.
    /// @param allocator Optional allocation callbacks (must match the surface's creation).
    inline void
    destroy_surface(instance_t& inst, surface_t& surface, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (surface.native != nullptr) vkDestroySurfaceKHR(inst.native, surface.native, allocator);
        surface.native = nullptr;
    }

    /// @brief Wraps vkGetPhysicalDeviceSurfaceSupportKHR (does the queue family support presentation
    ///        to this surface).
    /// @param device              The physical device to query.
    /// @param queue_family_index  The queue family to test.
    /// @param surface             The surface to test against.
    /// @param out_supported       Receives VK_TRUE/VK_FALSE.
    /// @return The native VkResult.
    inline VkResult
    get_physical_device_surface_support(physical_device_t& device, uint32_t queue_family_index,
                                        surface_t& surface, VkBool32* out_supported)
    {
        return vkGetPhysicalDeviceSurfaceSupportKHR(device.native, queue_family_index,
            surface.native, out_supported);
    }

    /// @brief Wraps vkGetPhysicalDeviceSurfaceCapabilitiesKHR, writing into a wrapped capabilities
    ///        struct.
    /// @param device           The physical device to query.
    /// @param surface          The surface to query.
    /// @param out_capabilities The capabilities struct to populate.
    /// @return The native VkResult.
    inline VkResult
    get_physical_device_surface_capabilities(physical_device_t& device, surface_t& surface,
                                             surface_capabilities_t& out_capabilities)
    {
        VkSurfaceCapabilitiesKHR& native_capabilities = out_capabilities;
        return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.native, surface.native,
            &native_capabilities);
    }

    /// @brief Wraps vkGetPhysicalDeviceSurfaceFormatsKHR.
    ///
    /// Native two-call pattern: pass out_formats == nullptr to query the count, then call again with
    /// a buffer sized to it. surface_format_t is layout-identical to VkSurfaceFormatKHR (guarded in
    /// structures.hpp), so the wrapper array is written into directly.
    /// @param device      The physical device to query.
    /// @param surface     The surface to query.
    /// @param count       In/out: capacity of out_formats, and the number actually written.
    /// @param out_formats Buffer of surface_format_t, or nullptr to only query the count.
    /// @return The native VkResult.
    inline VkResult
    get_physical_device_surface_formats(physical_device_t& device, surface_t& surface,
                                        uint32_t* count, surface_format_t* out_formats)
    {
        VkSurfaceFormatKHR* native_formats =
            out_formats ? &static_cast<VkSurfaceFormatKHR&>(*out_formats) : nullptr;
        return vkGetPhysicalDeviceSurfaceFormatsKHR(device.native, surface.native, count,
            native_formats);
    }

    /// @brief Wraps vkGetPhysicalDeviceSurfacePresentModesKHR.
    ///
    /// Native two-call pattern. VkPresentModeKHR is a plain enum, so present modes are written into a
    /// raw VkPresentModeKHR buffer (no wrapper).
    /// @param device     The physical device to query.
    /// @param surface    The surface to query.
    /// @param count      In/out: capacity of out_present_modes, and the number actually written.
    /// @param out_present_modes Buffer of present modes, or nullptr to only query the count.
    /// @return The native VkResult.
    inline VkResult
    get_physical_device_surface_present_modes(physical_device_t& device, surface_t& surface,
                                              uint32_t* count, VkPresentModeKHR* out_present_modes)
    {
        return vkGetPhysicalDeviceSurfacePresentModesKHR(device.native, surface.native, count,
            out_present_modes);
    }

    /// @brief Wraps vkGetPhysicalDeviceQueueFamilyProperties2.
    ///
    /// Native two-call pattern: pass out_properties == nullptr to query the count, then call again
    /// with a buffer sized to it. queue_family_properties_2_t is layout-identical to
    /// VkQueueFamilyProperties2 (guarded in structures.hpp), so the wrapper array is written
    /// directly; read the per-family data through .get_queue_family_properties().
    /// @param device        The physical device to query.
    /// @param count         In/out: capacity of out_properties, and the number actually written.
    /// @param out_properties Buffer of queue_family_properties_2_t, or nullptr to only query count.
    inline void
    get_physical_device_queue_family_properties(physical_device_t& device, uint32_t* count,
                                                 queue_family_properties_2_t* out_properties)
    {
        VkQueueFamilyProperties2* native_properties =
            out_properties ? &static_cast<VkQueueFamilyProperties2&>(*out_properties) : nullptr;
        vkGetPhysicalDeviceQueueFamilyProperties2(device.native, count, native_properties);
    }

    /// @brief Wraps vkCreateDevice, taking a wrapped create-info and writing into a wrapped handle.
    /// @param physical_device The physical device to create the logical device against.
    /// @param create_info     The device create info (queues, extensions, features).
    /// @param allocator       Optional allocation callbacks (may be nullptr).
    /// @param out_device      The handle to populate on success.
    /// @return The native VkResult from vkCreateDevice.
    inline VkResult
    create_device(physical_device_t& physical_device,
                  const device_create_info_t& create_info,
                  const VkAllocationCallbacks* allocator,
                  device_t& out_device)
    {
        const VkDeviceCreateInfo& native_create_info = create_info;
        return vkCreateDevice(physical_device.native, &native_create_info, allocator, &out_device.native);
    }

    /// @brief Wraps vkDestroyDevice, taking a wrapped device handle. Null-safe; the handle is nulled
    ///        afterward either way.
    /// @param device    The logical device to destroy.
    /// @param allocator Optional allocation callbacks (must match what was passed to create_device).
    inline void
    destroy_device(device_t& device, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (device.native != nullptr) vkDestroyDevice(device.native, allocator);
        device.native = nullptr;
    }

    /// @brief Wraps vkGetDeviceQueue, retrieving a queue handle from a created logical device.
    /// @param device             The logical device the queue belongs to.
    /// @param queue_family_index The family the queue was requested from.
    /// @param queue_index        The index within that family.
    /// @param out_queue          The handle to populate.
    inline void
    get_device_queue(device_t& device, uint32_t queue_family_index, uint32_t queue_index,
                     queue_t& out_queue)
    {
        vkGetDeviceQueue(device.native, queue_family_index, queue_index, &out_queue.native);
    }

    /// @brief Wraps vkDeviceWaitIdle. Blocks until the device has finished all outstanding work.
    ///        Used before tearing down swapchain resources so nothing is destroyed while in use.
    /// @param device The logical device to wait on.
    /// @return The native VkResult from vkDeviceWaitIdle.
    inline VkResult
    device_wait_idle(device_t& device)
    {
        return vkDeviceWaitIdle(device.native);
    }

    /// @brief Wraps vkCreateSwapchainKHR, taking a wrapped create-info and writing into a wrapped
    ///        handle.
    /// @param device       The logical device to create the swapchain against.
    /// @param create_info  The swapchain create info.
    /// @param allocator    Optional allocation callbacks (may be nullptr).
    /// @param out_swapchain The handle to populate on success.
    /// @return The native VkResult from vkCreateSwapchainKHR.
    inline VkResult
    create_swapchain(device_t& device,
                     const swapchain_create_info_t& create_info,
                     const VkAllocationCallbacks* allocator,
                     swapchain_t& out_swapchain)
    {
        const VkSwapchainCreateInfoKHR& native_create_info = create_info;
        return vkCreateSwapchainKHR(device.native, &native_create_info, allocator, &out_swapchain.native);
    }

    /// @brief Wraps vkDestroySwapchainKHR. Null-safe; the handle is nulled afterward either way.
    /// @param device    The logical device the swapchain was created against.
    /// @param swapchain The swapchain to destroy.
    /// @param allocator Optional allocation callbacks (must match create_swapchain).
    inline void
    destroy_swapchain(device_t& device, swapchain_t& swapchain, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (swapchain.native != nullptr) vkDestroySwapchainKHR(device.native, swapchain.native, allocator);
        swapchain.native = nullptr;
    }

    /// @brief Wraps vkCreateImageView, taking a wrapped create-info and writing into a wrapped handle.
    /// @param device      The logical device to create the view against.
    /// @param create_info The image view create info.
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_view    The handle to populate on success.
    /// @return The native VkResult from vkCreateImageView.
    inline VkResult
    create_image_view(device_t& device,
                      const image_view_create_info_t& create_info,
                      const VkAllocationCallbacks* allocator,
                      image_view_t& out_view)
    {
        const VkImageViewCreateInfo& native_create_info = create_info;
        return vkCreateImageView(device.native, &native_create_info, allocator, &out_view.native);
    }

    /// @brief Wraps vkDestroyImageView. Null-safe; the handle is nulled afterward either way.
    /// @param device    The logical device the view was created against.
    /// @param view      The view to destroy.
    /// @param allocator Optional allocation callbacks (must match create_image_view).
    inline void
    destroy_image_view(device_t& device, image_view_t& view, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (view.native != nullptr) vkDestroyImageView(device.native, view.native, allocator);
        view.native = nullptr;
    }

}
