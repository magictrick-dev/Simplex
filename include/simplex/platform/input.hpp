#pragma once
#include <utils/defs.hpp>

/// @brief  The keycodes correspond to platform-agnostic mappings that should reliably
///         translate to physical locations on a standard QWERTY layout.
enum EngineKeyCode : uint32_t
{

    EngineKeyCode_KeyA,
    EngineKeyCode_KeyB,
    EngineKeyCode_KeyC,
    EngineKeyCode_KeyD,
    EngineKeyCode_KeyE,
    EngineKeyCode_KeyF,
    EngineKeyCode_KeyG,
    EngineKeyCode_KeyH,
    EngineKeyCode_KeyI,
    EngineKeyCode_KeyJ,
    EngineKeyCode_KeyK,
    EngineKeyCode_KeyL,
    EngineKeyCode_KeyM,
    EngineKeyCode_KeyN,
    EngineKeyCode_KeyO,
    EngineKeyCode_KeyP,
    EngineKeyCode_KeyQ,
    EngineKeyCode_KeyR,
    EngineKeyCode_KeyS,
    EngineKeyCode_KeyT,
    EngineKeyCode_KeyU,
    EngineKeyCode_KeyV,
    EngineKeyCode_KeyW,
    EngineKeyCode_KeyX,
    EngineKeyCode_KeyY,
    EngineKeyCode_KeyZ,

    // Numbers (top row)
    EngineKeyCode_Key0,
    EngineKeyCode_Key1,
    EngineKeyCode_Key2,
    EngineKeyCode_Key3,
    EngineKeyCode_Key4,
    EngineKeyCode_Key5,
    EngineKeyCode_Key6,
    EngineKeyCode_Key7,
    EngineKeyCode_Key8,
    EngineKeyCode_Key9,

    // Function keys
    EngineKeyCode_F1,
    EngineKeyCode_F2,
    EngineKeyCode_F3,
    EngineKeyCode_F4,
    EngineKeyCode_F5,
    EngineKeyCode_F6,
    EngineKeyCode_F7,
    EngineKeyCode_F8,
    EngineKeyCode_F9,
    EngineKeyCode_F10,
    EngineKeyCode_F11,
    EngineKeyCode_F12,

    // Control keys
    EngineKeyCode_Escape,
    EngineKeyCode_Tab,
    EngineKeyCode_ShiftLeft,
    EngineKeyCode_ShiftRight,
    EngineKeyCode_ControlLeft,
    EngineKeyCode_ControlRight,
    EngineKeyCode_AltLeft,
    EngineKeyCode_AltRight,
    EngineKeyCode_Space,
    EngineKeyCode_Enter,
    EngineKeyCode_Backspace,
    EngineKeyCode_Delete,
    EngineKeyCode_Insert,
    EngineKeyCode_Home,
    EngineKeyCode_End,
    EngineKeyCode_PageUp,
    EngineKeyCode_PageDown,

    // Arrows
    EngineKeyCode_ArrowUp,
    EngineKeyCode_ArrowDown,
    EngineKeyCode_ArrowLeft,
    EngineKeyCode_ArrowRight,

    // Numpad
    EngineKeyCode_Numpad0,
    EngineKeyCode_Numpad1,
    EngineKeyCode_Numpad2,
    EngineKeyCode_Numpad3,
    EngineKeyCode_Numpad4,
    EngineKeyCode_Numpad5,
    EngineKeyCode_Numpad6,
    EngineKeyCode_Numpad7,
    EngineKeyCode_Numpad8,
    EngineKeyCode_Numpad9,
    EngineKeyCode_NumpadAdd,
    EngineKeyCode_NumpadSubtract,
    EngineKeyCode_NumpadMultiply,
    EngineKeyCode_NumpadDivide,
    EngineKeyCode_NumpadDecimal,
    EngineKeyCode_NumpadEnter,

    // Unknown / unmapped
    EngineKeyCode_Unknown,

};

enum EngineMouseCode : uint32_t
{
    // Mouse
    EngineMouseCode_MouseLeft,
    EngineMouseCode_MouseRight,
    EngineMouseCode_MouseMiddle,
    EngineMouseCode_MouseButton4,
    EngineMouseCode_MouseButton5,
    EngineMouseCode_MouseButton6,
    EngineMouseCode_MouseButton7,
    EngineMouseCode_MouseButton8,

    EngineMouseCode_Unknown,

};

enum EngineControllerIndex : uint32_t
{
    ControllerIndex0   = 0,
    ControllerIndex1   = 1,
    ControllerIndex2   = 2,
    ControllerIndex3   = 3,

    ControllerIndex_Primary  = 0,
    ControllerIndex_End = 4,
};

enum EngineControllerCode : uint32_t
{
    EngineControllerCode_A,
    EngineControllerCode_B,
    EngineControllerCode_X,
    EngineControllerCode_Y,
    EngineControllerCode_LT,
    EngineControllerCode_RT,
    EngineControllerCode_LSB,
    EngineControllerCode_RSB,
    EngineControllerCode_Start,
    EngineControllerCode_Select,
    EngineControllerCode_DirLeft,
    EngineControllerCode_DirRight,
    EngineControllerCode_DirUp,
    EngineControllerCode_DirDown,
    EngineControllerCode_LeftAnalog,
    EngineControllerCode_RightAnalog,
    EngineControllerCode_LeftTrigger,
    EngineControllerCode_RightTrigger,

    EngineControllerCode_Unknown,

};