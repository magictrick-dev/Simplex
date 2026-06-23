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

    /// @brief Wraps vkCreateShaderModule, taking a wrapped create-info and writing into a wrapped
    ///        handle. The driver copies the SPIR-V during this call, so the source blob the create
    ///        info points at may be freed once it returns.
    /// @param device      The logical device to create the module against.
    /// @param create_info The shader module create info (a SPIR-V blob).
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_module  The handle to populate on success.
    /// @return The native VkResult from vkCreateShaderModule.
    inline VkResult
    create_shader_module(device_t& device,
                         const shader_module_create_info_t& create_info,
                         const VkAllocationCallbacks* allocator,
                         shader_module_t& out_module)
    {
        const VkShaderModuleCreateInfo& native_create_info = create_info;
        return vkCreateShaderModule(device.native, &native_create_info, allocator, &out_module.native);
    }

    /// @brief Wraps vkDestroyShaderModule. Null-safe; the handle is nulled afterward either way.
    /// @param device        The logical device the module was created against.
    /// @param shader_module The module to destroy.
    /// @param allocator     Optional allocation callbacks (must match create_shader_module).
    inline void
    destroy_shader_module(device_t& device, shader_module_t& shader_module, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (shader_module.native != nullptr) vkDestroyShaderModule(device.native, shader_module.native, allocator);
        shader_module.native = nullptr;
    }

    /// @brief Wraps vkCreatePipelineLayout, taking a wrapped create-info and writing into a wrapped
    ///        handle. The descriptor-set-layout and push-constant arrays the create info points at are
    ///        consumed during this call, so they need only live until it returns.
    /// @param device      The logical device to create the layout against.
    /// @param create_info The pipeline layout create info (set layouts + push constant ranges).
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_layout  The handle to populate on success.
    /// @return The native VkResult from vkCreatePipelineLayout.
    inline VkResult
    create_pipeline_layout(device_t& device,
                           const pipeline_layout_create_info_t& create_info,
                           const VkAllocationCallbacks* allocator,
                           pipeline_layout_t& out_layout)
    {
        const VkPipelineLayoutCreateInfo& native_create_info = create_info;
        return vkCreatePipelineLayout(device.native, &native_create_info, allocator, &out_layout.native);
    }

    /// @brief Wraps vkDestroyPipelineLayout. Null-safe; the handle is nulled afterward either way.
    /// @param device    The logical device the layout was created against.
    /// @param layout    The pipeline layout to destroy.
    /// @param allocator Optional allocation callbacks (must match create_pipeline_layout).
    inline void
    destroy_pipeline_layout(device_t& device, pipeline_layout_t& layout, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (layout.native != nullptr) vkDestroyPipelineLayout(device.native, layout.native, allocator);
        layout.native = nullptr;
    }

    /// @brief Wraps vkCreateGraphicsPipelines for the single-pipeline case (createInfoCount == 1).
    ///
    /// Everything the create info points at (shader stages, state structs, any chained
    /// pipeline_rendering_create_info_t) must still be alive for the duration of this call. The
    /// optional pipeline cache lets the driver reuse prior compilation work; pass VK_NULL_HANDLE to
    /// skip it.
    /// @param device       The logical device to create the pipeline against.
    /// @param create_info  The graphics pipeline create info.
    /// @param allocator    Optional allocation callbacks (may be nullptr).
    /// @param out_pipeline The handle to populate on success.
    /// @param cache        Optional pipeline cache (defaults to VK_NULL_HANDLE).
    /// @return The native VkResult from vkCreateGraphicsPipelines.
    inline VkResult
    create_graphics_pipeline(device_t& device,
                             const graphics_pipeline_create_info_t& create_info,
                             const VkAllocationCallbacks* allocator,
                             pipeline_t& out_pipeline,
                             VkPipelineCache cache = VK_NULL_HANDLE)
    {
        const VkGraphicsPipelineCreateInfo& native_create_info = create_info;
        return vkCreateGraphicsPipelines(device.native, cache, 1, &native_create_info, allocator,
            &out_pipeline.native);
    }

    /// @brief Wraps vkDestroyPipeline. Null-safe; the handle is nulled afterward either way.
    /// @param device    The logical device the pipeline was created against.
    /// @param pipeline  The pipeline to destroy.
    /// @param allocator Optional allocation callbacks (must match create_graphics_pipeline).
    inline void
    destroy_pipeline(device_t& device, pipeline_t& pipeline, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (pipeline.native != nullptr) vkDestroyPipeline(device.native, pipeline.native, allocator);
        pipeline.native = nullptr;
    }

    // ---------------------------------------------------------------------------------------------
    // Command pools and command buffers.
    // ---------------------------------------------------------------------------------------------

    /// @brief Wraps vkCreateCommandPool, taking a wrapped create-info and writing into a wrapped handle.
    /// @param device      The logical device to create the pool against.
    /// @param create_info The command pool create info (queue family + flags).
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_pool    The handle to populate on success.
    /// @return The native VkResult from vkCreateCommandPool.
    inline VkResult
    create_command_pool(device_t& device,
                        const command_pool_create_info_t& create_info,
                        const VkAllocationCallbacks* allocator,
                        command_pool_t& out_pool)
    {
        const VkCommandPoolCreateInfo& native_create_info = create_info;
        return vkCreateCommandPool(device.native, &native_create_info, allocator, &out_pool.native);
    }

    /// @brief Wraps vkDestroyCommandPool. Null-safe; the handle is nulled afterward either way.
    ///        Destroying the pool frees every command buffer allocated from it.
    /// @param device    The logical device the pool was created against.
    /// @param pool      The command pool to destroy.
    /// @param allocator Optional allocation callbacks (must match create_command_pool).
    inline void
    destroy_command_pool(device_t& device, command_pool_t& pool, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (pool.native != nullptr) vkDestroyCommandPool(device.native, pool.native, allocator);
        pool.native = nullptr;
    }

    /// @brief Allocates a single command buffer from a pool (the common case). Builds a one-buffer
    ///        VkCommandBufferAllocateInfo internally. For multiple buffers, call once per buffer.
    /// @param device     The logical device the pool belongs to.
    /// @param pool       The pool to allocate from.
    /// @param out_buffer The handle to populate on success.
    /// @param level      Primary (default) or secondary.
    /// @return The native VkResult from vkAllocateCommandBuffers.
    inline VkResult
    allocate_command_buffer(device_t& device, command_pool_t& pool, command_buffer_t& out_buffer,
                            VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {
        command_buffer_allocate_info_t info { };
        info.commandPool        = pool.native;
        info.level              = level;
        info.commandBufferCount = 1;

        const VkCommandBufferAllocateInfo& native_info = info;
        return vkAllocateCommandBuffers(device.native, &native_info, &out_buffer.native);
    }

    /// @brief Frees a single command buffer back to its pool. Null-safe; the handle is nulled
    ///        afterward either way.
    /// @param device The logical device the pool belongs to.
    /// @param pool   The pool the buffer was allocated from.
    /// @param buffer The buffer to free.
    inline void
    free_command_buffer(device_t& device, command_pool_t& pool, command_buffer_t& buffer)
    {
        if (buffer.native != nullptr) vkFreeCommandBuffers(device.native, pool.native, 1, &buffer.native);
        buffer.native = nullptr;
    }

    /// @brief Wraps vkBeginCommandBuffer, taking a wrapped begin-info. Transitions the buffer into the
    ///        recording state; subsequent cmd_* calls record into it until end_command_buffer.
    /// @param buffer The command buffer to begin recording.
    /// @param info   The begin info (usage flags).
    /// @return The native VkResult.
    inline VkResult
    begin_command_buffer(command_buffer_t& buffer, const command_buffer_begin_info_t& info)
    {
        const VkCommandBufferBeginInfo& native_info = info;
        return vkBeginCommandBuffer(buffer.native, &native_info);
    }

    /// @brief Wraps vkEndCommandBuffer. Finishes recording; the buffer is then submittable.
    /// @param buffer The command buffer to finish.
    /// @return The native VkResult.
    inline VkResult
    end_command_buffer(command_buffer_t& buffer)
    {
        return vkEndCommandBuffer(buffer.native);
    }

    /// @brief Wraps vkResetCommandBuffer. Returns the buffer to the initial state for re-recording
    ///        (the pool must have been created with RESET_COMMAND_BUFFER).
    /// @param buffer The command buffer to reset.
    /// @param flags  Reset flags (default 0).
    /// @return The native VkResult.
    inline VkResult
    reset_command_buffer(command_buffer_t& buffer, VkCommandBufferResetFlags flags = 0)
    {
        return vkResetCommandBuffer(buffer.native, flags);
    }

    // ---------------------------------------------------------------------------------------------
    // Synchronization primitives (semaphores and fences).
    // ---------------------------------------------------------------------------------------------

    /// @brief Wraps vkCreateSemaphore, taking a wrapped create-info and writing into a wrapped handle.
    /// @param device       The logical device to create the semaphore against.
    /// @param create_info  The semaphore create info.
    /// @param allocator    Optional allocation callbacks (may be nullptr).
    /// @param out_semaphore The handle to populate on success.
    /// @return The native VkResult from vkCreateSemaphore.
    inline VkResult
    create_semaphore(device_t& device,
                     const semaphore_create_info_t& create_info,
                     const VkAllocationCallbacks* allocator,
                     semaphore_t& out_semaphore)
    {
        const VkSemaphoreCreateInfo& native_create_info = create_info;
        return vkCreateSemaphore(device.native, &native_create_info, allocator, &out_semaphore.native);
    }

    /// @brief Wraps vkDestroySemaphore. Null-safe; the handle is nulled afterward either way.
    /// @param device    The logical device the semaphore was created against.
    /// @param semaphore The semaphore to destroy.
    /// @param allocator Optional allocation callbacks (must match create_semaphore).
    inline void
    destroy_semaphore(device_t& device, semaphore_t& semaphore, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (semaphore.native != nullptr) vkDestroySemaphore(device.native, semaphore.native, allocator);
        semaphore.native = nullptr;
    }

    /// @brief Wraps vkCreateFence, taking a wrapped create-info and writing into a wrapped handle.
    /// @param device      The logical device to create the fence against.
    /// @param create_info The fence create info (set_signaled to create it already signaled).
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_fence   The handle to populate on success.
    /// @return The native VkResult from vkCreateFence.
    inline VkResult
    create_fence(device_t& device,
                 const fence_create_info_t& create_info,
                 const VkAllocationCallbacks* allocator,
                 fence_t& out_fence)
    {
        const VkFenceCreateInfo& native_create_info = create_info;
        return vkCreateFence(device.native, &native_create_info, allocator, &out_fence.native);
    }

    /// @brief Wraps vkDestroyFence. Null-safe; the handle is nulled afterward either way.
    /// @param device    The logical device the fence was created against.
    /// @param fence     The fence to destroy.
    /// @param allocator Optional allocation callbacks (must match create_fence).
    inline void
    destroy_fence(device_t& device, fence_t& fence, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (fence.native != nullptr) vkDestroyFence(device.native, fence.native, allocator);
        fence.native = nullptr;
    }

    /// @brief Waits on a single fence (the per-frame case). Wraps vkWaitForFences with count 1.
    /// @param device  The logical device the fence belongs to.
    /// @param fence   The fence to wait on.
    /// @param timeout Timeout in nanoseconds (default: wait indefinitely).
    /// @return The native VkResult (VK_SUCCESS, or VK_TIMEOUT if the timeout elapses).
    inline VkResult
    wait_for_fence(device_t& device, fence_t& fence, uint64_t timeout = UINT64_MAX)
    {
        return vkWaitForFences(device.native, 1, &fence.native, VK_TRUE, timeout);
    }

    /// @brief Resets a single fence back to the unsignaled state. Wraps vkResetFences with count 1.
    /// @param device The logical device the fence belongs to.
    /// @param fence  The fence to reset.
    /// @return The native VkResult.
    inline VkResult
    reset_fence(device_t& device, fence_t& fence)
    {
        return vkResetFences(device.native, 1, &fence.native);
    }

    // ---------------------------------------------------------------------------------------------
    // Presentation and submission.
    // ---------------------------------------------------------------------------------------------

    /// @brief Wraps vkAcquireNextImageKHR. Acquires the index of the next presentable swapchain image,
    ///        signaling a semaphore (and optionally a fence) when the image is ready to render into.
    /// @param device           The logical device the swapchain belongs to.
    /// @param swapchain        The swapchain to acquire from.
    /// @param timeout          Timeout in nanoseconds (UINT64_MAX to wait indefinitely).
    /// @param signal_semaphore Semaphore signaled when the image is available.
    /// @param fence            Optional fence signaled when the image is available (may be nullptr).
    /// @param out_image_index  Receives the acquired image index.
    /// @return The native VkResult (notably VK_ERROR_OUT_OF_DATE_KHR / VK_SUBOPTIMAL_KHR on resize).
    inline VkResult
    acquire_next_image(device_t& device, swapchain_t& swapchain, uint64_t timeout,
                       semaphore_t& signal_semaphore, fence_t* fence, uint32_t* out_image_index)
    {
        return vkAcquireNextImageKHR(device.native, swapchain.native, timeout,
            signal_semaphore.native, fence ? fence->native : VK_NULL_HANDLE, out_image_index);
    }

    /// @brief Wraps vkQueueSubmit for the single-submit case (submitCount == 1). The wait/signal
    ///        semaphores and command buffers come from the submit info; the optional fence is signaled
    ///        when the submitted work completes.
    /// @param queue The queue to submit to.
    /// @param info  The submit info (wait/signal semaphores, command buffers).
    /// @param fence Optional fence signaled on completion (may be nullptr).
    /// @return The native VkResult.
    inline VkResult
    queue_submit(queue_t& queue, const submit_info_t& info, fence_t* fence = nullptr)
    {
        const VkSubmitInfo& native_info = info;
        return vkQueueSubmit(queue.native, 1, &native_info, fence ? fence->native : VK_NULL_HANDLE);
    }

    /// @brief Wraps vkQueueWaitIdle. Blocks until the queue has finished all submitted work.
    /// @param queue The queue to wait on.
    /// @return The native VkResult.
    inline VkResult
    queue_wait_idle(queue_t& queue)
    {
        return vkQueueWaitIdle(queue.native);
    }

    /// @brief Wraps vkQueuePresentKHR, taking a wrapped present-info.
    /// @param queue The presentation queue.
    /// @param info  The present info (wait semaphores, swapchains, image indices).
    /// @return The native VkResult (notably VK_ERROR_OUT_OF_DATE_KHR / VK_SUBOPTIMAL_KHR on resize).
    inline VkResult
    queue_present(queue_t& queue, const present_info_t& info)
    {
        const VkPresentInfoKHR& native_info = info;
        return vkQueuePresentKHR(queue.native, &native_info);
    }

    // ---------------------------------------------------------------------------------------------
    // Command recording.
    //
    // Recorded between begin_command_buffer and end_command_buffer. cmd_begin_rendering /
    // cmd_end_rendering are the dynamic-rendering replacement for the render-pass begin/end scope.
    // ---------------------------------------------------------------------------------------------

    /// @brief Wraps vkCmdBeginRendering (Vulkan 1.3 core). Opens a dynamic-rendering scope described
    ///        by the rendering info (render area + attachments); draw commands are recorded inside it.
    /// @param buffer The command buffer being recorded.
    /// @param info   The rendering info.
    inline void
    cmd_begin_rendering(command_buffer_t& buffer, const rendering_info_t& info)
    {
        const VkRenderingInfo& native_info = info;
        vkCmdBeginRendering(buffer.native, &native_info);
    }

    /// @brief Wraps vkCmdEndRendering. Closes the dynamic-rendering scope opened by cmd_begin_rendering.
    /// @param buffer The command buffer being recorded.
    inline void
    cmd_end_rendering(command_buffer_t& buffer)
    {
        vkCmdEndRendering(buffer.native);
    }

    /// @brief Wraps vkCmdBindPipeline.
    /// @param buffer     The command buffer being recorded.
    /// @param bind_point The bind point (e.g. VK_PIPELINE_BIND_POINT_GRAPHICS).
    /// @param pipeline   The pipeline to bind.
    inline void
    cmd_bind_pipeline(command_buffer_t& buffer, VkPipelineBindPoint bind_point, pipeline_t& pipeline)
    {
        vkCmdBindPipeline(buffer.native, bind_point, pipeline.native);
    }

    /// @brief Wraps vkCmdSetViewport for the single-viewport case (sets viewport 0). Used when the
    ///        pipeline declares VK_DYNAMIC_STATE_VIEWPORT.
    /// @param buffer   The command buffer being recorded.
    /// @param viewport The viewport to set.
    inline void
    cmd_set_viewport(command_buffer_t& buffer, const viewport_t& viewport)
    {
        const VkViewport& native_viewport = viewport;
        vkCmdSetViewport(buffer.native, 0, 1, &native_viewport);
    }

    /// @brief Wraps vkCmdSetScissor for the single-scissor case (sets scissor 0). Used when the
    ///        pipeline declares VK_DYNAMIC_STATE_SCISSOR.
    /// @param buffer  The command buffer being recorded.
    /// @param scissor The scissor rectangle to set.
    inline void
    cmd_set_scissor(command_buffer_t& buffer, const rect_2d_t& scissor)
    {
        const VkRect2D& native_scissor = scissor;
        vkCmdSetScissor(buffer.native, 0, 1, &native_scissor);
    }

    /// @brief Wraps vkCmdDraw (non-indexed draw).
    /// @param buffer         The command buffer being recorded.
    /// @param vertex_count   Vertices to draw.
    /// @param instance_count Instances to draw.
    /// @param first_vertex   Index of the first vertex.
    /// @param first_instance Index of the first instance.
    inline void
    cmd_draw(command_buffer_t& buffer, uint32_t vertex_count, uint32_t instance_count,
             uint32_t first_vertex, uint32_t first_instance)
    {
        vkCmdDraw(buffer.native, vertex_count, instance_count, first_vertex, first_instance);
    }

    /// @brief Wraps vkCmdBindVertexBuffers for the single-binding case.
    /// @param buffer        The command buffer being recorded.
    /// @param binding       The binding number to bind to.
    /// @param vertex_buffer The vertex buffer to bind.
    /// @param offset        Byte offset into the buffer (default 0).
    inline void
    cmd_bind_vertex_buffer(command_buffer_t& buffer, uint32_t binding, buffer_t& vertex_buffer,
                           VkDeviceSize offset = 0)
    {
        VkBuffer native_buffer = vertex_buffer.native;
        vkCmdBindVertexBuffers(buffer.native, binding, 1, &native_buffer, &offset);
    }

    /// @brief Wraps vkCmdCopyBuffer for the single-region case. Used to upload from a host-visible
    ///        staging buffer into a device-local vertex buffer.
    /// @param buffer The command buffer being recorded (on a transfer-capable queue).
    /// @param src    The source buffer.
    /// @param dst    The destination buffer.
    /// @param region The copy region (src offset, dst offset, size).
    inline void
    cmd_copy_buffer(command_buffer_t& buffer, buffer_t& src, buffer_t& dst, const VkBufferCopy& region)
    {
        vkCmdCopyBuffer(buffer.native, src.native, dst.native, 1, &region);
    }

    // ---------------------------------------------------------------------------------------------
    // Buffers and device memory.
    // ---------------------------------------------------------------------------------------------

    /// @brief Wraps vkCreateBuffer, taking a wrapped create-info and writing into a wrapped handle.
    ///        A freshly created buffer has no backing store -- allocate_memory + bind_buffer_memory
    ///        must follow before it can be used.
    /// @param device      The logical device to create the buffer against.
    /// @param create_info The buffer create info (size, usage, sharing).
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_buffer  The handle to populate on success.
    /// @return The native VkResult from vkCreateBuffer.
    inline VkResult
    create_buffer(device_t& device,
                  const buffer_create_info_t& create_info,
                  const VkAllocationCallbacks* allocator,
                  buffer_t& out_buffer)
    {
        const VkBufferCreateInfo& native_create_info = create_info;
        return vkCreateBuffer(device.native, &native_create_info, allocator, &out_buffer.native);
    }

    /// @brief Wraps vkDestroyBuffer. Null-safe; the handle is nulled afterward either way. Does not
    ///        free the bound device memory -- free that separately with free_memory.
    /// @param device    The logical device the buffer was created against.
    /// @param buffer    The buffer to destroy.
    /// @param allocator Optional allocation callbacks (must match create_buffer).
    inline void
    destroy_buffer(device_t& device, buffer_t& buffer, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (buffer.native != nullptr) vkDestroyBuffer(device.native, buffer.native, allocator);
        buffer.native = nullptr;
    }

    /// @brief Wraps vkGetBufferMemoryRequirements, writing into a wrapped requirements struct. The
    ///        returned memoryTypeBits feeds physical_device_t::find_memory_type.
    /// @param device           The logical device the buffer belongs to.
    /// @param buffer           The buffer to query.
    /// @param out_requirements The requirements struct to populate.
    inline void
    get_buffer_memory_requirements(device_t& device, buffer_t& buffer, memory_requirements_t& out_requirements)
    {
        VkMemoryRequirements& native_requirements = out_requirements;
        vkGetBufferMemoryRequirements(device.native, buffer.native, &native_requirements);
    }

    /// @brief Wraps vkAllocateMemory, taking a wrapped allocate-info and writing into a wrapped handle.
    /// @param device      The logical device to allocate from.
    /// @param info        The memory allocate info (size + type index).
    /// @param allocator   Optional allocation callbacks (may be nullptr).
    /// @param out_memory  The handle to populate on success.
    /// @return The native VkResult from vkAllocateMemory.
    inline VkResult
    allocate_memory(device_t& device,
                    const memory_allocate_info_t& info,
                    const VkAllocationCallbacks* allocator,
                    device_memory_t& out_memory)
    {
        const VkMemoryAllocateInfo& native_info = info;
        return vkAllocateMemory(device.native, &native_info, allocator, &out_memory.native);
    }

    /// @brief Wraps vkFreeMemory. Null-safe; the handle is nulled afterward either way. Any buffer
    ///        still bound to this memory must already have been destroyed.
    /// @param device    The logical device the memory was allocated from.
    /// @param memory    The memory to free.
    /// @param allocator Optional allocation callbacks (must match allocate_memory).
    inline void
    free_memory(device_t& device, device_memory_t& memory, const VkAllocationCallbacks* allocator = nullptr)
    {
        if (memory.native != nullptr) vkFreeMemory(device.native, memory.native, allocator);
        memory.native = nullptr;
    }

    /// @brief Wraps vkBindBufferMemory. Backs a buffer with a region of an allocation.
    /// @param device The logical device.
    /// @param buffer The buffer to bind.
    /// @param memory The memory to bind it to.
    /// @param offset Byte offset into the memory (must satisfy the buffer's alignment; default 0).
    /// @return The native VkResult.
    inline VkResult
    bind_buffer_memory(device_t& device, buffer_t& buffer, device_memory_t& memory, VkDeviceSize offset = 0)
    {
        return vkBindBufferMemory(device.native, buffer.native, memory.native, offset);
    }

    /// @brief Wraps vkMapMemory. Maps a host-visible allocation into application address space.
    /// @param device   The logical device the memory belongs to.
    /// @param memory   The memory to map (must be HOST_VISIBLE).
    /// @param offset   Byte offset into the allocation.
    /// @param size     Bytes to map (VK_WHOLE_SIZE for the rest of the allocation).
    /// @param flags    Reserved; pass 0.
    /// @param out_data Receives the pointer to the mapped region.
    /// @return The native VkResult.
    inline VkResult
    map_memory(device_t& device, device_memory_t& memory, VkDeviceSize offset, VkDeviceSize size,
               VkMemoryMapFlags flags, void** out_data)
    {
        return vkMapMemory(device.native, memory.native, offset, size, flags, out_data);
    }

    /// @brief Wraps vkUnmapMemory.
    /// @param device The logical device the memory belongs to.
    /// @param memory The memory to unmap.
    inline void
    unmap_memory(device_t& device, device_memory_t& memory)
    {
        vkUnmapMemory(device.native, memory.native);
    }

}
