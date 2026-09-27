#pragma once

#include <seika/defines.h>

#if SKA_INPUT

#include <seika/input/input.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SkaGamepadMappingSourceType {
    SkaGamepadMappingSourceType_BUTTON,
    SkaGamepadMappingSourceType_AXIS,
    SkaGamepadMappingSourceType_HAT,
} SkaGamepadMappingSourceType;

// Only meaningful when source_type == SkaGamepadMappingSourceType_AXIS. Mirrors
// gamecontrollerdb.txt's 'aN' (NONE), 'aN~' (INVERTED), '+aN' (POSITIVE_HALF), '-aN' (NEGATIVE_HALF).
typedef enum SkaGamepadAxisModifier {
    SkaGamepadAxisModifier_NONE,
    SkaGamepadAxisModifier_INVERTED,
    SkaGamepadAxisModifier_POSITIVE_HALF,
    SkaGamepadAxisModifier_NEGATIVE_HALF,
} SkaGamepadAxisModifier;

typedef struct SkaGamepadMappingEntry {
    SkaInputKey target_key; // one of the SkaInputKey_GAMEPAD_* values
    SkaGamepadMappingSourceType source_type;
    uint8 index; // button index, axis index, or hat index depending on source_type
    SkaGamepadAxisModifier axis_modifier; // meaningful only when source_type == AXIS
    uint8 hat_bit; // meaningful only when source_type == HAT (1 = up, 2 = right, 4 = down, 8 = left)
} SkaGamepadMappingEntry;

typedef struct SkaGamepadMapping {
    const char* guid;
    const char* name;
    const SkaGamepadMappingEntry* entries;
    uint32 entry_count;
} SkaGamepadMapping;

// Looks up a controller's button/axis mapping by its GUID (exact, case-sensitive match --
// gamecontrollerdb.txt GUIDs are consistently lowercase hex). Returns NULL if not found.
const SkaGamepadMapping* ska_gamepad_db_find_by_guid(const char* guid);

// The full mapping table compiled in for the current platform (empty on platforms with no
// generated data). Entries are sorted by guid.
const SkaGamepadMapping* ska_gamepad_db_get_table(void);
usize ska_gamepad_db_get_table_count(void);

// Applies a mapping entry's axis modifier to a raw axis value already normalized to [-1, 1].
// NONE: passthrough. INVERTED: negate. POSITIVE_HALF/NEGATIVE_HALF: take that half of the raw
// excursion, remapped to [0, 1] (used for controllers reporting both triggers on one combined
// physical axis, e.g. gamecontrollerdb.txt's "+a2"/"-a2"). Platform-agnostic pure math, reusable
// by any backend (DirectInput, evdev, IOKit HID, ...).
f32 ska_gamepad_apply_axis_modifier(f32 raw_normalized, SkaGamepadAxisModifier modifier);

// Converts a DirectInput-style POV hat angle (hundredths of a degree, 0 = up/north, clockwise;
// negative = centered/no direction) into SDL's up/right/down/left bitmask (1/2/4/8, OR'd for
// diagonals) matching gamecontrollerdb.txt's h0.N encoding.
uint8 ska_gamepad_pov_angle_to_hat_bitmask(i32 pov_angle_hundredths_of_degree);

#ifdef __cplusplus
}
#endif

#endif // #if SKA_INPUT
