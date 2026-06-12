#include <simplex/platform/darwin/darwin_window.hpp>
#include <utils/logging.hpp>
#if defined(__APPLE__)
#include <vulkan/vulkan_metal.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

/// Maps a darwin virtual keycode onto the engine's physical-location keycodes.
/// Cocoa keycodes already identify physical key positions on the ANSI layout
/// (they are Carbon's kVK_* values, spelled in comments here to avoid the
/// Carbon framework dependency), so no scancode resolution is required.
static EngineKeyCode
darwin_translate_virtual_key(uint16_t key_code)
{

    switch (key_code)
    {

        case 0x00: return EngineKeyCode_KeyA;           // kVK_ANSI_A
        case 0x01: return EngineKeyCode_KeyS;           // kVK_ANSI_S
        case 0x02: return EngineKeyCode_KeyD;           // kVK_ANSI_D
        case 0x03: return EngineKeyCode_KeyF;           // kVK_ANSI_F
        case 0x04: return EngineKeyCode_KeyH;           // kVK_ANSI_H
        case 0x05: return EngineKeyCode_KeyG;           // kVK_ANSI_G
        case 0x06: return EngineKeyCode_KeyZ;           // kVK_ANSI_Z
        case 0x07: return EngineKeyCode_KeyX;           // kVK_ANSI_X
        case 0x08: return EngineKeyCode_KeyC;           // kVK_ANSI_C
        case 0x09: return EngineKeyCode_KeyV;           // kVK_ANSI_V
        case 0x0B: return EngineKeyCode_KeyB;           // kVK_ANSI_B
        case 0x0C: return EngineKeyCode_KeyQ;           // kVK_ANSI_Q
        case 0x0D: return EngineKeyCode_KeyW;           // kVK_ANSI_W
        case 0x0E: return EngineKeyCode_KeyE;           // kVK_ANSI_E
        case 0x0F: return EngineKeyCode_KeyR;           // kVK_ANSI_R
        case 0x10: return EngineKeyCode_KeyY;           // kVK_ANSI_Y
        case 0x11: return EngineKeyCode_KeyT;           // kVK_ANSI_T
        case 0x12: return EngineKeyCode_Key1;           // kVK_ANSI_1
        case 0x13: return EngineKeyCode_Key2;           // kVK_ANSI_2
        case 0x14: return EngineKeyCode_Key3;           // kVK_ANSI_3
        case 0x15: return EngineKeyCode_Key4;           // kVK_ANSI_4
        case 0x16: return EngineKeyCode_Key6;           // kVK_ANSI_6
        case 0x17: return EngineKeyCode_Key5;           // kVK_ANSI_5
        case 0x19: return EngineKeyCode_Key9;           // kVK_ANSI_9
        case 0x1A: return EngineKeyCode_Key7;           // kVK_ANSI_7
        case 0x1C: return EngineKeyCode_Key8;           // kVK_ANSI_8
        case 0x1D: return EngineKeyCode_Key0;           // kVK_ANSI_0
        case 0x1F: return EngineKeyCode_KeyO;           // kVK_ANSI_O
        case 0x20: return EngineKeyCode_KeyU;           // kVK_ANSI_U
        case 0x22: return EngineKeyCode_KeyI;           // kVK_ANSI_I
        case 0x23: return EngineKeyCode_KeyP;           // kVK_ANSI_P
        case 0x25: return EngineKeyCode_KeyL;           // kVK_ANSI_L
        case 0x26: return EngineKeyCode_KeyJ;           // kVK_ANSI_J
        case 0x28: return EngineKeyCode_KeyK;           // kVK_ANSI_K
        case 0x2D: return EngineKeyCode_KeyN;           // kVK_ANSI_N
        case 0x2E: return EngineKeyCode_KeyM;           // kVK_ANSI_M

        case 0x24: return EngineKeyCode_Enter;          // kVK_Return
        case 0x30: return EngineKeyCode_Tab;            // kVK_Tab
        case 0x31: return EngineKeyCode_Space;          // kVK_Space
        case 0x33: return EngineKeyCode_Backspace;      // kVK_Delete
        case 0x35: return EngineKeyCode_Escape;         // kVK_Escape

        case 0x38: return EngineKeyCode_ShiftLeft;      // kVK_Shift
        case 0x3C: return EngineKeyCode_ShiftRight;     // kVK_RightShift
        case 0x3B: return EngineKeyCode_ControlLeft;    // kVK_Control
        case 0x3E: return EngineKeyCode_ControlRight;   // kVK_RightControl
        case 0x3A: return EngineKeyCode_AltLeft;        // kVK_Option
        case 0x3D: return EngineKeyCode_AltRight;       // kVK_RightOption

        case 0x7A: return EngineKeyCode_F1;             // kVK_F1
        case 0x78: return EngineKeyCode_F2;             // kVK_F2
        case 0x63: return EngineKeyCode_F3;             // kVK_F3
        case 0x76: return EngineKeyCode_F4;             // kVK_F4
        case 0x60: return EngineKeyCode_F5;             // kVK_F5
        case 0x61: return EngineKeyCode_F6;             // kVK_F6
        case 0x62: return EngineKeyCode_F7;             // kVK_F7
        case 0x64: return EngineKeyCode_F8;             // kVK_F8
        case 0x65: return EngineKeyCode_F9;             // kVK_F9
        case 0x6D: return EngineKeyCode_F10;            // kVK_F10
        case 0x67: return EngineKeyCode_F11;            // kVK_F11
        case 0x6F: return EngineKeyCode_F12;            // kVK_F12

        // The help key occupies the insert position on full-size Apple boards.
        case 0x72: return EngineKeyCode_Insert;         // kVK_Help
        case 0x75: return EngineKeyCode_Delete;         // kVK_ForwardDelete
        case 0x73: return EngineKeyCode_Home;           // kVK_Home
        case 0x77: return EngineKeyCode_End;            // kVK_End
        case 0x74: return EngineKeyCode_PageUp;         // kVK_PageUp
        case 0x79: return EngineKeyCode_PageDown;       // kVK_PageDown

        case 0x7E: return EngineKeyCode_ArrowUp;        // kVK_UpArrow
        case 0x7D: return EngineKeyCode_ArrowDown;      // kVK_DownArrow
        case 0x7B: return EngineKeyCode_ArrowLeft;      // kVK_LeftArrow
        case 0x7C: return EngineKeyCode_ArrowRight;     // kVK_RightArrow

        case 0x52: return EngineKeyCode_Numpad0;        // kVK_ANSI_Keypad0
        case 0x53: return EngineKeyCode_Numpad1;        // kVK_ANSI_Keypad1
        case 0x54: return EngineKeyCode_Numpad2;        // kVK_ANSI_Keypad2
        case 0x55: return EngineKeyCode_Numpad3;        // kVK_ANSI_Keypad3
        case 0x56: return EngineKeyCode_Numpad4;        // kVK_ANSI_Keypad4
        case 0x57: return EngineKeyCode_Numpad5;        // kVK_ANSI_Keypad5
        case 0x58: return EngineKeyCode_Numpad6;        // kVK_ANSI_Keypad6
        case 0x59: return EngineKeyCode_Numpad7;        // kVK_ANSI_Keypad7
        case 0x5B: return EngineKeyCode_Numpad8;        // kVK_ANSI_Keypad8
        case 0x5C: return EngineKeyCode_Numpad9;        // kVK_ANSI_Keypad9
        case 0x45: return EngineKeyCode_NumpadAdd;      // kVK_ANSI_KeypadPlus
        case 0x4E: return EngineKeyCode_NumpadSubtract; // kVK_ANSI_KeypadMinus
        case 0x43: return EngineKeyCode_NumpadMultiply; // kVK_ANSI_KeypadMultiply
        case 0x4B: return EngineKeyCode_NumpadDivide;   // kVK_ANSI_KeypadDivide
        case 0x41: return EngineKeyCode_NumpadDecimal;  // kVK_ANSI_KeypadDecimal
        case 0x4C: return EngineKeyCode_NumpadEnter;    // kVK_ANSI_KeypadEnter

        default: break;

    }

    return EngineKeyCode_Unknown;

}

/// Modifier keys arrive as flag transitions rather than key down/up events;
/// the keycode identifies which modifier changed and the device-dependent
/// modifier bits (the NX_DEVICE* masks, spelled raw to avoid the IOKit header
/// dependency) recover whether it was a press or a release.
static bool
darwin_modifier_is_pressed(uint16_t key_code, uint64_t modifier_flags)
{

    switch (key_code)
    {
        case 0x38: return (modifier_flags & 0x0002) != 0; // kVK_Shift        / NX_DEVICELSHIFTKEYMASK
        case 0x3C: return (modifier_flags & 0x0004) != 0; // kVK_RightShift   / NX_DEVICERSHIFTKEYMASK
        case 0x3B: return (modifier_flags & 0x0001) != 0; // kVK_Control      / NX_DEVICELCTLKEYMASK
        case 0x3E: return (modifier_flags & 0x2000) != 0; // kVK_RightControl / NX_DEVICERCTLKEYMASK
        case 0x3A: return (modifier_flags & 0x0020) != 0; // kVK_Option       / NX_DEVICELALTKEYMASK
        case 0x3D: return (modifier_flags & 0x0040) != 0; // kVK_RightOption  / NX_DEVICERALTKEYMASK
        default:   return false;
    }

}

static EngineMouseCode
darwin_translate_mouse_button(NSEvent *event)
{

    switch ([event buttonNumber])
    {
        case 0:  return EngineMouseCode_MouseLeft;
        case 1:  return EngineMouseCode_MouseRight;
        case 2:  return EngineMouseCode_MouseMiddle;
        case 3:  return EngineMouseCode_MouseButton4;
        case 4:  return EngineMouseCode_MouseButton5;
        case 5:  return EngineMouseCode_MouseButton6;
        case 6:  return EngineMouseCode_MouseButton7;
        case 7:  return EngineMouseCode_MouseButton8;
        default: return EngineMouseCode_Unknown;
    }

}

namespace spx
{

    /// The Objective-C delegate below can not be befriended by a C++ class, so
    /// its callbacks route through this bridge into the window's private
    /// handlers instead.
    struct darwin_window_bridge
    {

        static void
        handle_close_request(darwin_window *window)
        {
            if (window != NULL)
            {
                window->handle_close_request();
            }
        }

        static void
        handle_resize(darwin_window *window)
        {
            if (window != NULL)
            {
                window->handle_resize();
            }
        }

        static void
        handle_focus(darwin_window *window, bool focused)
        {
            if (window != NULL)
            {
                window->handle_focus(focused);
            }
        }

        static void
        handle_miniaturize(darwin_window *window, bool miniaturized)
        {
            if (window != NULL)
            {
                window->handle_miniaturize(miniaturized);
            }
        }

        static void
        handle_backing_changed(darwin_window *window)
        {
            if (window != NULL)
            {
                window->handle_backing_changed();
            }
        }

    };

}

/// A borderless NSWindow refuses key and main status by default, which would
/// drop keyboard input the moment borderless() swaps the style mask.
@interface SpxDarwinWindow : NSWindow
@end

@implementation SpxDarwinWindow

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end

@interface SpxDarwinWindowDelegate : NSObject <NSWindowDelegate>
{
    @public
    spx::darwin_window *owner;
}
@end

@implementation SpxDarwinWindowDelegate

// The close request is flagged rather than acted upon; the owner decides when
// to destroy() per the interface contract, so Cocoa must not close the window
// itself.
- (BOOL)windowShouldClose:(NSWindow *)sender
{
    spx::darwin_window_bridge::handle_close_request(self->owner);
    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_resize(self->owner);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_miniaturize(self->owner, true);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_miniaturize(self->owner, false);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_focus(self->owner, true);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_focus(self->owner, false);
}

// Native (green button) fullscreen maps onto the same borderless state the
// manual presentation mode uses; the resize handler derives the state from
// the style mask.
- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_resize(self->owner);
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_resize(self->owner);
}

// Fires when the window lands on a display with a different point-to-pixel
// scale, which resizes the drawable without changing the frame.
- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
    spx::darwin_window_bridge::handle_backing_changed(self->owner);
}

@end

bool spx::darwin_window::
initialize_application()
{

    // The application object is shared by every darwin_window and lives for
    // the process; teardown is intentionally left to process exit for the same
    // reason glfwTerminate() is left to application shutdown.
    static bool initialized = false;
    if (initialized)
    {
        return true;
    }

    [NSApplication sharedApplication];

    // Processes launched outside of an app bundle default to the prohibited
    // activation policy, which can never show windows or take focus.
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];

    initialized = true;
    return true;

}

bool spx::darwin_window::
translate_event(NSEvent *event)
{

    // Events for other windows (or for none) carry no input for this one.
    if ([event window] != this->window)
    {
        return true;
    }

    switch ([event type])
    {

        case NSEventTypeKeyDown:
        {

            // Auto-repeats are filtered so pressed events are edge-triggered;
            // text input receives repeats through the character events below.
            if ([event isARepeat] == NO)
            {
                this->push_key_event(darwin_translate_virtual_key([event keyCode]), true);
            }

            // Each character is a UTF-16 code unit; characters outside the
            // basic multilingual plane arrive as a surrogate pair across two
            // units. The function-key private range (arrows, F-keys) is not
            // text and is skipped.
            NSString *characters = [event characters];
            for (NSUInteger index = 0; index < [characters length]; ++index)
            {

                const unichar unit = [characters characterAtIndex:index];
                if (unit >= 0xF700 && unit <= 0xF7FF)
                {
                    continue;
                }

                spx::window_event character_event = {};
                character_event.type = WindowEventType_InputKeyCharacter;
                character_event.key_character.char_code = static_cast<int32_t>(unit);
                this->push_event(character_event);

            }

            // Consumed to suppress the system beep on keys no responder
            // handles; with no menu there are no key equivalents to lose.
            return false;

        }

        case NSEventTypeKeyUp:
        {
            this->push_key_event(darwin_translate_virtual_key([event keyCode]), false);
            return true;
        }

        case NSEventTypeFlagsChanged:
        {

            // Unmapped modifiers (command, caps lock) have no engine keycode
            // and pass through untranslated.
            const EngineKeyCode key_code = darwin_translate_virtual_key([event keyCode]);
            if (key_code != EngineKeyCode_Unknown)
            {
                const bool pressed = darwin_modifier_is_pressed([event keyCode],
                        static_cast<uint64_t>([event modifierFlags]));
                this->push_key_event(key_code, pressed);
            }

            return true;

        }

        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
        {

            // Presses outside the content view belong to the frame (the title
            // bar), mirroring win32's client/non-client distinction. The event
            // continues on to Cocoa either way so dragging and the window
            // controls keep working.
            const NSPoint location = [this->view convertPoint:[event locationInWindow] fromView:nil];
            if (NSPointInRect(location, [this->view bounds]))
            {
                this->push_mouse_event(darwin_translate_mouse_button(event), true);
            }

            return true;

        }

        // Releases are reported regardless of position so a press dragged off
        // of the content view still observes its release.
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
        {
            this->push_mouse_event(darwin_translate_mouse_button(event), false);
            return true;
        }

        default: break;

    }

    return true;

}

void spx::darwin_window::
handle_close_request()
{

    this->close_requested = true;

    spx::window_event event = {};
    event.type = WindowEventType_WindowClose;
    this->push_event(event);

}

void spx::darwin_window::
handle_resize()
{

    if (this->window == NULL || this->view == NULL)
    {
        return;
    }

    // Minimization arrives through the miniaturize notifications instead.
    if ([this->window isMiniaturized])
    {
        return;
    }

    // Borderless is checked ahead of zoomed because a screen-sized frame
    // reads as zoomed on its own.
    size_state next_state = size_state::normal;
    if (this->borderless_mode || ([this->window styleMask] & NSWindowStyleMaskFullScreen) != 0)
    {
        next_state = size_state::borderless;
    }
    else if ([this->window isZoomed])
    {
        next_state = size_state::maximized;
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

    this->update_drawable_size();

    // Renderers key on this to recreate swap chains; the dimensions are the
    // backing (pixel) size of the drawable area, not the outer frame.
    const NSRect backing = [this->view convertRectToBacking:[this->view bounds]];
    spx::window_event resize_event = {};
    resize_event.type = WindowEventType_WindowResize;
    resize_event.window_resize.width  = static_cast<int32_t>(backing.size.width);
    resize_event.window_resize.height = static_cast<int32_t>(backing.size.height);
    this->push_event(resize_event);

}

void spx::darwin_window::
handle_focus(bool focused)
{

    spx::window_event event = {};
    event.type = (focused) ? WindowEventType_WindowFocused : WindowEventType_WindowUnfocused;
    this->push_event(event);

}

void spx::darwin_window::
handle_miniaturize(bool miniaturized)
{

    if (miniaturized)
    {

        if (this->current_size_state != size_state::minimized)
        {
            this->current_size_state = size_state::minimized;
            spx::window_event event = {};
            event.type = WindowEventType_WindowMinimized;
            this->push_event(event);
        }

        return;

    }

    // Restoring doesn't change the frame, so no resize notification fires; the
    // resize handler recomputes the restored state and geometry.
    this->handle_resize();

}

void spx::darwin_window::
handle_backing_changed()
{

    // The frame is unchanged but the pixel size of the drawable isn't; the
    // resize handler refreshes the layer and reports the new dimensions.
    this->handle_resize();

}

bool spx::darwin_window::
create(spx::string_view<char> window_title, uint32_t width, uint32_t height)
{

    if (this->window != NULL)
    {
        return false;
    }

    spx::logger::dispatch_information_log("Initializing Darwin window for MacOSX.");

    if (darwin_window::initialize_application() == false)
    {
        spx::logger::dispatch_critical_log("Cocoa application initialization failed.");
        return false;
    }

    @autoreleasepool
    {

        // The title view isn't guaranteed to be null-terminated; the length
        // constructor consumes the view as-is.
        NSString *title = [[[NSString alloc] initWithBytes:window_title.data()
                                                    length:window_title.size()
                                                  encoding:NSUTF8StringEncoding] autorelease];
        if (title == nil)
        {
            title = @"";
        }

        // The requested dimensions describe the content (drawable) area in
        // points; the frame is added on top of them.
        const NSRect content_rect = NSMakeRect(0.0, 0.0,
                static_cast<CGFloat>(width), static_cast<CGFloat>(height));

        const NSUInteger style_mask = static_cast<NSUInteger>(this->get_windowed_style_mask());
        SpxDarwinWindow *cocoa_window = [[SpxDarwinWindow alloc]
                initWithContentRect:content_rect
                          styleMask:style_mask
                            backing:NSBackingStoreBuffered
                              defer:NO];
        if (cocoa_window == nil)
        {
            spx::logger::dispatch_critical_log("Cocoa returned nil on window creation.");
            return false;
        }

        // The interface owns the window's lifetime, so the close button must
        // not release it. Windows are created hidden per the interface
        // contract; Cocoa windows are ordered out until explicitly fronted, so
        // nothing needs hiding here and show() reveals them.
        [cocoa_window setReleasedWhenClosed:NO];
        [cocoa_window setTitle:title];
        [cocoa_window center];

        // The content view hosts a CAMetalLayer rather than backing its own
        // layer; this is the surface Metal renderers attach to and MoltenVK
        // presents through. The device and pixel format are renderer concerns.
        NSView *content_view = [[NSView alloc] initWithFrame:content_rect];
        CAMetalLayer *metal_layer = [[CAMetalLayer layer] retain];
        [content_view setLayer:metal_layer];
        [content_view setWantsLayer:YES];
        [cocoa_window setContentView:content_view];

        SpxDarwinWindowDelegate *window_delegate = [[SpxDarwinWindowDelegate alloc] init];
        window_delegate->owner = this;
        [cocoa_window setDelegate:window_delegate];

        this->window = cocoa_window;
        this->view = content_view;
        this->layer = metal_layer;
        this->delegate = window_delegate;
        this->close_requested = false;
        this->current_size_state = size_state::normal;
        this->update_drawable_size();

    }

    return true;

}

void spx::darwin_window::
destroy()
{

    spx::logger::dispatch_information_log("Destroying Darwin window for MacOSX.");

    @autoreleasepool
    {

        if (this->window != NULL)
        {
            [this->window setDelegate:nil];
            [this->window close];
            [this->window release];
            this->window = NULL;
        }

        if (this->view != NULL)
        {
            [this->view release];
            this->view = NULL;
        }

        if (this->layer != NULL)
        {
            [this->layer release];
            this->layer = NULL;
        }

        if (this->delegate != NULL)
        {
            [this->delegate release];
            this->delegate = NULL;
        }

    }

    this->close_requested = false;
    this->borderless_mode = false;
    this->current_size_state = size_state::normal;

    // Pending events refer to a window that no longer exists.
    while (this->window_event_queue.empty() == false)
    {
        this->window_event_queue.pop();
    }

}

void spx::darwin_window::
poll_events()
{

    if (this->window == NULL)
    {
        return;
    }

    // Each event passes through the translation pass, which applies the
    // default handling and queues the translated window_events before the
    // event continues on to Cocoa; delegate callbacks (resize, focus, close)
    // also fire from inside this pump.
    @autoreleasepool
    {

        for (;;)
        {

            NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
            {
                break;
            }

            if (this->translate_event(event))
            {
                [NSApp sendEvent:event];
            }

        }

    }

    this->poll_controllers();

}

bool spx::darwin_window::
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

void spx::darwin_window::
hide()
{

    if (this->window == NULL)
    {
        return;
    }

    [this->window orderOut:nil];

}

void spx::darwin_window::
show()
{

    if (this->window == NULL)
    {
        return;
    }

    [this->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

}

void spx::darwin_window::
maximize()
{

    if (this->window == NULL)
    {
        return;
    }

    // Zoom toggles, so an already-zoomed window is left alone.
    if ([this->window isZoomed] == NO)
    {
        [this->window zoom:nil];
    }

}

void spx::darwin_window::
borderless()
{

    if (this->window == NULL || this->borderless_mode)
    {
        return;
    }

    // Native (green button) fullscreen already owns the screen.
    if (([this->window styleMask] & NSWindowStyleMaskFullScreen) != 0)
    {
        return;
    }

    NSScreen *screen = [this->window screen];
    if (screen == nil)
    {
        screen = [NSScreen mainScreen];
    }
    if (screen == nil)
    {
        return;
    }

    // Capture the windowed geometry so normalize() can restore it.
    const NSRect frame = [this->window frame];
    this->windowed_x      = frame.origin.x;
    this->windowed_y      = frame.origin.y;
    this->windowed_width  = frame.size.width;
    this->windowed_height = frame.size.height;

    // Swapping the frame for a borderless mask at the screen's bounds yields
    // borderless fullscreen without a space transition. Visibility isn't
    // touched so a hidden window stays hidden. The flag is raised first so the
    // resize this produces reports the borderless transition; the level lift
    // places the window above the menu bar and dock.
    this->borderless_mode = true;
    [this->window setStyleMask:NSWindowStyleMaskBorderless];
    [this->window setLevel:NSMainMenuWindowLevel + 1];
    [this->window setFrame:[screen frame] display:YES];

}

void spx::darwin_window::
normalize()
{

    if (this->window == NULL)
    {
        return;
    }

    // Native fullscreen restores through the space transition; the delegate's
    // exit notification reports the state change when it completes.
    if (([this->window styleMask] & NSWindowStyleMaskFullScreen) != 0)
    {
        [this->window toggleFullScreen:nil];
    }
    else if (this->borderless_mode)
    {
        // The flag drops first so the resize from the restore reports a
        // normalization rather than another borderless transition.
        this->borderless_mode = false;
        [this->window setLevel:NSNormalWindowLevel];
        this->apply_windowed_style();
        [this->window setFrame:NSMakeRect(this->windowed_x, this->windowed_y,
                this->windowed_width, this->windowed_height) display:YES];
    }

    if ([this->window isMiniaturized])
    {
        [this->window deminiaturize:nil];
    }

    if ([this->window isZoomed])
    {
        [this->window zoom:nil];
    }

}

void spx::darwin_window::
lock_resizing()
{

    this->resizing_locked = true;

    // The borderless frame has no resize edges to remove; the lock is
    // reapplied when the windowed style is restored.
    if (this->window == NULL || this->borderless_mode)
    {
        return;
    }

    this->apply_windowed_style();

}

void spx::darwin_window::
unlock_resizing()
{

    this->resizing_locked = false;

    if (this->window == NULL || this->borderless_mode)
    {
        return;
    }

    this->apply_windowed_style();

}

bool spx::darwin_window::
is_valid() const
{

    return this->window != NULL;

}

bool spx::darwin_window::
should_close() const
{

    // An invalid window has nothing to keep open.
    if (this->window == NULL)
    {
        return true;
    }

    return this->close_requested;

}

int32_t spx::darwin_window::
get_width() const
{

    if (this->window == NULL || this->view == NULL)
    {
        return 0;
    }

    // Dimensions are reported in backing (pixel) units rather than points so
    // they agree with the drawable size renderers consume, matching the pixel
    // semantics of the win32 client rect.
    const NSRect backing = [this->view convertRectToBacking:[this->view bounds]];
    return static_cast<int32_t>(backing.size.width);

}

int32_t spx::darwin_window::
get_height() const
{

    if (this->window == NULL || this->view == NULL)
    {
        return 0;
    }

    const NSRect backing = [this->view convertRectToBacking:[this->view bounds]];
    return static_cast<int32_t>(backing.size.height);

}

real32_t spx::darwin_window::
get_aspect_ratio() const
{

    if (this->window == NULL || this->view == NULL)
    {
        return 0.0f;
    }

    const NSRect backing = [this->view convertRectToBacking:[this->view bounds]];
    if (backing.size.height <= 0.0)
    {
        return 0.0f;
    }

    return static_cast<real32_t>(backing.size.width / backing.size.height);

}

bool spx::darwin_window::
is_visible() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return [this->window isVisible] != NO;

}

bool spx::darwin_window::
is_hidden() const
{

    return this->is_visible() == false;

}

bool spx::darwin_window::
is_minimized() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return [this->window isMiniaturized] != NO;

}

bool spx::darwin_window::
is_maximized() const
{

    if (this->window == NULL)
    {
        return false;
    }

    // A borderless or fullscreen frame fills the screen and reads as zoomed
    // on its own.
    if (this->borderless_mode || ([this->window styleMask] & NSWindowStyleMaskFullScreen) != 0)
    {
        return false;
    }

    return [this->window isZoomed] != NO;

}

bool spx::darwin_window::
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

    if (([this->window styleMask] & NSWindowStyleMaskFullScreen) != 0)
    {
        return false;
    }

    return this->borderless_mode == false;

}

bool spx::darwin_window::
is_active() const
{

    if (this->window == NULL)
    {
        return false;
    }

    return [this->window isKeyWindow] != NO;

}

bool spx::darwin_window::
is_inactive() const
{

    return this->is_active() == false;

}

WindowStatus spx::darwin_window::
get_vulkan_instance_extensions(const char ***extension_list, uint32_t *extension_count) const
{

    if (extension_list == NULL || extension_count == NULL)
    {
        return WindowStatus_InvalidArgument;
    }

    // Presentation goes through MoltenVK's metal surface; the requirements are
    // fixed and known at compile time, so the list is static storage per the
    // interface's ownership contract. VK_KHR_portability_enumeration and its
    // instance create flag are instance-creation concerns owned by the
    // renderer, not surface requirements.
    static const char *required_extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_METAL_SURFACE_EXTENSION_NAME,
    };

    *extension_list = required_extensions;
    *extension_count = static_cast<uint32_t>(sizeof(required_extensions) / sizeof(required_extensions[0]));
    return WindowStatus_OK;

}

WindowStatus spx::darwin_window::
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

    if (this->window == NULL || this->layer == NULL)
    {
        return WindowStatus_WindowInvalid;
    }

    // The surface wraps the same CAMetalLayer that Metal renderers attach to;
    // both presentation paths share one drawable target.
    VkMetalSurfaceCreateInfoEXT create_info = {};
    create_info.sType   = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    create_info.pLayer  = this->layer;

    VkResult result = vkCreateMetalSurfaceEXT(instance, &create_info, allocator, surface);
    if (result != VK_SUCCESS)
    {
        return WindowStatus_VulkanSurfaceFailed;
    }

    return WindowStatus_OK;

}

WindowStatus spx::darwin_window::
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

    // The native handle is the NSWindow; Metal renderers should prefer
    // get_metal_layer(), which is the actual presentation target.
    *native_handle = static_cast<void*>(this->window);
    return WindowStatus_OK;

}

uint64_t spx::darwin_window::
get_windowed_style_mask() const
{

    NSUInteger style_mask = NSWindowStyleMaskTitled
                          | NSWindowStyleMaskClosable
                          | NSWindowStyleMaskMiniaturizable;
    if (this->resizing_locked == false)
    {
        style_mask |= NSWindowStyleMaskResizable;
    }

    return static_cast<uint64_t>(style_mask);

}

void spx::darwin_window::
apply_windowed_style()
{

    // Unlike win32 there is no visibility or min/max state carried in the
    // mask to preserve; Cocoa tracks those independently of the style.
    [this->window setStyleMask:static_cast<NSUInteger>(this->get_windowed_style_mask())];

}

void spx::darwin_window::
update_drawable_size()
{

    if (this->window == NULL || this->view == NULL || this->layer == NULL)
    {
        return;
    }

    // The layer doesn't track the view's backing size on its own; without
    // this, resizes stretch the last presented drawable instead of
    // re-rasterizing at the new size.
    const NSRect backing = [this->view convertRectToBacking:[this->view bounds]];
    [this->layer setContentsScale:[this->window backingScaleFactor]];
    [this->layer setDrawableSize:NSMakeSize(backing.size.width, backing.size.height)];

}

void spx::darwin_window::
poll_controllers()
{

    // Controllers (the GameController framework) are sampled rather than
    // pump-driven, so they are polled here once per poll_events() pump. When
    // implemented, connection changes and button transitions for each
    // EngineControllerIndex slot are translated into window_events and pushed
    // like any other input so that poll_event() delivers them uniformly.

}

void spx::darwin_window::
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

void spx::darwin_window::
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

void spx::darwin_window::
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
