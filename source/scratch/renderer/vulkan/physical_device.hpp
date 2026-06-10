#pragma once
#include <vulkan/vulkan.h>

#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>


#include <scratch/renderer/vulkan/instance.hpp>
#include <scratch/renderer/vulkan/queue_family.hpp>

namespace spx::vk
{

    class physical_device
    {

        public:
            // Scoring constants for get_device_score(). Any negative score marks the
            // device as completely undesirable. The device type tiers are spaced an
            // order of magnitude apart so the soft bonuses (API version, memory in
            // MiB) can never promote a device past a more desirable type.
            static constexpr int64_t disqualified_score     = -1;
            static constexpr int64_t discrete_gpu_score     = 1'000'000'000;
            static constexpr int64_t integrated_gpu_score   = 100'000'000;
            static constexpr int64_t virtual_gpu_score      = 10'000'000;
            static constexpr int64_t cpu_device_score       = 1'000'000;
            static constexpr int64_t api_version_score      = 10'000;

        public:
            inline  physical_device() = default;
            inline ~physical_device() = default;

            inline 
            physical_device(VkInstance vulkan_instance, VkPhysicalDevice physical_device) 
                : device(physical_device)
            {


                // Gets the physical device properties.
                vkGetPhysicalDeviceProperties(this->device, &this->device_properties_1);
                vkGetPhysicalDeviceMemoryProperties(this->device, &this->memory_properties);

                this->driver_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
                this->device_properties_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                this->device_properties_2.pNext = &this->driver_properties;

                vkGetPhysicalDeviceProperties2(this->device, &this->device_properties_2);

                // Get the physical device features available.
                this->device_10_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                this->device_11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
                this->device_12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
                this->device_13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                this->device_14_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;

                // NOTE(Chris): Only chain the version-specific feature structs that the device
                //              actually supports. Querying a Vulkan1XFeatures struct on a device
                //              that reports a lower apiVersion is undefined behavior, so we build
                //              the pNext chain from the back, linking each struct only when its
                //              version is supported.
                void *features_chain = NULL;
                if (this->supports_version(VK_API_VERSION_1_4))
                {
                    this->device_14_features.pNext = features_chain;
                    features_chain = &this->device_14_features;
                }

                if (this->supports_version(VK_API_VERSION_1_3))
                {
                    this->device_13_features.pNext = features_chain;
                    features_chain = &this->device_13_features;
                }

                if (this->supports_version(VK_API_VERSION_1_2))
                {
                    this->device_12_features.pNext = features_chain;
                    features_chain = &this->device_12_features;
                }

                if (this->supports_version(VK_API_VERSION_1_1))
                {
                    this->device_11_features.pNext = features_chain;
                    features_chain = &this->device_11_features;
                }

                this->device_10_features.pNext = features_chain;
                vkGetPhysicalDeviceFeatures2(this->device, &this->device_10_features);

                // Gets the queue families and stores them in a neat structure.
                uint32_t queue_families_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(this->device, &queue_families_count, NULL);

                spx::dynamic_array<VkQueueFamilyProperties> queue_family_properties(queue_families_count);
                vkGetPhysicalDeviceQueueFamilyProperties(this->device, &queue_families_count, queue_family_properties.begin());

                for (size_t i = 0; i < queue_family_properties.size(); ++i)
                {
                    this->queue_families.emplace_back(i, queue_family_properties[i]);
                }

            }

            inline spx::string_view<char>
            get_device_name() const
            {
                return this->device_properties_1.deviceName;
            }

            inline uint32_t
            get_api_version() const
            {
                return this->device_properties_1.apiVersion;
            }

            inline bool
            supports_version(uint32_t version) const
            {

                // NOTE(Chris): apiVersion packs an optional variant in its high bits; standard
                //              Vulkan uses variant 0. Strip the variant from both sides so the
                //              comparison only weighs major/minor/patch. Callers pass the
                //              VK_API_VERSION_1_x constants directly.
                uint32_t device_version = VK_API_VERSION_VARIANT(this->device_properties_1.apiVersion) == VK_API_VERSION_VARIANT(version)
                    ? this->device_properties_1.apiVersion
                    : 0;

                return device_version >= version;

            }

            inline spx::string_view<char>
            get_driver_name() const
            {
                return this->driver_properties.driverName;
            }

            inline spx::string_view<char>
            get_driver_version() const
            {
                return this->driver_properties.driverInfo;
            }

            inline spx::dynamic_string<char>
            get_qualified_name() const
            {
                spx::dynamic_string<char> result;
                result += this->get_device_name();
                result += " ";
                result += this->get_driver_name();
                result += " ";
                result += this->get_driver_version();
                return std::move(result);
            }

            inline bool
            has_queue_family_with(VkQueueFlags flags) const
            {

                for (const auto &family : this->queue_families)
                {
                    if (family.has_flags(flags))
                    {
                        return true;
                    }
                }

                return false;

            }

            inline int64_t
            get_device_local_memory_size() const
            {

                // Heap 0 is not guaranteed to be the device-local heap, so sum every
                // heap flagged as device-local to get the real on-device memory budget.
                int64_t total_size = 0;
                for (uint32_t i = 0; i < this->memory_properties.memoryHeapCount; ++i)
                {
                    if (this->memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    {
                        total_size += this->memory_properties.memoryHeaps[i].size;
                    }
                }

                return total_size;

            }

            inline int64_t
            get_device_score() const
            {

                // Hard requirements come first; a device that fails any of them is
                // completely undesirable and reports a negative score so the front-end
                // never selects it. The renderer can not function without a graphics
                // queue, so that is the only hard requirement for now.
                if (!this->has_queue_family_with(VK_QUEUE_GRAPHICS_BIT))
                {
                    return physical_device::disqualified_score;
                }

                int64_t device_score = 0;

                // The device type is the dominant term; the tier constants are spaced
                // far enough apart that no amount of memory or version bonuses lets a
                // lower tier outrank a higher one.
                switch (this->device_properties_1.deviceType)
                {
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:      device_score += physical_device::discrete_gpu_score;    break;
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:    device_score += physical_device::integrated_gpu_score;  break;
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       device_score += physical_device::virtual_gpu_score;     break;
                    case VK_PHYSICAL_DEVICE_TYPE_CPU:               device_score += physical_device::cpu_device_score;      break;
                    default:                                                                                                break;
                }

                // Within a tier, prefer newer API support, then break remaining ties
                // with the device-local memory budget (scored in MiB so it stays well
                // below the gap between device type tiers).
                if (this->supports_version(VK_API_VERSION_1_1)) device_score += physical_device::api_version_score;
                if (this->supports_version(VK_API_VERSION_1_2)) device_score += physical_device::api_version_score;
                if (this->supports_version(VK_API_VERSION_1_3)) device_score += physical_device::api_version_score;
                if (this->supports_version(VK_API_VERSION_1_4)) device_score += physical_device::api_version_score;

                device_score += this->get_device_local_memory_size() / (1024 * 1024);

                return device_score;

            }

            inline operator VkPhysicalDevice() 
            { 
                return this->device; 
            }

            inline uint32_t
            get_queue_family_index_with(VkQueueFlags flags) const
            {

                for (const auto& family : this->queue_families)
                {
                    if (family.has_flags(flags))
                    {
                        return family.index;
                    }
                }

                throw std::runtime_error("Failed to find queue family with required flags.");
                return 0;

            }



        public:
            static inline spx::array_view<spx::vk::physical_device>
            get_physical_devices(spx::vk::instance instance)
            {

                static bool initialized = false;
                static spx::dynamic_array<spx::vk::physical_device> physical_devices;
                if (initialized == true) return physical_devices;
                initialized = true;

                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(instance, &device_count, NULL);

                spx::dynamic_array<VkPhysicalDevice> devices(device_count);
                vkEnumeratePhysicalDevices(instance, &device_count, devices.begin());
                for (auto device : devices)
                {
                    physical_devices.emplace_back(instance, device);
                }

                return physical_devices;

            }

            static inline spx::vk::physical_device
            get_optimal_device(spx::vk::instance instance)
            {

                auto physical_devices = spx::vk::physical_device::get_physical_devices(instance);

                spx::vk::physical_device optimal_device;
                int64_t maximum_score = -1;
                for (auto &device : physical_devices)
                {

                    const int64_t device_score = device.get_device_score();
                    spx::logger::dispatch_diagnostic_log("Physical device {} scored {}.", device.get_qualified_name().c_str(), device_score);

                    // Negative scores are completely undesirable devices and are never
                    // eligible for selection, regardless of what else is available.
                    if (device_score < 0) continue;

                    if (device_score > maximum_score)
                    {
                        maximum_score = device_score;
                        optimal_device = device;
                    }

                }

                if (optimal_device.device == NULL)
                {
                    throw std::runtime_error("Failed to find a suitable physical device.");
                }

                return optimal_device;

            }

        public:
            VkPhysicalDevice device = NULL;
            VkPhysicalDeviceProperties          device_properties_1 = {};
            VkPhysicalDeviceProperties2         device_properties_2 = {};
            VkPhysicalDeviceDriverProperties    driver_properties = {};
            VkPhysicalDeviceMemoryProperties    memory_properties = {};

            VkPhysicalDeviceFeatures2           device_10_features = {};
            VkPhysicalDeviceVulkan11Features    device_11_features = {};
            VkPhysicalDeviceVulkan12Features    device_12_features = {};
            VkPhysicalDeviceVulkan13Features    device_13_features = {};
            VkPhysicalDeviceVulkan14Features    device_14_features = {};

            spx::dynamic_array<spx::vk::queue_family> queue_families;

    };

}