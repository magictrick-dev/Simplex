#pragma once
#include <vulkan/vulkan.h>

#include <utils/defs.hpp>
#include <utils/logging.hpp>

#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>

#include <simplex/renderer/vulkan/instance.hpp>
#include <simplex/renderer/vulkan/surface.hpp>
#include <simplex/renderer/vulkan/queue_family.hpp>

namespace spx::vk
{

    class physical_device
    {

        public:
            static constexpr int64_t disqualified_score     = -1;
            static constexpr int64_t discrete_gpu_score     = 1'000'000'000;
            static constexpr int64_t integrated_gpu_score   = 100'000'000;
            static constexpr int64_t virtual_gpu_score      = 10'000'000;
            static constexpr int64_t cpu_device_score       = 1'000'000;
            static constexpr int64_t api_version_score      = 10'000;

        public:
            inline  physical_device() = default;
            inline ~physical_device() = default;

            physical_device(VkInstance vulkan_instance, 
                            VkSurfaceKHR surface, 
                            VkPhysicalDevice physical_device);

            uint32_t get_api_version() const;

            bool supports_version(uint32_t version) const;

            spx::string_view<char> get_device_name() const;
            spx::string_view<char> get_driver_name() const;
            spx::string_view<char> get_driver_version() const;
            spx::dynamic_string<char> get_qualified_name() const;

            bool has_queue_family_with(VkQueueFlags flags, bool require_presentation = false) const;

            int64_t get_device_local_memory_size() const;
            int64_t get_device_score() const;
            uint32_t get_queue_family_index_with(VkQueueFlags flags, bool require_presentation = false) const;

            inline operator VkPhysicalDevice() { return this->device; };

        public:
            static spx::array_view<spx::vk::physical_device> get_physical_devices(
                spx::vk::instance instance,
                spx::vk::surface surface);
            static spx::vk::physical_device get_optimal_device(
                spx::vk::instance instance,
                spx::vk::surface surface);

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