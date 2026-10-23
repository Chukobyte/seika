#include "gamepad_db.h"

#if SKA_INPUT

#include <stdlib.h>
#include <string.h>

static int compare_guid(const void* key, const void* element) {
    const char* guid = (const char*)key;
    const SkaGamepadMapping* mapping = (const SkaGamepadMapping*)element;
    return strcmp(guid, mapping->guid);
}

const SkaGamepadMapping* ska_gamepad_db_find_by_guid(const char* guid) {
    const SkaGamepadMapping* table = ska_gamepad_db_get_table();
    const usize count = ska_gamepad_db_get_table_count();
    if (count == 0) {
        return NULL;
    }
    // gamepad_db_generated.c's per-platform tables are emitted pre-sorted by guid, so a
    // binary search avoids needing any runtime hash-map/init-lifecycle state for what is
    // otherwise fully static data.
    return (const SkaGamepadMapping*)bsearch(guid, table, count, sizeof(SkaGamepadMapping), compare_guid);
}

f32 ska_gamepad_apply_axis_modifier(f32 raw_normalized, SkaGamepadAxisModifier modifier) {
    switch (modifier) {
        case SkaGamepadAxisModifier_INVERTED:
            return -raw_normalized;
        case SkaGamepadAxisModifier_POSITIVE_HALF:
            return raw_normalized > 0.0f ? raw_normalized : 0.0f;
        case SkaGamepadAxisModifier_NEGATIVE_HALF:
            return raw_normalized < 0.0f ? -raw_normalized : 0.0f;
        case SkaGamepadAxisModifier_NONE:
        default:
            return raw_normalized;
    }
}

uint8 ska_gamepad_pov_angle_to_hat_bitmask(i32 pov_angle_hundredths_of_degree) {
    if (pov_angle_hundredths_of_degree < 0) {
        return 0; // centered / no direction pressed
    }
    // 8-way sector, one bitmask per 45-degree wedge centered on each cardinal/diagonal
    // direction (up=0deg, clockwise), matching SDL's h0.N up/right/down/left encoding.
    static const uint8 sector_bitmask[8] = {
        1,     // 0deg: up
        1 | 2, // 45deg: up-right
        2,     // 90deg: right
        2 | 4, // 135deg: down-right
        4,     // 180deg: down
        4 | 8, // 225deg: down-left
        8,     // 270deg: left
        8 | 1, // 315deg: up-left
    };
    const i32 normalized_angle = pov_angle_hundredths_of_degree % 36000;
    const i32 sector = ((normalized_angle + 2250) / 4500) % 8;
    return sector_bitmask[sector];
}

#endif // #if SKA_INPUT
