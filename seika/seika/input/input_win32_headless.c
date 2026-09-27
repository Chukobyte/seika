#include "input_win32.h"

#if defined(PLATFORM_WINDOWS) && SKA_INPUT

#include <windows.h>

#include "seika/window/window.h"

// Named (non alpha-numeric) VK codes, mirroring the cases handled by ska_input_win32_vkcode_to_keycode.
static const uint32 namedVkCodes[] = {
    VK_TAB, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_NEXT, VK_PRIOR, VK_HOME, VK_END,
    VK_INSERT, VK_DELETE, VK_BACK, VK_SPACE, VK_RETURN, VK_ESCAPE,
    VK_OEM_7, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_2, VK_OEM_1, VK_OEM_PLUS,
    VK_OEM_4, VK_OEM_6, VK_OEM_5, VK_OEM_3,
    VK_CAPITAL, VK_SCROLL, VK_NUMLOCK, VK_SNAPSHOT, VK_PAUSE,
    VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4,
    VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
    VK_DECIMAL, VK_DIVIDE, VK_MULTIPLY, VK_SUBTRACT, VK_ADD,
    VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL, VK_LMENU, VK_RMENU,
    VK_LWIN, VK_RWIN, VK_APPS,
    VK_BROWSER_FORWARD, VK_BROWSER_BACK,
    VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
    VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20, VK_F21, VK_F22, VK_F23, VK_F24,
};

#define ALPHA_COUNT ((usize)26)
#define DIGIT_COUNT ((usize)10)
#define NAMED_VK_CODE_COUNT (sizeof(namedVkCodes) / sizeof(namedVkCodes[0]))
#define TRACKED_KEY_COUNT (ALPHA_COUNT + DIGIT_COUNT + NAMED_VK_CODE_COUNT)

static bool prev_key_down[TRACKED_KEY_COUNT] = {0};

static uint32 tracked_index_to_vkcode(usize index) {
    if (index < ALPHA_COUNT) {
        return (uint32)('A' + index);
    }
    index -= ALPHA_COUNT;
    if (index < DIGIT_COUNT) {
        return (uint32)('0' + index);
    }
    index -= DIGIT_COUNT;
    return namedVkCodes[index];
}

// Polls physical keyboard state directly via GetAsyncKeyState and feeds transitions into the
// same input state machine the Win32 window message pump uses. Lets keyboard input work for
// apps that never create a seika window (WindowProc otherwise never runs, so key state would
// stay permanently stale).
void ska_input_poll_headless_keyboard(void) {
    if (ska_win_get_primary_window()) return; // a real window handles input via WM_KEYDOWN/KEYUP instead

    for (usize i = 0; i < TRACKED_KEY_COUNT; i++) {
      const uint32 vk_code = tracked_index_to_vkcode(i);
      const bool is_down = (GetAsyncKeyState((int)vk_code) & 0x8000) != 0;
      if (is_down == prev_key_down[i]) {
        continue;
      }
      prev_key_down[i] = is_down;
      const SkaInputKey translated_key =
          ska_input_win32_vkcode_to_keycode(vk_code);
      ska_input_register_input_event3(
          SkaInputSourceType_KEYBOARD, translated_key,
          is_down ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED);
    }
}

#endif // #if defined(PLATFORM_WINDOWS) && SKA_INPUT
