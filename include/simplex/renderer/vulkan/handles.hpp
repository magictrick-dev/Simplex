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
    template <typename derived_t, typename native_t> 
    struct vk_handle_ext 
    { 
        inline  vk_handle_ext() = default;
        inline  vk_handle_ext(native_t handle) { }
        inline ~vk_handle_ext() = default;
    };

    // Forward declarations of the per-handle specializations. These must be visible before
    // vk_handle (below) names vk_handle_ext<vk_handle<T>, T> as its base, otherwise an
    // instantiation of vk_handle<T> can bind to the primary template above before the matching
    // specialization is seen -- which silently drops every member the specialization adds.
    template <typename derived_t> struct vk_handle_ext<derived_t, VkInstance>;
    template <typename derived_t> struct vk_handle_ext<derived_t, VkPhysicalDevice>;
    template <typename derived_t> struct vk_handle_ext<derived_t, VkSwapchainKHR>;

    /// @brief Wraps vulkan handles up.
    /// @tparam handle_type_t The handle type.
    ///
    /// We are wrapping vulkan handles with potential for mix-in helpers that can
    /// help shortcut some of the annoying boilerplate code that vulkan has.
    template <typename handle_type_t>
    struct vk_handle : vk_handle_ext<vk_handle<handle_type_t>, handle_type_t>
    {

        inline  vk_handle() = default;
        inline ~vk_handle() = default;

        inline vk_handle(handle_type_t handle)
            : vk_handle_ext<vk_handle, handle_type_t>(handle), native(handle)
        { };

        using native_type_t     = handle_type_t;
        native_type_t native    = { nullptr };

        inline operator native_type_t&()                { return this->native; }
        inline operator native_type_t const &() const   { return this->native; }

    };

    // ---------------------------------------------------------------------------------------------
    // Using statements.
    // ---------------------------------------------------------------------------------------------

    using instance_t                = vk_handle<VkInstance>;
    using physical_device_t         = vk_handle<VkPhysicalDevice>;
    using device_t                  = vk_handle<VkDevice>;
    using debug_utils_messenger_t   = vk_handle<VkDebugUtilsMessengerEXT>;
    using surface_t                 = vk_handle<VkSurfaceKHR>;
    using queue_t                   = vk_handle<VkQueue>;
    using swapchain_t               = vk_handle<VkSwapchainKHR>;
    using image_t                   = vk_handle<VkImage>;
    using image_view_t              = vk_handle<VkImageView>;
    using shader_module_t           = vk_handle<VkShaderModule>;
    using pipeline_layout_t         = vk_handle<VkPipelineLayout>;
    using pipeline_t                = vk_handle<VkPipeline>;
    using command_pool_t            = vk_handle<VkCommandPool>;
    using command_buffer_t          = vk_handle<VkCommandBuffer>;
    using semaphore_t               = vk_handle<VkSemaphore>;
    using fence_t                   = vk_handle<VkFence>;
    using buffer_t                  = vk_handle<VkBuffer>;
    using device_memory_t           = vk_handle<VkDeviceMemory>;

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

        /// @brief Returns the highest Vulkan version supported by the loader/instance, packed with
        ///        the VK_API_VERSION_* layout. Used to decide how far the physical-device property
        ///        and feature pNext chains can be extended.
        /// @return The supported instance version, or VK_API_VERSION_1_0 if the query is unavailable.
        static inline uint32_t
        get_instance_version()
        {
            uint32_t version = VK_API_VERSION_1_0;
            vkEnumerateInstanceVersion(&version);
            return version;
        }

    };

    /// @brief VkPhysicalDevice mixin.
    ///
    /// Lazily queries and caches the device's properties and features on first access. The pNext
    /// chains for the version-specific 1.1/1.2/1.3/1.4 structs are extended only as far as the
    /// effective Vulkan version supports -- the smaller of the loader/instance version
    /// (instance_t::get_instance_version) and the device's own reported apiVersion -- so we never
    /// chain a struct the instance or device would reject.
    template <typename derived_t>
    struct vk_handle_ext<derived_t, VkPhysicalDevice>
    {

        inline  vk_handle_ext() = default;
        inline ~vk_handle_ext() = default;

        // Eagerly fill the cached property/feature data from the handle. We use the parameter
        // directly rather than reading it back out of the derived object: this runs during the
        // derived's base-construction, before its own members (including native) are initialized.
        inline vk_handle_ext(VkPhysicalDevice handle)
        {
            this->populate(handle);
        }

        /// @brief The marketing device name (VkPhysicalDeviceProperties::deviceName).
        inline spx::string_view<char>
        get_device_name()
        {
            return this->device_10_properties.get_properties().get_device_name();
        }

        /// @brief A human-readable vendor name resolved from the PCI vendorID. Vulkan reports no
        ///        vendor string directly, so this maps the well-known IDs.
        inline spx::string_view<char>
        get_device_vendor()
        {
            switch (this->device_10_properties.get_properties().get_vendor_id())
            {
                case 0x1002: return "AMD";
                case 0x1010: return "ImgTec";
                case 0x106B: return "Apple";
                case 0x10DE: return "NVIDIA";
                case 0x13B5: return "ARM";
                case 0x5143: return "Qualcomm";
                case 0x8086: return "Intel";
                default:     return "Unknown";
            }
        }

        /// @brief The device's reported conformance version (from the Vulkan 1.2 properties). Zeroed
        ///        on devices/instances below Vulkan 1.2, since that struct is not chained there.
        inline spx::vk::conformance_version_t
        get_device_api_support()
        {
            return this->device_12_properties.conformanceVersion;
        }

        /// @brief The device's reported Vulkan API version (from the base properties), packed in the
        ///        VK_API_VERSION_* layout. This is what the scoring routine ranks on first.
        inline uint32_t
        get_device_api_version()
        {
            return this->device_10_properties.get_properties().get_api_version();
        }

        /// @brief The driver name reported by the device (from the Vulkan 1.2 properties), e.g.
        ///        "MoltenVK" or "Kosmickrisp". Useful for telling apart multiple ICDs that expose the
        ///        same physical GPU under the same device name. Empty on devices below Vulkan 1.2.
        inline spx::string_view<char>
        get_device_driver_name()
        {
            return spx::string_view<char>(this->device_12_properties.driverName);
        }

        /// @brief Total device-local (VRAM) memory in bytes, summed across all DEVICE_LOCAL heaps.
        inline VkDeviceSize
        get_device_local_memory()
        {
            return this->device_local_memory;
        }

        /// @brief The device's queue families, pre-fetched and cached at construction. Index into the
        ///        returned array to read a family's properties (queue_family_properties_2_t exposes
        ///        the per-family data through get_queue_family_properties()); the array index is the
        ///        queue family index used elsewhere in the API (e.g. supports_presentation).
        inline const spx::dynamic_array<spx::vk::queue_family_properties_2_t>&
        get_queue_families() const
        {
            return this->queue_families;
        }

        /// @brief Whether the device advertises the given device extension. Queried live (not cached)
        ///        since it is only needed a handful of times at device-creation. Used, for example, to
        ///        decide whether VK_KHR_portability_subset must be enabled.
        /// @param extension_name The extension to look for.
        /// @return True if the device supports the extension, false otherwise.
        inline bool32_t
        supports_extension(spx::string_view<char> extension_name) const
        {
            VkPhysicalDevice handle = static_cast<const derived_t*>(this)->native;

            uint32_t extension_count = 0;
            vkEnumerateDeviceExtensionProperties(handle, nullptr, &extension_count, nullptr);

            spx::dynamic_array<VkExtensionProperties> extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(handle, nullptr, &extension_count, extensions.begin());

            for (const auto& extension : extensions)
            {
                if (extension_name == spx::string_view<char>(extension.extensionName)) return true;
            }

            return false;
        }

        /// @brief Picks a memory type index satisfying a buffer/image's requirements and the desired
        ///        memory properties. The type_filter is the memoryTypeBits from a resource's
        ///        VkMemoryRequirements (bit i set means memory type i is allowed); properties is the
        ///        VkMemoryPropertyFlags the chosen type must contain (e.g. DEVICE_LOCAL for a GPU-side
        ///        buffer, or HOST_VISIBLE | HOST_COHERENT for a CPU-writable staging buffer). Queried
        ///        live since it is only needed at allocation time.
        /// @param type_filter The allowed memory types (a resource's memoryTypeBits).
        /// @param properties  The memory property flags the chosen type must include.
        /// @return The matching memory type index, or (uint32_t)-1 if none qualifies.
        inline uint32_t
        find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const
        {
            VkPhysicalDevice handle = static_cast<const derived_t*>(this)->native;

            VkPhysicalDeviceMemoryProperties memory_properties { };
            vkGetPhysicalDeviceMemoryProperties(handle, &memory_properties);

            for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
            {
                if ((type_filter & (1u << i)) &&
                    (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                    return i;
            }

            return (uint32_t)-1;
        }

        /// @brief Checks that every base (Vulkan 1.0) feature requested in `features` is supported by
        ///        this device. A feature is "requested" when its VkBool32 member is VK_TRUE; the call
        ///        passes only if every requested member is also VK_TRUE in the device's cached feature
        ///        set (the request must be a subset of what's available). The actual field-by-field
        ///        comparison is owned by the feature struct itself (physical_device_10_features_t::
        ///        is_subset); this just supplies the device's cached features as the available set.
        /// @param features The requested feature set to validate.
        /// @return True if the device supports all requested features.
        inline bool32_t
        validate_10_features(const spx::vk::physical_device_10_features_t& features) const
        {
            return spx::vk::physical_device_10_features_t::is_subset(features, this->device_10_features);
        }

        /// @brief Vulkan 1.1 counterpart of validate_10_features. Fast-exits false when the device's
        ///        effective Vulkan version predates 1.1, since the struct's features can't exist --
        ///        and the cached struct was never queried -- in that case.
        inline bool32_t
        validate_11_features(const spx::vk::physical_device_11_features_t& features) const
        {
            if (this->effective_version < VK_API_VERSION_1_1) return false;
            return spx::vk::physical_device_11_features_t::is_subset(features, this->device_11_features);
        }

        /// @brief Vulkan 1.2 counterpart of validate_10_features. Fast-exits false below Vulkan 1.2.
        inline bool32_t
        validate_12_features(const spx::vk::physical_device_12_features_t& features) const
        {
            if (this->effective_version < VK_API_VERSION_1_2) return false;
            return spx::vk::physical_device_12_features_t::is_subset(features, this->device_12_features);
        }

        /// @brief Vulkan 1.3 counterpart of validate_10_features. Fast-exits false below Vulkan 1.3.
        inline bool32_t
        validate_13_features(const spx::vk::physical_device_13_features_t& features) const
        {
            if (this->effective_version < VK_API_VERSION_1_3) return false;
            return spx::vk::physical_device_13_features_t::is_subset(features, this->device_13_features);
        }

        /// @brief Vulkan 1.4 counterpart of validate_10_features. Fast-exits false below Vulkan 1.4.
        inline bool32_t
        validate_14_features(const spx::vk::physical_device_14_features_t& features) const
        {
            if (this->effective_version < VK_API_VERSION_1_4) return false;
            return spx::vk::physical_device_14_features_t::is_subset(features, this->device_14_features);
        }

        /// @brief A coarse desirability score for this device. Higher is better. Tiered priority:
        ///        supported Vulkan version first, then device class (discrete > integrated >
        ///        virtual > CPU), then device-local memory (in MB) as the final tiebreaker. The tier
        ///        weights are spaced so a higher tier always outranks everything below it. Sufficient
        ///        for picking a sensible default GPU.
        inline uint64_t
        get_device_score()
        {
            const auto& properties = this->device_10_properties.get_properties();

            uint32_t api = properties.get_api_version();
            uint64_t version_rank = (uint64_t)VK_API_VERSION_MAJOR(api) * 1000 + VK_API_VERSION_MINOR(api);

            // Prefer real GPUs; discrete over integrated, then virtual/CPU fallbacks.
            uint64_t type_rank = 0;
            switch (properties.get_device_type())
            {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   type_rank = 4; break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: type_rank = 3; break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    type_rank = 2; break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:            type_rank = 1; break;
                default:                                     type_rank = 0; break;
            }

            // Tier weights: version (1e12) > device class (1e7) > VRAM in MB (< ~1e6 in practice),
            // so each tier dominates the ones beneath it.
            uint64_t score = 0;
            score += version_rank * 1000000000000ull;
            score += type_rank    * 10000000ull;
            score += this->device_local_memory / (1024ull * 1024ull);
            return score;
        }

        /// @brief Enumerates the physical devices visible to the given instance.
        /// @param instance The instance to enumerate against.
        /// @return The available physical devices.
        static inline spx::dynamic_array<vk_handle<VkPhysicalDevice>>
        get_physical_devices(const instance_t& instance)
        {

            uint32_t device_count = 0;
            vkEnumeratePhysicalDevices(instance.native, &device_count, NULL);

            spx::dynamic_array<VkPhysicalDevice> handles(device_count);
            vkEnumeratePhysicalDevices(instance.native, &device_count, handles.begin());

            spx::dynamic_array<vk_handle<VkPhysicalDevice>> devices;
            for (VkPhysicalDevice handle : handles)
            {
                devices.emplace_back(handle);
            }

            return std::move(devices);

        }

        /// @brief Picks the highest-scoring physical device visible to the instance (see
        ///        get_device_score). Note this only ranks devices -- it does not check queue,
        ///        surface, or extension suitability, which the caller should validate separately.
        /// @param instance The instance to enumerate against.
        /// @return The optimal device, or a null handle (native == nullptr) if none are available.
        static inline vk_handle<VkPhysicalDevice>
        select_optimal_device(const instance_t& instance)
        {

            auto devices = physical_device_t::get_physical_devices(instance);

            vk_handle<VkPhysicalDevice> best { };
            uint64_t best_score = 0;
            bool32_t found = false;

            for (auto& device : devices)
            {
                uint64_t score = device.get_device_score();
                if (found == false || score > best_score)
                {
                    best        = device;
                    best_score  = score;
                    found       = true;
                }
            }

            return best;

        }

        private:

            spx::vk::physical_device_10_features_t device_10_features { };
            spx::vk::physical_device_11_features_t device_11_features { };
            spx::vk::physical_device_12_features_t device_12_features { };
            spx::vk::physical_device_13_features_t device_13_features { };
            spx::vk::physical_device_14_features_t device_14_features { };

            spx::vk::physical_device_10_properties_t device_10_properties { };
            spx::vk::physical_device_11_properties_t device_11_properties { };
            spx::vk::physical_device_12_properties_t device_12_properties { };
            spx::vk::physical_device_13_properties_t device_13_properties { };
            spx::vk::physical_device_14_properties_t device_14_properties { };

            spx::dynamic_array<spx::vk::queue_family_properties_2_t> queue_families { };

            VkDeviceSize device_local_memory { 0 };

            // Effective Vulkan version = min(instance, device), cached from populate(). Bounds how far
            // the version-specific feature/property chains were actually queried.
            uint32_t effective_version { 0 };

            /// @brief Populates the cached property/feature structs from the given handle, chaining
            ///        the version-specific structs up to the effective supported version, then
            ///        severs the chains so the cached data is self-contained and copy/move-safe.
            inline void
            populate(VkPhysicalDevice handle)
            {

                // Effective version = min(instance-supported, device-supported). The device's own
                // apiVersion comes from the always-available v1 properties query.
                VkPhysicalDeviceProperties base_properties { };
                vkGetPhysicalDeviceProperties(handle, &base_properties);

                uint32_t instance_version = instance_t::get_instance_version();
                uint32_t device_version   = base_properties.apiVersion;
                uint32_t version = (instance_version < device_version) ? instance_version : device_version;

                // Remember the effective version: the feature chains below are only populated up to
                // this point, so the feature validators must gate on it (not the raw device version)
                // to avoid validating against a struct the driver never filled.
                this->effective_version = version;

                // Build the properties chain (a wrapper's address is its native struct's address).
                void* properties_chain = nullptr;
                if (version >= VK_API_VERSION_1_4) { this->device_14_properties.pNext = properties_chain; properties_chain = &this->device_14_properties; }
                if (version >= VK_API_VERSION_1_3) { this->device_13_properties.pNext = properties_chain; properties_chain = &this->device_13_properties; }
                if (version >= VK_API_VERSION_1_2) { this->device_12_properties.pNext = properties_chain; properties_chain = &this->device_12_properties; }
                if (version >= VK_API_VERSION_1_1) { this->device_11_properties.pNext = properties_chain; properties_chain = &this->device_11_properties; }
                this->device_10_properties.pNext = properties_chain;

                VkPhysicalDeviceProperties2& native_properties = this->device_10_properties;
                vkGetPhysicalDeviceProperties2(handle, &native_properties);

                // Build the matching features chain.
                void* features_chain = nullptr;
                if (version >= VK_API_VERSION_1_4) { this->device_14_features.pNext = features_chain; features_chain = &this->device_14_features; }
                if (version >= VK_API_VERSION_1_3) { this->device_13_features.pNext = features_chain; features_chain = &this->device_13_features; }
                if (version >= VK_API_VERSION_1_2) { this->device_12_features.pNext = features_chain; features_chain = &this->device_12_features; }
                if (version >= VK_API_VERSION_1_1) { this->device_11_features.pNext = features_chain; features_chain = &this->device_11_features; }
                this->device_10_features.pNext = features_chain;

                VkPhysicalDeviceFeatures2& native_features = this->device_10_features;
                vkGetPhysicalDeviceFeatures2(handle, &native_features);

                // Total device-local memory, summed across all DEVICE_LOCAL heaps. Used by the
                // scoring routine (and generally the headline "how much VRAM" figure).
                VkPhysicalDeviceMemoryProperties memory_properties { };
                vkGetPhysicalDeviceMemoryProperties(handle, &memory_properties);

                this->device_local_memory = 0;
                for (uint32_t i = 0; i < memory_properties.memoryHeapCount; ++i)
                {
                    if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                        this->device_local_memory += memory_properties.memoryHeaps[i].size;
                }

                // Pre-fetch and cache the queue families via the two-call pattern. The _2 query
                // requires each element's sType pre-set, which the wrapper's default ctor handles;
                // pNext stays null (no chained structs) so the cached array carries no self-
                // referential pointers and stays copy/move-safe like the structs above.
                uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties2(handle, &queue_family_count, nullptr);

                this->queue_families = spx::dynamic_array<spx::vk::queue_family_properties_2_t>(queue_family_count);
                VkQueueFamilyProperties2* native_queue_families = queue_family_count
                    ? &static_cast<VkQueueFamilyProperties2&>(this->queue_families[0]) : nullptr;
                vkGetPhysicalDeviceQueueFamilyProperties2(handle, &queue_family_count, native_queue_families);

                // The pNext links were only needed for the two queries above. Now that the driver
                // has filled the structs, sever them: the cached members become plain value data
                // with no self-referential pointers, so copying/moving this wrapper (e.g. when the
                // enumerating dynamic_array reallocates) can't leave a dangling chain behind.
                this->device_10_properties.pNext = nullptr;
                this->device_11_properties.pNext = nullptr;
                this->device_12_properties.pNext = nullptr;
                this->device_13_properties.pNext = nullptr;
                this->device_14_properties.pNext = nullptr;

                this->device_10_features.pNext = nullptr;
                this->device_11_features.pNext = nullptr;
                this->device_12_features.pNext = nullptr;
                this->device_13_features.pNext = nullptr;
                this->device_14_features.pNext = nullptr;

            }

    };

    /// @brief VkSwapchainKHR mixin.
    ///
    /// Hosts the selection helpers used to turn a surface's reported capabilities/formats/present
    /// modes into the concrete parameters a swapchain is created with, plus a helper to read back the
    /// images the swapchain owns. The helpers are static (they operate on query results, not on a
    /// live swapchain); get_swapchain_images is the only instance method and reads this swapchain's
    /// own handle.
    template <typename derived_t>
    struct vk_handle_ext<derived_t, VkSwapchainKHR>
    {

        inline  vk_handle_ext() = default;
        inline  vk_handle_ext(VkSwapchainKHR handle) { }
        inline ~vk_handle_ext() = default;

        /// @brief Picks a surface format, preferring 32-bit BGRA in the sRGB-nonlinear color space
        ///        (the standard choice for a gamma-correct desktop swapchain). Falls back to the
        ///        first reported format, which the spec guarantees exists when the list is non-empty.
        /// @param available The formats reported by get_physical_device_surface_formats.
        /// @return The chosen format.
        static inline spx::vk::surface_format_t
        choose_surface_format(spx::array_view<spx::vk::surface_format_t> available)
        {
            for (const auto& format : available)
            {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return format;
            }

            return available.empty() ? spx::vk::surface_format_t { } : available.front();
        }

        /// @brief Picks a present mode, preferring mailbox (low-latency triple buffering) when offered.
        ///        Falls back to FIFO, which the spec requires every implementation to support.
        /// @param available The present modes reported by get_physical_device_surface_present_modes.
        /// @return The chosen present mode.
        static inline VkPresentModeKHR
        choose_present_mode(spx::array_view<VkPresentModeKHR> available)
        {
            for (VkPresentModeKHR mode : available)
            {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        /// @brief Resolves the swapchain extent. When the surface reports a fixed currentExtent we are
        ///        required to match it; the 0xFFFFFFFF sentinel means we are free to choose, in which
        ///        case the window's framebuffer size is used, clamped to the surface's min/max extent.
        /// @param capabilities       The surface capabilities.
        /// @param framebuffer_width  The window framebuffer width (used only in the sentinel case).
        /// @param framebuffer_height The window framebuffer height (used only in the sentinel case).
        /// @return The extent to create the swapchain with.
        static inline spx::vk::extent_2d_t
        choose_extent(const spx::vk::surface_capabilities_t& capabilities,
                      uint32_t framebuffer_width, uint32_t framebuffer_height)
        {
            if (capabilities.currentExtent.width != 0xFFFFFFFFu)
                return capabilities.currentExtent;

            const uint32_t min_w = capabilities.minImageExtent.width;
            const uint32_t max_w = capabilities.maxImageExtent.width;
            const uint32_t min_h = capabilities.minImageExtent.height;
            const uint32_t max_h = capabilities.maxImageExtent.height;

            spx::vk::extent_2d_t extent { };
            extent.width  = framebuffer_width  < min_w ? min_w : (framebuffer_width  > max_w ? max_w : framebuffer_width);
            extent.height = framebuffer_height < min_h ? min_h : (framebuffer_height > max_h ? max_h : framebuffer_height);
            return extent;
        }

        /// @brief Chooses how many images to request: one more than the surface minimum (so the
        ///        application is not perpetually waiting on the driver), clamped to the maximum when
        ///        the surface reports one (maxImageCount == 0 means "no limit").
        /// @param capabilities The surface capabilities.
        /// @return The image count to request.
        static inline uint32_t
        choose_image_count(const spx::vk::surface_capabilities_t& capabilities)
        {
            uint32_t count = capabilities.minImageCount + 1;
            if (capabilities.maxImageCount > 0 && count > capabilities.maxImageCount)
                count = capabilities.maxImageCount;
            return count;
        }

        /// @brief Reads back the images this swapchain owns. The images are owned by the swapchain
        ///        (not the application) and must not be destroyed individually; they are wrapped only
        ///        so views can be created over them. Follows the native two-call count/fill pattern.
        /// @param device The logical device the swapchain belongs to.
        /// @return The swapchain's images wrapped as handles.
        inline spx::dynamic_array<image_t>
        get_swapchain_images(device_t& device) const
        {
            VkSwapchainKHR handle = static_cast<const derived_t*>(this)->native;

            uint32_t image_count = 0;
            vkGetSwapchainImagesKHR(device.native, handle, &image_count, nullptr);

            spx::dynamic_array<VkImage> raw_images(image_count);
            vkGetSwapchainImagesKHR(device.native, handle, &image_count, raw_images.begin());

            spx::dynamic_array<image_t> images;
            for (VkImage image : raw_images)
            {
                images.emplace_back(image);
            }

            return std::move(images);
        }

    };

}