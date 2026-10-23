#include "input_directinput.h"

#if defined(PLATFORM_WINDOWS) && SKA_INPUT

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "seika/logger.h"
#include "seika/input/input.h"
#include "seika/input/gamepad_db.h"

#define SKA_DINPUT_MAX_AXES 8
// [-1000, 1000] range configured on every axis object via DIPROP_RANGE, so raw values normalize
// to [-1, 1] with a single fixed divisor.
#define SKA_DINPUT_AXIS_RANGE 1000
#define SKA_DINPUT_STICK_DEADZONE 0.15f
// Smaller deadzone for POSITIVE_HALF/NEGATIVE_HALF-modified axes (triggers): their post-modifier
// range is already [0, 1], not [-1, 1], so a proportionally smaller deadzone is appropriate.
#define SKA_DINPUT_TRIGGER_DEADZONE 0.05f
// Re-enumerate for hot-plug changes at most this often; EnumDevices isn't free enough to call
// every frame.
#define SKA_DINPUT_REENUMERATE_INTERVAL_MS 1000

typedef struct SkaDirectInputGamepad {
    bool connected;
    LPDIRECTINPUTDEVICE8 device;
    GUID di_instance_guid;              // DirectInput's own per-connection instance id
    const SkaGamepadMapping* mapping;   // resolved once at connect time; NULL if unmapped
    DWORD axis_field_offset[SKA_DINPUT_MAX_AXES]; // compacted aN -> DIJOYSTATE2 byte offset (DIJOFS_*)
    uint8 axis_count;
    DIJOYSTATE2 prev_state;
    bool axis_was_active[SKA_DINPUT_MAX_AXES]; // deadzone hysteresis, per compacted axis slot
} SkaDirectInputGamepad;

static LPDIRECTINPUT8 g_dinput = NULL;
static SkaDirectInputGamepad g_gamepads[SKA_INPUT_MAX_DEVICES];
static DWORD g_last_enumerate_tick = 0;
static HWND g_hwnd = NULL; // set on every ska_input_directinput_enumerate() call; reused for periodic re-enumeration

// Scratch state passed through IDirectInputDevice8_EnumObjects while setting up a newly found
// device -- collects axis offsets so they can be sorted/compacted after enumeration completes.
typedef struct SkaDinputEnumObjectsContext {
    LPDIRECTINPUTDEVICE8 device;
    DWORD axis_offsets[SKA_DINPUT_MAX_AXES];
    uint8 axis_count;
} SkaDinputEnumObjectsContext;

static BOOL CALLBACK enum_objects_callback(const DIDEVICEOBJECTINSTANCE* obj, VOID* context_ptr) {
    SkaDinputEnumObjectsContext* context = (SkaDinputEnumObjectsContext*)context_ptr;
    if ((obj->dwType & DIDFT_AXIS) != 0) {
        DIPROPRANGE range = {0};
        range.diph.dwSize = sizeof(DIPROPRANGE);
        range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        range.diph.dwObj = obj->dwType;
        range.diph.dwHow = DIPH_BYID;
        range.lMin = -SKA_DINPUT_AXIS_RANGE;
        range.lMax = SKA_DINPUT_AXIS_RANGE;
        IDirectInputDevice8_SetProperty(context->device, DIPROP_RANGE, &range.diph);

        if (context->axis_count < SKA_DINPUT_MAX_AXES) {
            // obj->dwOfs follows SetDataFormat(&c_dfDIJoystick2)'s standard DIJOFS_* slot layout,
            // which is the order gamecontrollerdb.txt's Windows entries assume once sorted/compacted.
            context->axis_offsets[context->axis_count++] = obj->dwOfs;
        }
    }
    return DIENUM_CONTINUE;
}

// Ascending insertion sort over the (at most 8) collected axis offsets -- this is the "sort by
// DIJOFS" half of SDL's enumerate-sort-compact algorithm; compaction falls out for free since we
// only ever collected offsets for axes this device actually has.
static void sort_axis_offsets(DWORD* offsets, uint8 count) {
    for (uint8 i = 1; i < count; i++) {
        const DWORD key = offsets[i];
        int j = (int)i - 1;
        while (j >= 0 && offsets[j] > key) {
            offsets[j + 1] = offsets[j];
            j--;
        }
        offsets[j + 1] = key;
    }
}

// Builds an SDL-style 16-byte joystick GUID (32 lowercase hex chars) from VID/PID: bus=3 (USB),
// crc=0, version=0, driver_sig/data=0 -- matches most real gamecontrollerdb.txt Windows entries.
// Devices without a real VID/PID won't match any DB entry (documented gap, not a crash risk).
static void build_sdl_guid_string(LPDIRECTINPUTDEVICE8 device, char out_guid[33]) {
    DIPROPDWORD prop = {0};
    prop.diph.dwSize = sizeof(DIPROPDWORD);
    prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    prop.diph.dwObj = 0;
    prop.diph.dwHow = DIPH_DEVICE;

    uint16 vendor = 0;
    uint16 product = 0;
    if (SUCCEEDED(IDirectInputDevice8_GetProperty(device, DIPROP_VIDPID, &prop.diph))) {
        vendor = (uint16)LOWORD(prop.dwData);
        product = (uint16)HIWORD(prop.dwData);
    }
    const uint16 bus = (vendor != 0 && product != 0) ? 3 /* USB */ : 0;

    uint8 bytes[16] = {0};
    memcpy(&bytes[0], &bus, sizeof(uint16));
    // bytes[2..4): crc, left 0 (no name-hash fallback implemented)
    memcpy(&bytes[4], &vendor, sizeof(uint16));
    // bytes[6..8): zero pad
    memcpy(&bytes[8], &product, sizeof(uint16));
    // bytes[10..16): zero pad + version(0) + driver_sig/data(0)

    for (int i = 0; i < 16; i++) {
        snprintf(out_guid + i * 2, 3, "%02x", bytes[i]);
    }
}

static SkaInputDeviceIndex find_free_slot(void) {
    for (SkaInputDeviceIndex i = 0; i < SKA_INPUT_MAX_DEVICES; i++) {
        if (!g_gamepads[i].connected) {
            return i;
        }
    }
    return -1;
}

static SkaInputDeviceIndex find_slot_by_instance(const GUID* instance_guid) {
    for (SkaInputDeviceIndex i = 0; i < SKA_INPUT_MAX_DEVICES; i++) {
        if (g_gamepads[i].connected && IsEqualGUID(&g_gamepads[i].di_instance_guid, instance_guid)) {
            return i;
        }
    }
    return -1;
}

static void release_slot(SkaInputDeviceIndex slot) {
    SkaDirectInputGamepad* gamepad = &g_gamepads[slot];
    if (gamepad->device) {
        IDirectInputDevice8_Unacquire(gamepad->device);
        IDirectInputDevice8_Release(gamepad->device);
    }
    *gamepad = (SkaDirectInputGamepad){0};
    ska_input_register_gamepad_removed_event(slot);
}

typedef struct SkaDinputEnumDevicesContext {
    HWND hwnd;
    bool seen_this_pass[SKA_INPUT_MAX_DEVICES];
} SkaDinputEnumDevicesContext;

static BOOL CALLBACK enum_devices_callback(const DIDEVICEINSTANCE* instance, VOID* context_ptr) {
    SkaDinputEnumDevicesContext* context = (SkaDinputEnumDevicesContext*)context_ptr;

    const SkaInputDeviceIndex existing_slot = find_slot_by_instance(&instance->guidInstance);
    if (existing_slot >= 0) {
        context->seen_this_pass[existing_slot] = true;
        return DIENUM_CONTINUE;
    }

    const SkaInputDeviceIndex slot = find_free_slot();
    if (slot < 0) {
        return DIENUM_CONTINUE; // already tracking SKA_INPUT_MAX_DEVICES gamepads
    }

    LPDIRECTINPUTDEVICE8 device = NULL;
    if (FAILED(IDirectInput8_CreateDevice(g_dinput, &instance->guidInstance, &device, NULL))) {
        return DIENUM_CONTINUE;
    }
    if (FAILED(IDirectInputDevice8_SetDataFormat(device, &c_dfDIJoystick2))) {
        IDirectInputDevice8_Release(device);
        return DIENUM_CONTINUE;
    }
    // Background + non-exclusive: gamepad input keeps working when the window loses focus or
    // another app also has the device open, matching typical game conventions.
    IDirectInputDevice8_SetCooperativeLevel(device, context->hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    SkaDinputEnumObjectsContext obj_context = {.device = device};
    IDirectInputDevice8_EnumObjects(device, enum_objects_callback, &obj_context, DIDFT_ALL);
    sort_axis_offsets(obj_context.axis_offsets, obj_context.axis_count);

    char guid_string[33];
    build_sdl_guid_string(device, guid_string);

    SkaDirectInputGamepad* gamepad = &g_gamepads[slot];
    *gamepad = (SkaDirectInputGamepad){0};
    gamepad->connected = true;
    gamepad->device = device;
    gamepad->di_instance_guid = instance->guidInstance;
    gamepad->mapping = ska_gamepad_db_find_by_guid(guid_string);
    gamepad->axis_count = obj_context.axis_count;
    memcpy(gamepad->axis_field_offset, obj_context.axis_offsets, sizeof(DWORD) * obj_context.axis_count);

    if (!gamepad->mapping) {
        ska_logger_warn("DirectInput gamepad '%s' (guid=%s) has no gamepad_db mapping -- it will "
            "be tracked for connect/disconnect only, no button/axis input will be translated.",
            instance->tszProductName, guid_string);
    }

    IDirectInputDevice8_Acquire(device);
    context->seen_this_pass[slot] = true;
    ska_input_register_gamepad_added_event(slot);

    return DIENUM_CONTINUE;
}

bool ska_input_directinput_init(HINSTANCE h_instance) {
    memset(g_gamepads, 0, sizeof(g_gamepads));
    const HRESULT hr = DirectInput8Create(h_instance, DIRECTINPUT_VERSION, &IID_IDirectInput8,
        (void**)&g_dinput, NULL);
    if (FAILED(hr)) {
        ska_logger_error("DirectInput8Create failed (hr=0x%lx)", (unsigned long)hr);
        return false;
    }
    return true;
}

void ska_input_directinput_enumerate(HWND hwnd) {
    if (!g_dinput) {
        return;
    }
    g_hwnd = hwnd;

    SkaDinputEnumDevicesContext context = {.hwnd = hwnd};
    IDirectInput8_EnumDevices(g_dinput, DI8DEVCLASS_GAMECTRL, enum_devices_callback, &context, DIEDFL_ATTACHEDONLY);

    for (SkaInputDeviceIndex i = 0; i < SKA_INPUT_MAX_DEVICES; i++) {
        if (g_gamepads[i].connected && !context.seen_this_pass[i]) {
            release_slot(i);
        }
    }

    g_last_enumerate_tick = GetTickCount();
}

static void poll_one_gamepad(SkaInputDeviceIndex slot) {
    SkaDirectInputGamepad* gamepad = &g_gamepads[slot];

    IDirectInputDevice8_Poll(gamepad->device); // harmless no-op on devices that don't need it

    DIJOYSTATE2 state;
    HRESULT hr = IDirectInputDevice8_GetDeviceState(gamepad->device, sizeof(DIJOYSTATE2), &state);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
        if (FAILED(IDirectInputDevice8_Acquire(gamepad->device)) ||
            FAILED(IDirectInputDevice8_GetDeviceState(gamepad->device, sizeof(DIJOYSTATE2), &state))) {
            release_slot(slot);
            return;
        }
    } else if (FAILED(hr)) {
        return; // transient failure, try again next frame
    }

    if (!gamepad->mapping) {
        gamepad->prev_state = state;
        return; // unmapped controller: connect/disconnect tracked, no synthesized input events
    }

    for (uint32 i = 0; i < gamepad->mapping->entry_count; i++) {
        const SkaGamepadMappingEntry* entry = &gamepad->mapping->entries[i];
        switch (entry->source_type) {
            case SkaGamepadMappingSourceType_BUTTON: {
                if (entry->index >= 128) {
                    break;
                }
                const bool now_pressed = (state.rgbButtons[entry->index] & 0x80) != 0;
                const bool was_pressed = (gamepad->prev_state.rgbButtons[entry->index] & 0x80) != 0;
                if (now_pressed != was_pressed) {
                    ska_input_register_input_event3(SkaInputSourceType_GAMEPAD, entry->target_key,
                        now_pressed ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED);
                }
                break;
            }
            case SkaGamepadMappingSourceType_HAT: {
                if (entry->index >= 4) {
                    break;
                }
                const uint8 now_mask = ska_gamepad_pov_angle_to_hat_bitmask((i32)state.rgdwPOV[entry->index]);
                const uint8 prev_mask = ska_gamepad_pov_angle_to_hat_bitmask((i32)gamepad->prev_state.rgdwPOV[entry->index]);
                const bool now_pressed = (now_mask & entry->hat_bit) != 0;
                const bool was_pressed = (prev_mask & entry->hat_bit) != 0;
                if (now_pressed != was_pressed) {
                    ska_input_register_input_event3(SkaInputSourceType_GAMEPAD, entry->target_key,
                        now_pressed ? SkaInputTriggerType_PRESSED : SkaInputTriggerType_RELEASED);
                }
                break;
            }
            case SkaGamepadMappingSourceType_AXIS: {
                if (entry->index >= gamepad->axis_count) {
                    break;
                }
                const LONG raw = *(const LONG*)((const char*)&state + gamepad->axis_field_offset[entry->index]);
                const f32 raw_normalized = (f32)raw / (f32)SKA_DINPUT_AXIS_RANGE;
                const f32 value = ska_gamepad_apply_axis_modifier(raw_normalized, entry->axis_modifier);
                const bool is_half_axis = entry->axis_modifier == SkaGamepadAxisModifier_POSITIVE_HALF ||
                                           entry->axis_modifier == SkaGamepadAxisModifier_NEGATIVE_HALF;
                const f32 deadzone = is_half_axis ? SKA_DINPUT_TRIGGER_DEADZONE : SKA_DINPUT_STICK_DEADZONE;
                const bool is_active = fabsf(value) > deadzone;

                if (is_active) {
                    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, entry->target_key,
                        SkaInputTriggerType_AXIS_IN_MOTION, slot, value);
                } else if (gamepad->axis_was_active[entry->index]) {
                    ska_input_register_input_event(SkaInputSourceType_GAMEPAD, entry->target_key,
                        SkaInputTriggerType_AXIS_STOPPED_MOTION, slot, 0.0f);
                }
                gamepad->axis_was_active[entry->index] = is_active;
                break;
            }
        }
    }

    gamepad->prev_state = state;
}

void ska_input_directinput_poll_gamepads(void) {
    if (!g_dinput) {
        return;
    }

    if (g_hwnd && GetTickCount() - g_last_enumerate_tick >= SKA_DINPUT_REENUMERATE_INTERVAL_MS) {
        ska_input_directinput_enumerate(g_hwnd);
    }

    for (SkaInputDeviceIndex i = 0; i < SKA_INPUT_MAX_DEVICES; i++) {
        if (g_gamepads[i].connected) {
            poll_one_gamepad(i);
        }
    }
}

void ska_input_directinput_shutdown(void) {
    for (SkaInputDeviceIndex i = 0; i < SKA_INPUT_MAX_DEVICES; i++) {
        if (g_gamepads[i].connected) {
            release_slot(i);
        }
    }
    if (g_dinput) {
        IDirectInput8_Release(g_dinput);
        g_dinput = NULL;
    }
}

#endif // #if defined(PLATFORM_WINDOWS) && SKA_INPUT
