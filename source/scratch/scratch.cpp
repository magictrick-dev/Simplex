#include <scratch/scratch.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <utils/defs.hpp>
#include <utils/typeid.hpp>
#include <utils/linear.hpp>
#include <utils/system/resource_manager.hpp>
#include <utils/system/memory_alloc.hpp>
#include <utils/logging.hpp>

#include <simplex/array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/string_view.hpp>
#include <simplex/array_view.hpp>
#include <simplex/dynamic_string.hpp>
#include <simplex/static_string.hpp>
#include <simplex/static_queue.hpp>
#include <simplex/hashed_sparse_map.hpp>

#include <simplex/platform/window.hpp>
#include <simplex/platform/glfw_window.hpp>
#include <scratch/renderer/vulkan_renderer.hpp>

// Window setup.
static spx::vk::vulkan_renderer renderer { };
static spx::glfw_window glfw_window { };

int 
scratch_main()
{

    // Hijack the thread name for the logging manager.
    spx::logger::set_thread_name("SCRATCH");

    // Initialize the window.
    if (!glfw_window.create("Simplex Engine", 1280, 720))
    {
        spx::logger::dispatch_error_log("Failed to initialize window.");
        spx::logger::process_message_queue();
        return 0;
    }

    glfw_window.lock_resizing();
    glfw_window.show();

    if (renderer.initialize(&glfw_window) != RendererResultType_OK)
    {
        spx::logger::dispatch_error_log("Failed to properly initialize the vulkan renderer.");
        spx::logger::process_message_queue();
        glfw_window.destroy();
        return 0;
    }

    while (!glfw_window.should_close())
    {

        glfw_window.poll_events();
        spx::logger::process_message_queue();

    }

    renderer.deinitialize();
    glfw_window.destroy();

    spx::logger::process_message_queue(); // Process the remaining windows.
    return 0;

}