#pragma once

#include <seika/defines.h>

#if defined(PLATFORM_WINDOWS)

#include <seika/assert.h>
#include <seika/input/input.h>

SkaInputKey ska_input_win32_vkcode_to_keycode(uint32 vkcode) {
    // First check letters
    if (vkcode >= 'A' && vkcode <= 'Z') {
        return (SkaInputKey)(SkaInputKey_KEYBOARD_A + (vkcode - 'A'));
    }
    // Next numbers
    if (vkcode >= '0' && vkcode <= '9') {
        return (SkaInputKey)(SkaInputKey_KEYBOARD_NUM_0 + (vkcode - '0'));
    }
    switch (vkcode)
    {
        // Navigation
        case VK_TAB:       return SkaInputKey_KEYBOARD_TAB;
        case VK_LEFT:      return SkaInputKey_KEYBOARD_LEFT;
        case VK_RIGHT:     return SkaInputKey_KEYBOARD_RIGHT;
        case VK_UP:        return SkaInputKey_KEYBOARD_UP;
        case VK_DOWN:      return SkaInputKey_KEYBOARD_DOWN;
        case VK_NEXT:      return SkaInputKey_KEYBOARD_PAGE_DOWN; // PageDown
        case VK_PRIOR:     return SkaInputKey_KEYBOARD_PAGE_UP;   // PageUp
        case VK_HOME:      return SkaInputKey_KEYBOARD_HOME;
        case VK_END:       return SkaInputKey_KEYBOARD_END;
        case VK_INSERT:    return SkaInputKey_KEYBOARD_INSERT;
        case VK_DELETE:    return SkaInputKey_KEYBOARD_DELETE;
        case VK_BACK:      return SkaInputKey_KEYBOARD_BACKSPACE;
        case VK_SPACE:     return SkaInputKey_KEYBOARD_SPACE;
        case VK_RETURN:    return SkaInputKey_KEYBOARD_RETURN;
        case VK_ESCAPE:    return SkaInputKey_KEYBOARD_ESCAPE;
        // Punctuation / OEM keys
        case VK_OEM_7:     return SkaInputKey_KEYBOARD_QUOTE;      // ' or "
        case VK_OEM_COMMA: return SkaInputKey_KEYBOARD_COMMA;      // ,
        case VK_OEM_MINUS: return SkaInputKey_KEYBOARD_MINUS;      // -
        case VK_OEM_PERIOD:return SkaInputKey_KEYBOARD_PERIOD;     // .
        case VK_OEM_2:     return SkaInputKey_KEYBOARD_SLASH;      // /
        case VK_OEM_1:     return SkaInputKey_KEYBOARD_SEMICOLON;  // ;
        case VK_OEM_PLUS:  return SkaInputKey_KEYBOARD_EQUALS;     // =
        case VK_OEM_4:     return SkaInputKey_KEYBOARD_LEFT_BRACKET;  // [
        case VK_OEM_6:     return SkaInputKey_KEYBOARD_RIGHT_BRACKET; // ]
        case VK_OEM_5:     return SkaInputKey_KEYBOARD_BACKSLASH;  // '\'
        case VK_OEM_3:     return SkaInputKey_KEYBOARD_BACKQUOTE;  // ` (grave accent)
        // Lock keys / special
        case VK_CAPITAL:   return SkaInputKey_KEYBOARD_CAPS_LOCK;
        case VK_SCROLL:    return SkaInputKey_KEYBOARD_SCROLL_LOCK;
        case VK_NUMLOCK:   return SkaInputKey_KEYBOARD_NUM_LOCK_CLEAR;
        case VK_SNAPSHOT:  return SkaInputKey_KEYBOARD_PRINT_SCREEN;
        case VK_PAUSE:     return SkaInputKey_KEYBOARD_PAUSE;
        // Numpad
        case VK_NUMPAD0:   return SkaInputKey_KEYBOARD_KEYPAD_0;
        case VK_NUMPAD1:   return SkaInputKey_KEYBOARD_KEYPAD_1;
        case VK_NUMPAD2:   return SkaInputKey_KEYBOARD_KEYPAD_2;
        case VK_NUMPAD3:   return SkaInputKey_KEYBOARD_KEYPAD_3;
        case VK_NUMPAD4:   return SkaInputKey_KEYBOARD_KEYPAD_4;
        case VK_NUMPAD5:   return SkaInputKey_KEYBOARD_KEYPAD_5;
        case VK_NUMPAD6:   return SkaInputKey_KEYBOARD_KEYPAD_6;
        case VK_NUMPAD7:   return SkaInputKey_KEYBOARD_KEYPAD_7;
        case VK_NUMPAD8:   return SkaInputKey_KEYBOARD_KEYPAD_8;
        case VK_NUMPAD9:   return SkaInputKey_KEYBOARD_KEYPAD_9;
        case VK_DECIMAL:   return SkaInputKey_KEYBOARD_KEYPAD_PERIOD;
        case VK_DIVIDE:    return SkaInputKey_KEYBOARD_KEYPAD_DIVIDE;
        case VK_MULTIPLY:  return SkaInputKey_KEYBOARD_KEYPAD_MULTIPLY;
        case VK_SUBTRACT:  return SkaInputKey_KEYBOARD_KEYPAD_MINUS;
        case VK_ADD:       return SkaInputKey_KEYBOARD_KEYPAD_PLUS;
        case VK_LSHIFT:    return SkaInputKey_KEYBOARD_LEFT_SHIFT;
        case VK_RSHIFT:    return SkaInputKey_KEYBOARD_RIGHT_SHIFT;
        case VK_LCONTROL:  return SkaInputKey_KEYBOARD_LEFT_CONTROL;
        case VK_RCONTROL:  return SkaInputKey_KEYBOARD_RIGHT_CONTROL;
        case VK_LMENU:     return SkaInputKey_KEYBOARD_LEFT_ALT;   // "menu" is Alt
        case VK_RMENU:     return SkaInputKey_KEYBOARD_RIGHT_ALT;
        // Win / GUI
        case VK_LWIN:      return SkaInputKey_KEYBOARD_LEFT_GUI;
        case VK_RWIN:      return SkaInputKey_KEYBOARD_RIGHT_GUI;
        case VK_APPS:      return SkaInputKey_KEYBOARD_APPLICATION;
        // Browser or "App" keys
        case VK_BROWSER_FORWARD: return SkaInputKey_KEYBOARD_APP_FORWARD;
        case VK_BROWSER_BACK:    return SkaInputKey_KEYBOARD_APP_BACK;
        // Function keys (F1 .. F24)
        case VK_F1:  return SkaInputKey_KEYBOARD_F1;
        case VK_F2:  return SkaInputKey_KEYBOARD_F2;
        case VK_F3:  return SkaInputKey_KEYBOARD_F3;
        case VK_F4:  return SkaInputKey_KEYBOARD_F4;
        case VK_F5:  return SkaInputKey_KEYBOARD_F5;
        case VK_F6:  return SkaInputKey_KEYBOARD_F6;
        case VK_F7:  return SkaInputKey_KEYBOARD_F7;
        case VK_F8:  return SkaInputKey_KEYBOARD_F8;
        case VK_F9:  return SkaInputKey_KEYBOARD_F9;
        case VK_F10: return SkaInputKey_KEYBOARD_F10;
        case VK_F11: return SkaInputKey_KEYBOARD_F11;
        case VK_F12: return SkaInputKey_KEYBOARD_F12;
        case VK_F13: return SkaInputKey_KEYBOARD_F13;
        case VK_F14: return SkaInputKey_KEYBOARD_F14;
        case VK_F15: return SkaInputKey_KEYBOARD_F15;
        case VK_F16: return SkaInputKey_KEYBOARD_F16;
        case VK_F17: return SkaInputKey_KEYBOARD_F17;
        case VK_F18: return SkaInputKey_KEYBOARD_F18;
        case VK_F19: return SkaInputKey_KEYBOARD_F19;
        case VK_F20: return SkaInputKey_KEYBOARD_F20;
        case VK_F21: return SkaInputKey_KEYBOARD_F21;
        case VK_F22: return SkaInputKey_KEYBOARD_F22;
        case VK_F23: return SkaInputKey_KEYBOARD_F23;
        case VK_F24: return SkaInputKey_KEYBOARD_F24;

        default:
            ska_logger_warn("Unsupported vkCode = %u", vkcode);
            return SkaInputKey_INVALID;
    }
}

#endif // #if defined(PLATFORM_WINDOWS)
