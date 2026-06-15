#pragma once
#include <simplex/renderer/renderer_interface.hpp>
#include <simplex/renderer/vulkan/functions.hpp>
#include <simplex/renderer/vulkan/handles.hpp>
#include <simplex/renderer/vulkan/structures.hpp>

namespace spx::vk
{

    class vulkan_renderer : public spx::renderer_interface
    {

        static constexpr bool32_t enable_validation = true;

        public:
            inline  vulkan_renderer() = default;
            inline ~vulkan_renderer() = default;

        protected:
            void create_instance();
            void select_physical_device();
            void create_logical_device();
            void create_surface();

            /// @brief Debug messenger callback. Implemented separately so it can be wired into the
            ///        logging system; matches PFN_vkDebugUtilsMessengerCallbackEXT.
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

        private:
            spx::vk::instance_t instance;
            spx::vk::physical_device_t physical_device;
            spx::vk::debug_utils_messenger_t debug_messenger;

    };

}
