#pragma once
#include <simplex/renderer/renderer_interface.hpp>
#include <simplex/renderer/vulkan/functions.hpp>
#include <simplex/renderer/vulkan/handles.hpp>
#include <simplex/renderer/vulkan/structures.hpp>

namespace spx::vk
{

    class vulkan_renderer : public spx::renderer_interface
    {

        public:
            inline  vulkan_renderer() = default;
            inline ~vulkan_renderer() = default;

        protected:
            void create_instance();
            void select_physical_device();
            void create_logical_device();
            void create_surface();

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

    };

}
