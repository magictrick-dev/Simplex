#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <utils/defs.hpp>
#include <cli/cli.hpp>
#include <utils/test_registry.hpp>
#include <utils/system/resource_manager.hpp>
#include <parsers/rdview/rdview_parser.hpp>
#include <parsers/rdview/rdview_ref_visitor.hpp>

#ifndef SIMPLEX_PLATFORM_INFORMATION
#   define SIMPLEX_PLATFORM_INFORMATION
#   if defined(__APPLE__)
#       define SIMPLEX_PLATFORM_TYPE "Apple MacOSX"
#       define SIMPLEX_FRONTEND_RENDERER "OpenGL"
#       define SIMPLEX_BACKEND_RENDERER "RenderView"
#   elif defined(_WIN32)
#       define SIMPLEX_PLATFORM_TYPE "Microsoft Windows Win32"
#       define SIMPLEX_FRONTEND_RENDERER "OpenGL"
#       define SIMPLEX_BACKEND_RENDERER "RenderView"
#   elif defined(__unix__)
#       define SIMPLEX_PLATFORM_TYPE "Linux UNIX"
#       define SIMPLEX_FRONTEND_RENDERER "OpenGL"
#       define SIMPLEX_BACKEND_RENDERER "RenderView"
#   else
#       define SIMPLEX_PLATFORM_TYPE "Unknown"
#       define SIMPLEX_FRONTEND_RENDERER "Unavailable"
#       define SIMPLEX_BACKEND_RENDERER "Unavailable"
#   endif
#endif

static inline void
print_engine_information()
{
    printf("Simplex Rendering Engine - Version 0.0A - 2026 Christopher DeJong - MagicTrick-Dev\n");
    printf("    - Platform              : %s\n", SIMPLEX_PLATFORM_TYPE);
    printf("    - Frontend Renderer     : %s\n", SIMPLEX_FRONTEND_RENDERER);
    printf("    - Backend Renderer      : %s\n", SIMPLEX_BACKEND_RENDERER);
}

static int
entry(int argc, char **argv)
{

    print_engine_information();

    CLIParser cli(argc, argv);
    cli.add_argument_rule("--run-tests", {}, "Runs the full test suite.");
    cli.add_positional_rule(1, CLIValueType::Path, "Input scene description file (.rd).");

    try
    {
        cli.parse();
    }
    catch (const CLIParseException &e)
    {
        fprintf(stderr, "CLI error: %s\n", e.what());
        cli.print_help();
        return 1;
    }

    // NOTE(Chris): Running tests will bypass the runtime.
    if (cli.has_argument("--run-tests")) 
    {
        TestRegistry::RunEverything();
        return 0;
    }

    // Prepare the RDView file and load it in memory.
    std::filesystem::path file_path = std::filesystem::weakly_canonical(cli.get_arg(1));
    ResourceManager &resource_manager = ResourceManager::Get();
    
    ResourceHandle rdview_resource_handle = {};
    if (resource_manager.register_text_file_resource(file_path, &rdview_resource_handle) != ResourceManagerResult_OK)
    {
        std::cout << "Failed to find RDView script file: " << file_path << std::endl;
        return -1;
    }

    resource_manager.load(rdview_resource_handle);
    resource_manager.wait(rdview_resource_handle);
    
    ResourceView rdview_resource_view = {};
    auto result = resource_manager.fetch_resource_view(rdview_resource_handle, &rdview_resource_view);
    if (result != ResourceManagerResult_OK || rdview_resource_view.type != ResourceViewType_Text)
    {
        std::cout << "Resource type doesn't match expected resource view." << std::endl;
        return -1;
    }

    const char *file_source = rdview_resource_view.text_view.text;

    RDViewParser parser(file_source, file_path);
    if (!parser.match_everything())
    {
        std::cout << "Failed to fully parser: " << file_path << std::endl;
    }
    else
    {
        std::cout << "Parse completed for: " << file_path << std::endl;
        RDViewReferenceVisitor reference_output;
        reference_output.accept(parser.get_root());
        std::cout << "REFERENCE:" << std::endl;
        std::cout << reference_output.get_output() << std::endl;
    }

    resource_manager.unload(rdview_resource_handle);
    resource_manager.wait(rdview_resource_handle);
    resource_manager.remove(rdview_resource_handle);

    return 0;

}

#if defined(__APPLE__) && defined(__MACH__)
#   include <GLAD/glad.h>
#   include <GLFW/glfw3.h>

    int 
    main(int argc, char **argv)
    {
        return entry(argc, argv);
    }

#endif

#if defined(__unix__)

    int 
    main(int argc, char **argv)
    {
        return entry(argc, argv);
    }

#endif

#if defined(_WIN32)
#   include <windows.h>
#   include <conio.h>

    static inline void
    construct_cli_arguments(int *input_argc, char ***input_argv)
    {

        int argc = 0;
        LPWSTR *wide_argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        char **argv = (char**)malloc(sizeof(const char*)*argc);
        for (int i = 0; i < argc; ++i)
        {

            int required_size = WideCharToMultiByte(CP_ACP, 0, wide_argv[i], -1, NULL, 0, NULL, NULL);
            char *buffer = (char*)malloc(required_size);
            WideCharToMultiByte(CP_ACP, 0, wide_argv[i], -1, buffer, required_size, NULL, NULL);
            argv[i] = buffer;
            
        }

        *input_argc = argc;
        *input_argv = argv;

    }

    static inline void
    deconstruct_cli_arguments(int argc, char **argv)
    {

        for (int i = 0; i < argc; ++i)
        {

            const char *string = argv[i];
            free((char*)string);

        }

        free(argv);

    }

    int WINAPI 
    wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
    {

        // Allocate the console.
        AllocConsole();
        freopen_s((FILE**)stdout,   "CONOUT$",  "w", stdout);
        freopen_s((FILE**)stderr,   "CONOUT$",  "w", stderr);
        freopen_s((FILE**)stdin,    "CONIN$",   "r", stdin);

        // Construct the command line arguments equivalent to the C-standard format.
        int argc;
        char **argv;
        construct_cli_arguments(&argc, &argv);

        int result = entry(argc, argv);

        // Release the memory.
        deconstruct_cli_arguments(argc, argv);
        
        // Hold the console before exitting.
        printf("Press any character to continue.\n");
        const char c = _getch();
        return result;

    }

#endif