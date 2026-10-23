#include "input_cocoa.h"

#if defined(PLATFORM_MACOS) && SKA_INPUT

// Raw macOS virtual keycode values (NSEvent.keyCode / kVK_* in Carbon's HIToolbox headers) --
// used as integer literals instead of #include <Carbon/Carbon.h> + the kVK_* names, so this file
// doesn't need Carbon.framework linked just for these stable, decades-unchanged constants. These
// are hardware-position codes, not layout-resolved characters -- see the header's comment.
SkaInputKey ska_input_cocoa_keycode_to_keycode(unsigned short mac_virtual_keycode) {
    switch (mac_virtual_keycode) {
        case 0x00: return SkaInputKey_KEYBOARD_A;
        case 0x01: return SkaInputKey_KEYBOARD_S;
        case 0x02: return SkaInputKey_KEYBOARD_D;
        case 0x03: return SkaInputKey_KEYBOARD_F;
        case 0x04: return SkaInputKey_KEYBOARD_H;
        case 0x05: return SkaInputKey_KEYBOARD_G;
        case 0x06: return SkaInputKey_KEYBOARD_Z;
        case 0x07: return SkaInputKey_KEYBOARD_X;
        case 0x08: return SkaInputKey_KEYBOARD_C;
        case 0x09: return SkaInputKey_KEYBOARD_V;
        case 0x0B: return SkaInputKey_KEYBOARD_B;
        case 0x0C: return SkaInputKey_KEYBOARD_Q;
        case 0x0D: return SkaInputKey_KEYBOARD_W;
        case 0x0E: return SkaInputKey_KEYBOARD_E;
        case 0x0F: return SkaInputKey_KEYBOARD_R;
        case 0x10: return SkaInputKey_KEYBOARD_Y;
        case 0x11: return SkaInputKey_KEYBOARD_T;
        case 0x12: return SkaInputKey_KEYBOARD_NUM_1;
        case 0x13: return SkaInputKey_KEYBOARD_NUM_2;
        case 0x14: return SkaInputKey_KEYBOARD_NUM_3;
        case 0x15: return SkaInputKey_KEYBOARD_NUM_4;
        case 0x16: return SkaInputKey_KEYBOARD_NUM_6;
        case 0x17: return SkaInputKey_KEYBOARD_NUM_5;
        case 0x18: return SkaInputKey_KEYBOARD_EQUALS;
        case 0x19: return SkaInputKey_KEYBOARD_NUM_9;
        case 0x1A: return SkaInputKey_KEYBOARD_NUM_7;
        case 0x1B: return SkaInputKey_KEYBOARD_MINUS;
        case 0x1C: return SkaInputKey_KEYBOARD_NUM_8;
        case 0x1D: return SkaInputKey_KEYBOARD_NUM_0;
        case 0x1E: return SkaInputKey_KEYBOARD_RIGHT_BRACKET;
        case 0x1F: return SkaInputKey_KEYBOARD_O;
        case 0x20: return SkaInputKey_KEYBOARD_U;
        case 0x21: return SkaInputKey_KEYBOARD_LEFT_BRACKET;
        case 0x22: return SkaInputKey_KEYBOARD_I;
        case 0x23: return SkaInputKey_KEYBOARD_P;
        case 0x24: return SkaInputKey_KEYBOARD_RETURN;
        case 0x25: return SkaInputKey_KEYBOARD_L;
        case 0x26: return SkaInputKey_KEYBOARD_J;
        case 0x27: return SkaInputKey_KEYBOARD_QUOTE;
        case 0x28: return SkaInputKey_KEYBOARD_K;
        case 0x29: return SkaInputKey_KEYBOARD_SEMICOLON;
        case 0x2A: return SkaInputKey_KEYBOARD_BACKSLASH;
        case 0x2B: return SkaInputKey_KEYBOARD_COMMA;
        case 0x2C: return SkaInputKey_KEYBOARD_SLASH;
        case 0x2D: return SkaInputKey_KEYBOARD_N;
        case 0x2E: return SkaInputKey_KEYBOARD_M;
        case 0x2F: return SkaInputKey_KEYBOARD_PERIOD;
        case 0x30: return SkaInputKey_KEYBOARD_TAB;
        case 0x31: return SkaInputKey_KEYBOARD_SPACE;
        case 0x32: return SkaInputKey_KEYBOARD_BACKQUOTE;
        case 0x33: return SkaInputKey_KEYBOARD_BACKSPACE;
        case 0x35: return SkaInputKey_KEYBOARD_ESCAPE;
        case 0x37: return SkaInputKey_KEYBOARD_LEFT_GUI;
        case 0x38: return SkaInputKey_KEYBOARD_LEFT_SHIFT;
        case 0x39: return SkaInputKey_KEYBOARD_CAPS_LOCK;
        case 0x3A: return SkaInputKey_KEYBOARD_LEFT_ALT;
        case 0x3B: return SkaInputKey_KEYBOARD_LEFT_CONTROL;
        case 0x3C: return SkaInputKey_KEYBOARD_RIGHT_SHIFT;
        case 0x3D: return SkaInputKey_KEYBOARD_RIGHT_ALT;
        case 0x3E: return SkaInputKey_KEYBOARD_RIGHT_CONTROL;
        case 0x41: return SkaInputKey_KEYBOARD_KEYPAD_PERIOD;
        case 0x43: return SkaInputKey_KEYBOARD_KEYPAD_MULTIPLY;
        case 0x45: return SkaInputKey_KEYBOARD_KEYPAD_PLUS;
        case 0x47: return SkaInputKey_KEYBOARD_NUM_LOCK_CLEAR;
        case 0x4B: return SkaInputKey_KEYBOARD_KEYPAD_DIVIDE;
        case 0x4C: return SkaInputKey_KEYBOARD_KEYPAD_ENTER;
        case 0x4E: return SkaInputKey_KEYBOARD_KEYPAD_MINUS;
        case 0x51: return SkaInputKey_KEYBOARD_KEYPAD_EQUALS;
        case 0x52: return SkaInputKey_KEYBOARD_KEYPAD_0;
        case 0x53: return SkaInputKey_KEYBOARD_KEYPAD_1;
        case 0x54: return SkaInputKey_KEYBOARD_KEYPAD_2;
        case 0x55: return SkaInputKey_KEYBOARD_KEYPAD_3;
        case 0x56: return SkaInputKey_KEYBOARD_KEYPAD_4;
        case 0x57: return SkaInputKey_KEYBOARD_KEYPAD_5;
        case 0x58: return SkaInputKey_KEYBOARD_KEYPAD_6;
        case 0x59: return SkaInputKey_KEYBOARD_KEYPAD_7;
        case 0x5B: return SkaInputKey_KEYBOARD_KEYPAD_8;
        case 0x5C: return SkaInputKey_KEYBOARD_KEYPAD_9;
        case 0x60: return SkaInputKey_KEYBOARD_F5;
        case 0x61: return SkaInputKey_KEYBOARD_F6;
        case 0x62: return SkaInputKey_KEYBOARD_F7;
        case 0x63: return SkaInputKey_KEYBOARD_F3;
        case 0x64: return SkaInputKey_KEYBOARD_F8;
        case 0x65: return SkaInputKey_KEYBOARD_F9;
        case 0x67: return SkaInputKey_KEYBOARD_F11;
        case 0x69: return SkaInputKey_KEYBOARD_F13;
        case 0x6A: return SkaInputKey_KEYBOARD_F16;
        case 0x6B: return SkaInputKey_KEYBOARD_F14;
        case 0x6D: return SkaInputKey_KEYBOARD_F10;
        case 0x6F: return SkaInputKey_KEYBOARD_F12;
        case 0x71: return SkaInputKey_KEYBOARD_F15;
        case 0x72: return SkaInputKey_KEYBOARD_INSERT;
        case 0x73: return SkaInputKey_KEYBOARD_HOME;
        case 0x74: return SkaInputKey_KEYBOARD_PAGE_UP;
        case 0x75: return SkaInputKey_KEYBOARD_DELETE;
        case 0x76: return SkaInputKey_KEYBOARD_F4;
        case 0x77: return SkaInputKey_KEYBOARD_END;
        case 0x78: return SkaInputKey_KEYBOARD_F2;
        case 0x79: return SkaInputKey_KEYBOARD_PAGE_DOWN;
        case 0x7A: return SkaInputKey_KEYBOARD_F1;
        case 0x7B: return SkaInputKey_KEYBOARD_LEFT;
        case 0x7C: return SkaInputKey_KEYBOARD_RIGHT;
        case 0x7D: return SkaInputKey_KEYBOARD_DOWN;
        case 0x7E: return SkaInputKey_KEYBOARD_UP;
        default: return SkaInputKey_INVALID;
    }
}

#endif // #if defined(PLATFORM_MACOS) && SKA_INPUT
