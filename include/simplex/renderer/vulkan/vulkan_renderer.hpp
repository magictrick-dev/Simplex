#pragma once
#include <simplex/renderer/renderer_interface.hpp>
#include <simplex/renderer/vulkan/instance.hpp>
#include <simplex/renderer/vulkan/physical_device.hpp>
#include <simplex/renderer/vulkan/logical_device.hpp>

namespace spx::vk
{

    class vulkan_renderer : public spx::renderer_interface
    {

        public:
            inline  vulkan_renderer() = default;
            inline ~vulkan_renderer() = default;

        protected:
            virtual RendererResultType internal_initialize() override;
            virtual RendererResultType internal_frame_begin() override;
            virtual RendererResultType internal_render_begin() override;
            virtual RendererResultType internal_render_end() override;
            virtual RendererResultType internal_frame_end() override;
            virtual RendererResultType internal_deinitialize() override;

        private:
            spx::vk::instance instance;
            spx::vk::physical_device physical_device;
            spx::vk::logical_device logical_device;

    };

}
