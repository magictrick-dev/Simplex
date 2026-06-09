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

#include <scratch/renderer/vulkan_renderer.hpp>

// Window setup.
static GLFWwindow *window;
static spx::vk::vulkan_renderer renderer;

static inline void 
init_window(spx::string_view<char> window_name, const int32_t width, const int32_t height)
{

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // Will address later.

    window = glfwCreateWindow(width, height, window_name.data(), NULL, NULL);

}

int 
scratch_main()
{

    // Hijack the thread name for the logging manager.
    spx::logger::set_thread_name("SCRATCH");

    init_window("Vulkan Scratch", 1280, 720);
    if (renderer.initialize(window) != EngineResultType_OK)
    {
        spx::logger::dispatch_error_log("Failed to properly initialize the vulkan renderer.");
        spx::logger::process_message_queue();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }

    spx::logger::dispatch_information_log("Initialized Vulkan scratch renderer.");

    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();
        spx::logger::process_message_queue();

    }

    renderer.deinitialize();
    spx::logger::dispatch_information_log("Deinitialized Vulkan scratch renderer.");
    spx::logger::process_message_queue();

    glfwDestroyWindow(window);
    glfwTerminate();

    spx::logger::dispatch_information_log("Scratch Vulkan renderer shutdown successfully.");
    spx::logger::process_message_queue();

    return 0;

}