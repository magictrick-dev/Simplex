#include <scratch/scratch.hpp>

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
//#include <simplex/renderer/vulkan/vulkan_renderer.hpp>
#include <simplex/renderer/vulkan/structures.hpp>
#include <simplex/renderer/vulkan/handles.hpp>

//static spx::vk::vulkan_renderer renderer { };

#if defined(_WIN32)
#   include <simplex/platform/win32/win32_window.hpp>
    static spx::win32_window window { };
#elif defined(__APPLE__)
#   include <simplex/platform/darwin/darwin_window.hpp>
    static spx::darwin_window window { };
#else
#   include <simplex/platform/glfw_window.hpp>
    static spx::glfw_window window { };
#endif

int 
scratch_main()
{

    spx::vk::instance_create_info create_info { };
    spx::logger::dispatch_debug_log("Sizeof spx instance create info: {}", sizeof(create_info));
    spx::logger::dispatch_debug_log("Sizeof vk  instance create info: {}", sizeof(VkInstanceCreateInfo));

    auto list = spx::vk::instance::get_available_instance_layers();
    for (auto l : list)
    {
        spx::logger::dispatch_debug_log("Instance layer available is: {}", l.c_str());
    }

#if 0

    // Hijack the thread name for the logging manager.
    spx::logger::set_thread_name("SCRATCH");

    // Initialize the window.
    if (!window.create("Simplex Engine", 1280, 720))
    {
        spx::logger::dispatch_error_log("Failed to initialize window.");
        spx::logger::process_message_queue();
        return 0;
    }

    window.lock_resizing();
    window.show();

    if (renderer.initialize(&window) != RendererResultType_OK)
    {
        spx::logger::dispatch_error_log("Failed to properly initialize the vulkan renderer.");
        spx::logger::process_message_queue();
        window.destroy();
        return 0;
    }

    while (!window.should_close())
    {

        window.poll_events();
        spx::logger::process_message_queue();

    }

    renderer.deinitialize();
    window.destroy();
#endif

    spx::logger::process_message_queue(); // Process the remaining windows.
    return 0;

}