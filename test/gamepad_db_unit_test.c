#include <unity.h>
#include <string.h>
#include <seika/input/gamepad_db.h>

void setUp(void) {}
void tearDown(void) {}

void seika_gamepad_db_table_is_non_empty_test(void) {
    TEST_ASSERT_GREATER_THAN(0, ska_gamepad_db_get_table_count());
    TEST_ASSERT_NOT_NULL(ska_gamepad_db_get_table());
}

void seika_gamepad_db_table_is_sorted_by_guid_test(void) {
    const SkaGamepadMapping* table = ska_gamepad_db_get_table();
    const usize count = ska_gamepad_db_get_table_count();
    for (usize i = 1; i < count; i++) {
        TEST_ASSERT_TRUE_MESSAGE(strcmp(table[i - 1].guid, table[i].guid) < 0,
            "gamepad_db_generated.c's table must be sorted (and de-duplicated) by guid for bsearch to work");
    }
}

void seika_gamepad_db_find_by_guid_unknown_returns_null_test(void) {
    TEST_ASSERT_NULL(ska_gamepad_db_find_by_guid("not-a-real-guid"));
}

// Xbox 360 Controller, taken from this platform's section of the generated table.
#if defined(PLATFORM_WINDOWS)
#define KNOWN_XBOX_360_GUID "03000000380700001647000000000000"
#define KNOWN_XBOX_360_ENTRY_COUNT 20
#elif defined(PLATFORM_LINUX)
#define KNOWN_XBOX_360_GUID "030000005e0400008e02000010010000"
#define KNOWN_XBOX_360_ENTRY_COUNT 21
#elif defined(PLATFORM_MACOS)
#define KNOWN_XBOX_360_GUID "030000005e0400008e02000000000000"
#define KNOWN_XBOX_360_ENTRY_COUNT 21
#endif

void seika_gamepad_db_find_by_guid_known_controller_test(void) {
#ifndef KNOWN_XBOX_360_GUID
    TEST_IGNORE_MESSAGE("No desktop gamepad table on this platform");
#else
    const SkaGamepadMapping* mapping = ska_gamepad_db_find_by_guid(KNOWN_XBOX_360_GUID);
    TEST_ASSERT_NOT_NULL(mapping);
    TEST_ASSERT_EQUAL_STRING("Xbox 360 Controller", mapping->name);
    TEST_ASSERT_EQUAL_UINT32(KNOWN_XBOX_360_ENTRY_COUNT, mapping->entry_count);

    bool found_face_south = false, found_left_stick_x = false;
    for (uint32 i = 0; i < mapping->entry_count; i++) {
        const SkaGamepadMappingEntry* entry = &mapping->entries[i];
        if (entry->target_key == SkaInputKey_GAMEPAD_FACE_BUTTON_SOUTH) {
            TEST_ASSERT_EQUAL(SkaGamepadMappingSourceType_BUTTON, entry->source_type);
            TEST_ASSERT_EQUAL_UINT8(0, entry->index);
            found_face_south = true;
        }
        if (entry->target_key == SkaInputKey_GAMEPAD_LEFT_ANALOG_2D_AXIS_X) {
            TEST_ASSERT_EQUAL(SkaGamepadMappingSourceType_AXIS, entry->source_type);
            TEST_ASSERT_EQUAL_UINT8(0, entry->index);
            found_left_stick_x = true;
        }
    }
    TEST_ASSERT_TRUE(found_face_south);
    TEST_ASSERT_TRUE(found_left_stick_x);
#endif
}

void seika_gamepad_apply_axis_modifier_none_test(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.75f, ska_gamepad_apply_axis_modifier(0.75f, SkaGamepadAxisModifier_NONE));
    TEST_ASSERT_EQUAL_FLOAT(-0.75f, ska_gamepad_apply_axis_modifier(-0.75f, SkaGamepadAxisModifier_NONE));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_gamepad_apply_axis_modifier(0.0f, SkaGamepadAxisModifier_NONE));
}

void seika_gamepad_apply_axis_modifier_inverted_test(void) {
    TEST_ASSERT_EQUAL_FLOAT(-0.75f, ska_gamepad_apply_axis_modifier(0.75f, SkaGamepadAxisModifier_INVERTED));
    TEST_ASSERT_EQUAL_FLOAT(0.75f, ska_gamepad_apply_axis_modifier(-0.75f, SkaGamepadAxisModifier_INVERTED));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_gamepad_apply_axis_modifier(0.0f, SkaGamepadAxisModifier_INVERTED));
}

void seika_gamepad_apply_axis_modifier_positive_half_test(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.5f, ska_gamepad_apply_axis_modifier(0.5f, SkaGamepadAxisModifier_POSITIVE_HALF));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ska_gamepad_apply_axis_modifier(1.0f, SkaGamepadAxisModifier_POSITIVE_HALF));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_gamepad_apply_axis_modifier(-0.5f, SkaGamepadAxisModifier_POSITIVE_HALF));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_gamepad_apply_axis_modifier(0.0f, SkaGamepadAxisModifier_POSITIVE_HALF));
}

void seika_gamepad_apply_axis_modifier_negative_half_test(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.5f, ska_gamepad_apply_axis_modifier(-0.5f, SkaGamepadAxisModifier_NEGATIVE_HALF));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, ska_gamepad_apply_axis_modifier(-1.0f, SkaGamepadAxisModifier_NEGATIVE_HALF));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_gamepad_apply_axis_modifier(0.5f, SkaGamepadAxisModifier_NEGATIVE_HALF));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ska_gamepad_apply_axis_modifier(0.0f, SkaGamepadAxisModifier_NEGATIVE_HALF));
}

void seika_gamepad_pov_angle_to_hat_bitmask_cardinal_and_diagonal_test(void) {
    TEST_ASSERT_EQUAL_UINT8(1, ska_gamepad_pov_angle_to_hat_bitmask(0));         // up
    TEST_ASSERT_EQUAL_UINT8(1 | 2, ska_gamepad_pov_angle_to_hat_bitmask(4500)); // up-right
    TEST_ASSERT_EQUAL_UINT8(2, ska_gamepad_pov_angle_to_hat_bitmask(9000));      // right
    TEST_ASSERT_EQUAL_UINT8(2 | 4, ska_gamepad_pov_angle_to_hat_bitmask(13500));// down-right
    TEST_ASSERT_EQUAL_UINT8(4, ska_gamepad_pov_angle_to_hat_bitmask(18000));     // down
    TEST_ASSERT_EQUAL_UINT8(4 | 8, ska_gamepad_pov_angle_to_hat_bitmask(22500));// down-left
    TEST_ASSERT_EQUAL_UINT8(8, ska_gamepad_pov_angle_to_hat_bitmask(27000));     // left
    TEST_ASSERT_EQUAL_UINT8(8 | 1, ska_gamepad_pov_angle_to_hat_bitmask(31500));// up-left
    TEST_ASSERT_EQUAL_UINT8(1, ska_gamepad_pov_angle_to_hat_bitmask(35900));     // just short of wrapping back to up
}

void seika_gamepad_pov_angle_to_hat_bitmask_centered_test(void) {
    TEST_ASSERT_EQUAL_UINT8(0, ska_gamepad_pov_angle_to_hat_bitmask(-1));
}

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();

    RUN_TEST(seika_gamepad_db_table_is_non_empty_test);
    RUN_TEST(seika_gamepad_db_table_is_sorted_by_guid_test);
    RUN_TEST(seika_gamepad_db_find_by_guid_unknown_returns_null_test);
    RUN_TEST(seika_gamepad_db_find_by_guid_known_controller_test);

    RUN_TEST(seika_gamepad_apply_axis_modifier_none_test);
    RUN_TEST(seika_gamepad_apply_axis_modifier_inverted_test);
    RUN_TEST(seika_gamepad_apply_axis_modifier_positive_half_test);
    RUN_TEST(seika_gamepad_apply_axis_modifier_negative_half_test);

    RUN_TEST(seika_gamepad_pov_angle_to_hat_bitmask_cardinal_and_diagonal_test);
    RUN_TEST(seika_gamepad_pov_angle_to_hat_bitmask_centered_test);

    return UNITY_END();
}
