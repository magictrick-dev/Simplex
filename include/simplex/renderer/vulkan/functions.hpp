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
    /// reinterpret_cast (the layout guards in structures.hpp enforce that this is valid), so
    /// nothing is copied and these compile down to the exact same call you would have written
    /// by hand. Routing every call through here is what lets the wrappers drop the address-of
    /// (operator&) overload, since the conversion happens explicitly inside the wrapper.

    /// @brief Wraps vkCreateInstance, taking a wrapped create-info and writing into a wrapped handle.
    /// @param create_info  The instance create info.
    /// @param allocator    Optional allocation callbacks (may be nullptr).
    /// @param out_instance The handle to populate on success.
    /// @return The native VkResult from vkCreateInstance.
    inline VkResult
    create_instance(const instance_create_info& create_info,
                    const VkAllocationCallbacks* allocator,
                    instance& out_instance)
    {
        const VkInstanceCreateInfo& native_create_info = create_info;
        return vkCreateInstance(&native_create_info, allocator, &out_instance.native);
    }

    /// @brief Wraps vkDestroyInstance, taking a wrapped instance handle.
    /// @param inst      The instance to destroy.
    /// @param allocator Optional allocation callbacks (may be nullptr). Must match what was passed to create_instance.
    inline void
    destroy_instance(instance& inst, const VkAllocationCallbacks* allocator = nullptr)
    {
        vkDestroyInstance(inst.native, allocator);
        inst.native = nullptr;
    }

}
