#pragma once
#include <simplex/renderer/renderer_interface.hpp>
#include <simplex/renderer/vulkan/functions.hpp>
#include <simplex/renderer/vulkan/handles.hpp>
#include <simplex/renderer/vulkan/structures.hpp>
#include <simplex/dynamic_array.hpp>

namespace spx::vk
{

    class vulkan_renderer : public spx::renderer_interface
    {

        static constexpr bool32_t enable_validation = true;
        static constexpr uint32_t invalid_queue_family = (uint32_t)-1;

        public:
            inline  vulkan_renderer() = default;
            inline ~vulkan_renderer() = default;

        protected:
            bool32_t create_instance();
            bool32_t select_physical_device();
            bool32_t create_logical_device();
            bool32_t create_surface();
            bool32_t create_swapchain();

            // Rebuilds the swapchain (and its image views) against the window's current size, reusing
            // create_swapchain after tearing the old resources down. Used to service window resizes.
            bool32_t recreate_swapchain();

            // Destroys the image views and the swapchain. The swapchain images themselves are owned by
            // the swapchain and are not destroyed individually -- only the wrapper array is cleared.
            void     destroy_swapchain();

        protected:
            static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
                VkDebugUtilsMessageTypeFlagsEXT             message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void*                                       user_data);

        protected:
            virtual RendererResultType internal_initialize() override;
            virtual RendererResultType internal_frame_begin() override;
            virtual RendererResultType internal_render_begin() override;
            virtual RendererResultType internal_render_end() override;
            virtual RendererResultType internal_frame_end() override;
            virtual RendererResultType internal_deinitialize() override;
            virtual RendererResultType internal_resize() override;

        private:
            spx::vk::instance_t instance;
            spx::vk::physical_device_t physical_device;
            spx::vk::device_t device;
            spx::vk::debug_utils_messenger_t debug_messenger;
            spx::vk::surface_t surface;

            spx::vk::queue_t graphics_queue;
            spx::vk::queue_t present_queue;

            uint32_t graphics_queue_family_index = invalid_queue_family;
            uint32_t present_queue_family_index  = invalid_queue_family;

            spx::vk::swapchain_t swapchain;
            spx::dynamic_array<spx::vk::image_t>      swapchain_images;
            spx::dynamic_array<spx::vk::image_view_t> swapchain_image_views;
            VkFormat                                  swapchain_format = VK_FORMAT_UNDEFINED;
            spx::vk::extent_2d_t                      swapchain_extent;

    };

}
