#include <simplex/platform/win32/win32_window.hpp>
#include <vulkan/vulkan_win32.h>
#if defined(_WIN32)

static constexpr const wchar_t *win32_window_class_name = L"SimplexWin32WindowClass";

/// Maps a virtual key onto the engine's physical-location keycodes. The lparam
/// of the originating message is required because VK_SHIFT, VK_CONTROL, VK_MENU,
/// and VK_RETURN do not distinguish left/right (or numpad) on their own; the
/// scancode and extended-key bits resolve the ambiguity.
static EngineKeyCode
win32_translate_virtual_key(WPARAM wparam, LPARAM lparam)
{

    const bool extended = (HIWORD(lparam) & KF_EXTENDED) == KF_EXTENDED;

    switch (wparam)
    {

        // VK_SHIFT doesn't carry the extended bit; the scancode resolves the side.
        case VK_SHIFT:
        {
            const UINT scancode = static_cast<UINT>((lparam >> 16) & 0xFF);
            const UINT shift_vk = MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX);
            return (shift_vk == VK_RSHIFT) ? EngineKeyCode_ShiftRight : EngineKeyCode_ShiftLeft;
        }

        case VK_CONTROL:    return (extended) ? EngineKeyCode_ControlRight : EngineKeyCode_ControlLeft;
        case VK_MENU:       return (extended) ? EngineKeyCode_AltRight : EngineKeyCode_AltLeft;
        case VK_RETURN:     return (extended) ? EngineKeyCode_NumpadEnter : EngineKeyCode_Enter;

        case VK_ESCAPE:     return EngineKeyCode_Escape;
        case VK_TAB:        return EngineKeyCode_Tab;
        case VK_SPACE:      return EngineKeyCode_Space;
        case VK_BACK:       return EngineKeyCode_Backspace;
        case VK_DELETE:     return EngineKeyCode_Delete;
        case VK_INSERT:     return EngineKeyCode_Insert;
        case VK_HOME:       return EngineKeyCode_Home;
        case VK_END:        return EngineKeyCode_End;
        case VK_PRIOR:      return EngineKeyCode_PageUp;
        case VK_NEXT:       return EngineKeyCode_PageDown;

        case VK_UP:         return EngineKeyCode_ArrowUp;
        case VK_DOWN:       return EngineKeyCode_ArrowDown;
        case VK_LEFT:       return EngineKeyCode_ArrowLeft;
        case VK_RIGHT:      return EngineKeyCode_ArrowRight;

        case VK_ADD:        return EngineKeyCode_NumpadAdd;
        case VK_SUBTRACT:   return EngineKeyCode_NumpadSubtract;
        case VK_MULTIPLY:   return EngineKeyCode_NumpadMultiply;
        case VK_DIVIDE:     return EngineKeyCode_NumpadDivide;
        case VK_DECIMAL:    return EngineKeyCode_NumpadDecimal;

        default: break;

    }

    // The remaining ranges are contiguous in both the VK space and the enum.
    if (wparam >= 'A' && wparam <= 'Z')
    {
        return static_cast<EngineKeyCode>(EngineKeyCode_KeyA + (wparam - 'A'));
    }

    if (wparam >= '0' && wparam <= '9')
    {
        return static_cast<EngineKeyCode>(EngineKeyCode_Key0 + (wparam - '0'));
    }

    if (wparam >= VK_F1 && wparam <= VK_F12)
    {
        return static_cast<EngineKeyCode>(EngineKeyCode_F1 + (wparam - VK_F1));
    }

    if (wparam >= VK_NUMPAD0 && wparam <= VK_NUMPAD9)
    {
        return static_cast<EngineKeyCode>(EngineKeyCode_Numpad0 + (wparam - VK_NUMPAD0));
    }

    return EngineKeyCode_Unknown;

}

bool spx::win32_window::
register_window_class(HINSTANCE instance)
{

    // The class is shared by every win32_window and lives for the process;
    // unregistering is intentionally left to process teardown for the same
    // reason glfwTerminate() is left to application shutdown.
    static ATOM window_class_atom = 0;
    if (window_class_atom != 0)
    {
        return true;
    }

    WNDCLASSEXW window_class    = {};
    window_class.cbSize         = sizeof(WNDCLASSEXW);
    window_class.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc    = &win32_window::window_procedure;
    window_class.hInstance      = instance;
    window_class.hCursor        = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    window_class.lpszClassName  = win32_window_class_name;

    window_class_atom = RegisterClassExW(&window_class);
    return window_class_atom != 0;

}

LRESULT CALLBACK spx::win32_window::
window_procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

    // The instance pointer arrives through CreateWindowExW's lparam; capturing
    // the handle this early lets creation-time messages (the initial WM_SIZE)
    // observe a valid window.
    if (message == WM_NCCREATE)
    {
        CREATESTRUCTW *create_info = reinterpret_cast<CREATESTRUCTW*>(lparam);
        win32_window *self = static_cast<win32_window*>(create_info->lpCreateParams);
        if (self != NULL)
        {
            self->window = hwnd;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }

    win32_window *self = reinterpret_cast<win32_window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (self == NULL)
    {
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }

    return self->handle_message(hwnd, message, wparam, lparam);

}

LRESULT spx::win32_window::
handle_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

    switch (message)
    {

        // The close request is flagged rather than acted upon; the owner decides
        // when to destroy() per the interface contract, so the message must not
        // reach DefWindowProc (which would destroy the window itself).
        case WM_CLOSE:
        {
            this->close_requested = true;
            spx::window_event event = {};
            event.type = WindowEventType_WindowClose;
            this->push_event(event);
            return 0;
        }

        case WM_SIZE:
        {

            if (wparam == SIZE_MINIMIZED)
            {
                if (this->current_size_state != size_state::minimized)
                {
                    this->current_size_state = size_state::minimized;
                    spx::window_event event = {};
                    event.type = WindowEventType_WindowMinimized;
                    this->push_event(event);
                }
                return 0;
            }

            size_state next_state = size_state::normal;
            if (wparam == SIZE_MAXIMIZED)
            {
                next_state = size_state::maximized;
            }
            else if (this->borderless_mode)
            {
                next_state = size_state::borderless;
            }

            if (next_state != this->current_size_state)
            {
                spx::window_event state_event = {};
                switch (next_state)
                {
                    case size_state::maximized:     state_event.type = WindowEventType_WindowMaximized;             break;
                    case size_state::borderless:    state_event.type = WindowEventType_WindowFullscreenBorderless;  break;
                    default:                        state_event.type = WindowEventType_WindowNormalized;            break;
                }
                this->current_size_state = next_state;
                this->push_event(state_event);
            }

            // Renderers key on this to recreate swap chains; the dimensions are
            // the client (drawable) area, not the outer frame.
            spx::window_event resize_event = {};
            resize_event.type = WindowEventType_WindowResize;
            resize_event.window_resize.width  = static_cast<int32_t>(LOWORD(lparam));
            resize_event.window_resize.height = static_cast<int32_t>(HIWORD(lparam));
            this->push_event(resize_event);
            return 0;

        }

        case WM_SETFOCUS:
        {
            spx::window_event event = {};
            event.type = WindowEventType_WindowFocused;
            this->push_event(event);
            return 0;
        }

        case WM_KILLFOCUS:
        {
            spx::window_event event = {};
            event.type = WindowEventType_WindowUnfocused;
            this->push_event(event);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            // Auto-repeats are filtered so pressed events are edge-triggered;
            // text input receives repeats through WM_CHAR instead. The message
            // still falls through to DefWindowProc so Alt+F4 and friends work.
            if ((HIWORD(lparam) & KF_REPEAT) == 0)
            {
                this->push_key_event(win32_translate_virtual_key(wparam, lparam), true);
            }
            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            this->push_key_event(win32_translate_virtual_key(wparam, lparam), false);
            break;
        }

        // The character is a UTF-16 code unit; characters outside the basic
        // multilingual plane arrive as a surrogate pair across two events.
        case WM_CHAR:
        {
            spx::window_event event = {};
            event.type = WindowEventType_InputKeyCharacter;
            event.key_character.char_code = static_cast<int32_t>(wparam);
            this->push_event(event);
            return 0;
        }

        // Consumed to suppress the system beep on unhandled alt-combinations.
        case WM_SYSCHAR:
        {
            return 0;
        }

        // Alt and F10 would otherwise enter the modal menu loop and stall the
        // render loop until the user dismisses it.
        case WM_SYSCOMMAND:
        {
            if ((wparam & 0xFFF0) == SC_KEYMENU)
            {
                return 0;
            }
            break;
        }

        case WM_LBUTTONDOWN:    { this->push_mouse_event(EngineMouseCode_MouseLeft, true);     return 0; }
        case WM_LBUTTONUP:      { this->push_mouse_event(EngineMouseCode_MouseLeft, false);    return 0; }
        case WM_RBUTTONDOWN:    { this->push_mouse_event(EngineMouseCode_MouseRight, true);    return 0; }
        case WM_RBUTTONUP:      { this->push_mouse_event(EngineMouseCode_MouseRight, false);   return 0; }
        case WM_MBUTTONDOWN:    { this->push_mouse_event(EngineMouseCode_MouseMiddle, true);   return 0; }
        case WM_MBUTTONUP:      { this->push_mouse_event(EngineMouseCode_MouseMiddle, false);  return 0; }

        // Returning TRUE is the documented contract for the X-button messages.
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        {
            const EngineMouseCode mouse_code = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)
                ? EngineMouseCode_MouseButton4
                : EngineMouseCode_MouseButton5;
            this->push_mouse_event(mouse_code, (message == WM_XBUTTONDOWN));
            return TRUE;
        }

        // The swap chain owns the client area; letting GDI clear it only flickers.
        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_DESTROY:
        {
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            return 0;
        }

        default: break;

    }

    return DefWindowProcW(hwnd, message, wparam, lparam);

}

bool spx::win32_window::
create(spx::string_view<char> window_title, uint32_t width, uint32_t height)
{

    if (this->window != NULL)
    {
        return false;
    }

    spx::logger::dispatch_diagnostic_log("Creating a Win32 window instance.");

    HINSTANCE module_instance = GetModuleHandleW(NULL);
    if (win32_window::register_window_class(module_instance) == false)
    {
        spx::logger::dispatch_critical_log("Win32 window class registration failed.");
        return false;
    }

    // The title view isn't guaranteed to be null-terminated; titles longer
    // than the buffer are truncated. UTF-8 to UTF-16 conversion never expands
    // the unit count, so clamping the byte length bounds the wide length.
    wchar_t title[256];
    constexpr int title_capacity = static_cast<int>(sizeof(title) / sizeof(title[0])) - 1;
    int title_length = static_cast<int>(window_title.size());
    if (title_length > title_capacity)
    {
        title_length = title_capacity;
    }

    int converted_length = 0;
    if (title_length > 0)
    {
        converted_length = MultiByteToWideChar(CP_UTF8, 0, window_title.data(),
                title_length, title, title_capacity);
    }
    title[converted_length] = L'\0';

    // The requested dimensions describe the client (drawable) area; the frame
    // is added on top of them.
    const DWORD style = this->get_windowed_style();
    RECT frame = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRectEx(&frame, style, FALSE, 0);

    // Windows are created hidden per the interface contract; show() reveals
    // them. The window procedure captures the handle during WM_NCCREATE.
    this->instance = module_instance;
    HWND hwnd = CreateWindowExW(0, win32_window_class_name, title, style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            frame.right - frame.left, frame.bottom - frame.top,
            NULL, NULL, module_instance, this);
    if (hwnd == NULL)
    {
        this->window = NULL;
        this->instance = NULL;
        spx::logger::dispatch_critical_log("Win32 returned NULL on window creation.");
        return false;
    }

    this->window = hwnd;
    this->close_requested = false;
    this->current_size_state = size_state::normal;
    return true;

}

void spx::win32_window::
destroy()
{

    if (this->window != NULL)
    {
        DestroyWindow(this->window);
        this->window = NULL;
    }

    this->instance = NULL;
    this->close_requested = false;
    this->borderless_mode = false;
    this->current_size_state = size_state::normal;

    // Pending events refer to a window that no longer exists.
    while (this->window_event_queue.empty() == false)
    {
        this->window_event_queue.pop();
    }

    spx::logger::dispatch_diagnostic_log("Destroying a Win32 window instance.");

}

void spx::win32_window::
poll_events()
{

    if (this->window == NULL)
    {
        return;
    }

    // Dispatch routes each message through the window procedure, which applies
    // the default handling and queues the translated window_events.
    MSG message = {};
    while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE) != 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    this->poll_controllers();

}

bool spx::win32_window::
poll_event(spx::window_event *event)
{

    if (event == NULL)
    {
        return false;
    }

    if (this->window_event_queue.empty())
    {
        return false;
    }

    *event = this->window_event_queue.front();
    this->window_event_queue.pop();
    return true;

}

void spx::win32_window::
hide()
{

    if (this->window == NULL)
    {
        return;
    }

    ShowWindow(this->window, SW_HIDE);

}

void spx::win32_window::
show()
{

    if (this->window == NULL)
    {
        return;
    }

    ShowWindow(this->window, SW_SHOW);

}

void spx::win32_window::
maximize()
{

    if (this->window == NULL)
    {
        return;
    }

    ShowWindow(this->window, SW_MAXIMIZE);

}

void spx::win32_window::
borderless()
{

    if (this->window == NULL || this->borderless_mode)
    {
        return;
    }

    // Capture the windowed geometry so normalize() can restore it.
    this->windowed_placement.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(this->window, &this->windowed_placement) == FALSE)
    {
        return;
    }

    MONITORINFO monitor_info = {};
    monitor_info.cbSize = sizeof(MONITORINFO);
    HMONITOR monitor = MonitorFromWindow(this->window, MONITOR_DEFAULTTONEAREST);
    if (GetMonitorInfoW(monitor, &monitor_info) == FALSE)
    {
        return;
    }

    // Swapping the frame for WS_POPUP at the monitor's bounds yields borderless
    // fullscreen without an exclusive mode switch. Visibility isn't touched so
    // a hidden window stays hidden. The flag is raised first so the WM_SIZE
    // this produces reports the borderless transition.
    this->borderless_mode = true;

    LONG_PTR style = GetWindowLongPtrW(this->window, GWL_STYLE);
    style &= ~(static_cast<LONG_PTR>(WS_OVERLAPPEDWINDOW));
    style |= static_cast<LONG_PTR>(WS_POPUP);
    SetWindowLongPtrW(this->window, GWL_STYLE, style);

    const RECT bounds = monitor_info.rcMonitor;
    SetWindowPos(this->window, HWND_TOP, bounds.left, bounds.top,
            bounds.right - bounds.left, bounds.bottom - bounds.top,
            SWP_FRAMECHANGED | SWP_NOACTIVATE);

}

void spx::win32_window::
normalize()
{

    if (this->window == NULL)
    {
        return;
    }

    if (this->borderless_mode)
    {
        // The flag drops first so the WM_SIZE from the restore reports a
        // normalization rather than another borderless transition.
        this->borderless_mode = false;
        this->apply_windowed_style();
        SetWindowPlacement(this->window, &this->windowed_placement);
    }

    ShowWindow(this->window, SW_RESTORE);

}

void spx::win32_window::
lock_resizing()
{

    this->resizing_locked = true;

    // The borderless frame has no resize handles to remove; the lock is
    // reapplied when the windowed style is restored.
    if (this->window == NULL || this->borderless_mode)
    {
        return;
    }

    this->apply_windowed_style();

}

void spx::win32_window::
unlock_resizing()
{

    this->resizing_locked = false;

    if (this->window == NULL || this->borderless_mode)
    {
        return;
    }

    this->apply_windowed_style();

}

bool spx::win32_window::
is_valid() const
{

    return this->window != NULL;

}

bool spx::win32_window::
should_close() const
{

    // An invalid window has nothing to keep open.
    if (this->window == NULL)
    {
        return true;
    }

    return this->close_requested;

}

int32_t spx::win32_window::
get_width() const
{

    if (this->window == NULL)
    {
        return 0;
    }

    RECT client = {};
    if (GetClientRect(this->window, &client) == FALSE)
    {
        return 0;
    }

    return static_cast<int32_t>(client.right - client.left);

}

int32_t spx::win32_window::
get_height() const
{

    if (this->window == NULL)
    {
        return 0;
    }

    RECT client = {};
    if (GetClientRect(this->window, &client) == FALSE)
    {
        return 0;
    }

    return static_cast<int32_t>(client.bottom - client.top);

}

real32_t spx::win32_window::
get_aspect_ratio() const
{

    if (this->window == NULL)
    {
        return 0.0f;
    }

    RECT client = {};
    if (GetClientRect(this->window, &client) == FALSE)
    {
        return 0.0f;
    }

    const int32_t width  = static_cast<int32_t>(client.right - client.left);
    const int32_t height = static_cast<int32_t>(client.bottom - client.top);
    if (height == 0)
    {
        return 0.0f;
    }

    return static_cast<real32_t>(width) / static_cast<real32_t>(height);

}

bool spx::win32_window::
is_visible() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return IsWindowVisible(this->window) != FALSE;

}

bool spx::win32_window::
is_hidden() const
{

    return this->is_visible() == false;

}

bool spx::win32_window::
is_minimized() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return IsIconic(this->window) != FALSE;

}

bool spx::win32_window::
is_maximized() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return IsZoomed(this->window) != FALSE;

}

bool spx::win32_window::
is_normalized() const
{

    if (this->window == NULL)
    {
        return false;
    }

    if (this->is_minimized() || this->is_maximized())
    {
        return false;
    }

    return this->borderless_mode == false;

}

bool spx::win32_window::
is_active() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return GetForegroundWindow() == this->window;

}

bool spx::win32_window::
is_inactive() const
{

    return this->is_active() == false;

}

WindowStatus spx::win32_window::
get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const
{

    if (extension_list == NULL || extension_count == NULL)
    {
        return WindowStatus_InvalidArgument;
    }

    // The win32 surface requirements are fixed and known at compile time; the
    // list is static storage per the interface's ownership contract.
    static const char *required_extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };

    *extension_list = required_extensions;
    *extension_count = static_cast<uint32_t>(sizeof(required_extensions) / sizeof(required_extensions[0]));
    return WindowStatus_OK;

}

WindowStatus spx::win32_window::
create_vulkan_surface(VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const
{

    if (surface == NULL)
    {
        return WindowStatus_InvalidArgument;
    }

    *surface = VK_NULL_HANDLE;

    if (instance == VK_NULL_HANDLE)
    {
        return WindowStatus_InvalidArgument;
    }

    if (this->window == NULL)
    {
        return WindowStatus_WindowInvalid;
    }

    VkWin32SurfaceCreateInfoKHR create_info = {};
    create_info.sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hinstance   = this->instance;
    create_info.hwnd        = this->window;

    VkResult result = vkCreateWin32SurfaceKHR(instance, &create_info, allocator, surface);
    if (result != VK_SUCCESS)
    {
        return WindowStatus_VulkanSurfaceFailed;
    }

    return WindowStatus_OK;

}

WindowStatus spx::win32_window::
get_native_handle(void **native_handle) const
{

    if (native_handle == NULL)
    {
        return WindowStatus_InvalidArgument;
    }

    *native_handle = NULL;

    if (this->window == NULL)
    {
        return WindowStatus_WindowInvalid;
    }

    *native_handle = static_cast<void*>(this->window);
    return WindowStatus_OK;

}

DWORD spx::win32_window::
get_windowed_style() const
{

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (this->resizing_locked)
    {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }

    return style;

}

void spx::win32_window::
apply_windowed_style()
{

    // Visibility and min/max state live in the style word and must survive the
    // swap; the frame-changed pos call makes the new frame take effect.
    LONG_PTR current = GetWindowLongPtrW(this->window, GWL_STYLE);
    LONG_PTR style = static_cast<LONG_PTR>(this->get_windowed_style());
    style |= (current & (WS_VISIBLE | WS_MINIMIZE | WS_MAXIMIZE));

    SetWindowLongPtrW(this->window, GWL_STYLE, style);
    SetWindowPos(this->window, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

}

void spx::win32_window::
poll_controllers()
{

    // Controllers (XInput) are polled rather than message-driven, so they are
    // sampled here once per poll_events() pump. When implemented, connection
    // changes and button transitions for each EngineControllerIndex slot are
    // translated into window_events and pushed like any other input so that
    // poll_event() delivers them uniformly.

}

void spx::win32_window::
push_event(const spx::window_event &event)
{

    // Default handling already happened at translation time, so an overflow
    // only loses the notification; dropping the oldest keeps the most recent
    // events available to the user.
    if (this->window_event_queue.full())
    {
        this->window_event_queue.pop();
    }

    this->window_event_queue.push(event);

}

void spx::win32_window::
push_key_event(EngineKeyCode key_code, bool pressed)
{

    spx::window_event event = {};
    if (pressed)
    {
        event.type = WindowEventType_InputKeyPressed;
        event.key_pressed.key_code = key_code;
    }
    else
    {
        event.type = WindowEventType_InputKeyReleased;
        event.key_released.key_code = key_code;
    }

    this->push_event(event);

}

void spx::win32_window::
push_mouse_event(EngineMouseCode mouse_code, bool pressed)
{

    spx::window_event event = {};
    if (pressed)
    {
        event.type = WindowEventType_InputMousePressed;
        event.mouse_pressed.mouse_code = mouse_code;
    }
    else
    {
        event.type = WindowEventType_InputMouseReleased;
        event.mouse_released.mouse_code = mouse_code;
    }

    this->push_event(event);

}

#endif