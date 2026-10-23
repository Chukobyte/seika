#include "input_x11.h"

// !PLATFORM_EMSCRIPTEN exclusion: see the matching comment in window_x11.c.
#if defined(PLATFORM_LINUX) && !defined(PLATFORM_EMSCRIPTEN) && SKA_INPUT

#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h> // for XkbKeycodeToKeysym

#include <seika/logger.h>

// Resolves the group-0/level-0 (unshifted) keysym for a hardware keycode via Xkb. Unlike raw
// X11 hardware keycodes (which, like macOS virtual keycodes, are arbitrary per-driver scancode
// numbers), keysyms are semantic and layout-resolved by the X server's active keyboard map --
// the X11 equivalent of Win32's shift-independent VK_* codes used by ska_input_win32_vkcode_to_keycode.
SkaInputKey ska_input_x11_keysym_to_keycode(Display* display, unsigned int hardware_keycode) {
    const KeySym keysym = XkbKeycodeToKeysym(display, (KeyCode)hardware_keycode, 0, 0);

    // Letters
    if (keysym >= XK_a && keysym <= XK_z) {
        return (SkaInputKey)(SkaInputKey_KEYBOARD_A + (keysym - XK_a));
    }
    if (keysym >= XK_A && keysym <= XK_Z) {
        return (SkaInputKey)(SkaInputKey_KEYBOARD_A + (keysym - XK_A));
    }
    // Numbers
    if (keysym >= XK_0 && keysym <= XK_9) {
        return (SkaInputKey)(SkaInputKey_KEYBOARD_NUM_0 + (keysym - XK_0));
    }

    switch (keysym) {
        // Navigation
        case XK_Tab:       return SkaInputKey_KEYBOARD_TAB;
        case XK_Left:      return SkaInputKey_KEYBOARD_LEFT;
        case XK_Right:     return SkaInputKey_KEYBOARD_RIGHT;
        case XK_Up:        return SkaInputKey_KEYBOARD_UP;
        case XK_Down:      return SkaInputKey_KEYBOARD_DOWN;
        case XK_Next:      return SkaInputKey_KEYBOARD_PAGE_DOWN;
        case XK_Prior:     return SkaInputKey_KEYBOARD_PAGE_UP;
        case XK_Home:      return SkaInputKey_KEYBOARD_HOME;
        case XK_End:       return SkaInputKey_KEYBOARD_END;
        case XK_Insert:    return SkaInputKey_KEYBOARD_INSERT;
        case XK_Delete:    return SkaInputKey_KEYBOARD_DELETE;
        case XK_BackSpace: return SkaInputKey_KEYBOARD_BACKSPACE;
        case XK_space:     return SkaInputKey_KEYBOARD_SPACE;
        case XK_Return:    return SkaInputKey_KEYBOARD_RETURN;
        case XK_Escape:    return SkaInputKey_KEYBOARD_ESCAPE;
        // Punctuation
        case XK_apostrophe:   return SkaInputKey_KEYBOARD_QUOTE;
        case XK_comma:         return SkaInputKey_KEYBOARD_COMMA;
        case XK_minus:         return SkaInputKey_KEYBOARD_MINUS;
        case XK_period:        return SkaInputKey_KEYBOARD_PERIOD;
        case XK_slash:         return SkaInputKey_KEYBOARD_SLASH;
        case XK_semicolon:     return SkaInputKey_KEYBOARD_SEMICOLON;
        case XK_equal:         return SkaInputKey_KEYBOARD_EQUALS;
        case XK_bracketleft:   return SkaInputKey_KEYBOARD_LEFT_BRACKET;
        case XK_bracketright:  return SkaInputKey_KEYBOARD_RIGHT_BRACKET;
        case XK_backslash:     return SkaInputKey_KEYBOARD_BACKSLASH;
        case XK_grave:         return SkaInputKey_KEYBOARD_BACKQUOTE;
        // Lock keys / special
        case XK_Caps_Lock:    return SkaInputKey_KEYBOARD_CAPS_LOCK;
        case XK_Scroll_Lock:  return SkaInputKey_KEYBOARD_SCROLL_LOCK;
        case XK_Num_Lock:     return SkaInputKey_KEYBOARD_NUM_LOCK_CLEAR;
        case XK_Print:        return SkaInputKey_KEYBOARD_PRINT_SCREEN;
        case XK_Pause:        return SkaInputKey_KEYBOARD_PAUSE;
        // Numpad
        case XK_KP_0:         return SkaInputKey_KEYBOARD_KEYPAD_0;
        case XK_KP_1:         return SkaInputKey_KEYBOARD_KEYPAD_1;
        case XK_KP_2:         return SkaInputKey_KEYBOARD_KEYPAD_2;
        case XK_KP_3:         return SkaInputKey_KEYBOARD_KEYPAD_3;
        case XK_KP_4:         return SkaInputKey_KEYBOARD_KEYPAD_4;
        case XK_KP_5:         return SkaInputKey_KEYBOARD_KEYPAD_5;
        case XK_KP_6:         return SkaInputKey_KEYBOARD_KEYPAD_6;
        case XK_KP_7:         return SkaInputKey_KEYBOARD_KEYPAD_7;
        case XK_KP_8:         return SkaInputKey_KEYBOARD_KEYPAD_8;
        case XK_KP_9:         return SkaInputKey_KEYBOARD_KEYPAD_9;
        case XK_KP_Decimal:   return SkaInputKey_KEYBOARD_KEYPAD_PERIOD;
        case XK_KP_Divide:    return SkaInputKey_KEYBOARD_KEYPAD_DIVIDE;
        case XK_KP_Multiply:  return SkaInputKey_KEYBOARD_KEYPAD_MULTIPLY;
        case XK_KP_Subtract:  return SkaInputKey_KEYBOARD_KEYPAD_MINUS;
        case XK_KP_Add:       return SkaInputKey_KEYBOARD_KEYPAD_PLUS;
        case XK_KP_Enter:     return SkaInputKey_KEYBOARD_KEYPAD_ENTER;
        case XK_KP_Equal:     return SkaInputKey_KEYBOARD_KEYPAD_EQUALS;
        // Modifiers
        case XK_Shift_L:      return SkaInputKey_KEYBOARD_LEFT_SHIFT;
        case XK_Shift_R:      return SkaInputKey_KEYBOARD_RIGHT_SHIFT;
        case XK_Control_L:    return SkaInputKey_KEYBOARD_LEFT_CONTROL;
        case XK_Control_R:    return SkaInputKey_KEYBOARD_RIGHT_CONTROL;
        case XK_Alt_L:        return SkaInputKey_KEYBOARD_LEFT_ALT;
        case XK_Alt_R:        return SkaInputKey_KEYBOARD_RIGHT_ALT;
        case XK_Super_L:      return SkaInputKey_KEYBOARD_LEFT_GUI;
        case XK_Super_R:      return SkaInputKey_KEYBOARD_RIGHT_GUI;
        case XK_Menu:         return SkaInputKey_KEYBOARD_APPLICATION;
        // Browser / "App" keys -- not present on every X server/WM combo (XF86 vendor keysyms)
        case XF86XK_Forward:  return SkaInputKey_KEYBOARD_APP_FORWARD;
        case XF86XK_Back:     return SkaInputKey_KEYBOARD_APP_BACK;
        // Function keys (F1 .. F24)
        case XK_F1:  return SkaInputKey_KEYBOARD_F1;
        case XK_F2:  return SkaInputKey_KEYBOARD_F2;
        case XK_F3:  return SkaInputKey_KEYBOARD_F3;
        case XK_F4:  return SkaInputKey_KEYBOARD_F4;
        case XK_F5:  return SkaInputKey_KEYBOARD_F5;
        case XK_F6:  return SkaInputKey_KEYBOARD_F6;
        case XK_F7:  return SkaInputKey_KEYBOARD_F7;
        case XK_F8:  return SkaInputKey_KEYBOARD_F8;
        case XK_F9:  return SkaInputKey_KEYBOARD_F9;
        case XK_F10: return SkaInputKey_KEYBOARD_F10;
        case XK_F11: return SkaInputKey_KEYBOARD_F11;
        case XK_F12: return SkaInputKey_KEYBOARD_F12;
        case XK_F13: return SkaInputKey_KEYBOARD_F13;
        case XK_F14: return SkaInputKey_KEYBOARD_F14;
        case XK_F15: return SkaInputKey_KEYBOARD_F15;
        case XK_F16: return SkaInputKey_KEYBOARD_F16;
        case XK_F17: return SkaInputKey_KEYBOARD_F17;
        case XK_F18: return SkaInputKey_KEYBOARD_F18;
        case XK_F19: return SkaInputKey_KEYBOARD_F19;
        case XK_F20: return SkaInputKey_KEYBOARD_F20;
        case XK_F21: return SkaInputKey_KEYBOARD_F21;
        case XK_F22: return SkaInputKey_KEYBOARD_F22;
        case XK_F23: return SkaInputKey_KEYBOARD_F23;
        case XK_F24: return SkaInputKey_KEYBOARD_F24;

        default:
            ska_logger_warn("Unsupported X11 keysym = 0x%lx", (unsigned long)keysym);
            return SkaInputKey_INVALID;
    }
}

#endif // #if defined(PLATFORM_LINUX) && !defined(PLATFORM_EMSCRIPTEN) && SKA_INPUT
