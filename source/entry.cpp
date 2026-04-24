#include <utils/defs.hpp>
#include <utils/resources.hpp>
#include <cli/cli.hpp>

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

    // Flags.
    cli.add_flag_rule('r', "Re-run the previous session.");
    cli.add_flag_rule('M', "Enable memory diagnostics.");

    // Arguments.
    cli.add_argument_rule("--no-hardware",          {},                                              "Disable hardware acceleration.");
    cli.add_argument_rule("--memory-limit",         { CLIValueType::Integer },                       "Cap the resident memory footprint.");
    cli.add_argument_rule("--image-format",         { CLIValueType::String },                        "Override the emitted image format (PPM, BMP, ...).");
    cli.add_argument_rule("--image-size",           { CLIValueType::Integer, CLIValueType::Integer },"Force the output image dimensions.");
    cli.add_argument_rule("--run-all-tests",        {},                                              "Run the full test suite.");
    cli.add_argument_rule("--run-rdview-tests",     {},                                              "Run the rdview test suite.");
    cli.add_argument_rule("--run-memory-tests",     {},                                              "Run the memory test suite.");
    cli.add_argument_rule("--force-renderer",       { CLIValueType::String },                        "Force a specific renderer backend.");
    cli.add_argument_rule("--compare-to",           { CLIValueType::String, CLIValueType::String },  "Compare against the given renderer backends.");

    // Positionals.
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