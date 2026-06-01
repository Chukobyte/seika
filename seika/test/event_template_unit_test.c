#include <unity.h>
#include <seika/data_structures/event_template.h>
#include <seika/memory.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

// ===== TEST TYPES =====

typedef struct PlayerData {
    i32 id;
    char name[32];
    f32 health;
} PlayerData;

typedef struct Entity {
    i32 id;
    char name[32];
} Entity;

// Declare and implement event types
SKA_EVENT_DECLARE(HealthChangedEvent, i32)
SKA_EVENT_IMPLEMENT(HealthChangedEvent, i32)

SKA_EVENT_DECLARE(DamageTakenEvent, f32)
SKA_EVENT_IMPLEMENT(DamageTakenEvent, f32)

SKA_EVENT_DECLARE(PlayerUpdatedEvent, PlayerData)
SKA_EVENT_IMPLEMENT(PlayerUpdatedEvent, PlayerData)

SKA_EVENT_DECLARE(EntitySpawnedEvent, Entity*)
SKA_EVENT_IMPLEMENT(EntitySpawnedEvent, Entity*)

// ===== TEST TRACKING =====

static i32 g_call_count = 0;
static i32 g_last_int_value = 0;
static f32 g_last_f32_value = 0.0f;
static PlayerData g_last_player = {0};
static Entity* g_last_entity = NULL;

// ===== CALLBACKS =====

static void on_health_changed(i32 health) {
    g_call_count++;
    g_last_int_value = health;
}

static void on_health_changed_2(i32 health) {
    g_call_count++;
    g_last_int_value = health * 2;
}

static void on_damage_taken(f32 damage) {
    g_call_count++;
    g_last_f32_value = damage;
}

static void on_player_updated(PlayerData player) {
    g_call_count++;
    g_last_player = player;
}

static void on_entity_spawned(Entity* entity) {
    g_call_count++;
    g_last_entity = entity;
}

// ===== BASIC TESTS =====

void seika_event_template_int_basic_test(void) {
    g_call_count = 0;
    g_last_int_value = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_notify(&event, 50);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count);
    TEST_ASSERT_EQUAL_INT32(50, g_last_int_value);

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_int_multiple_subscribers_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed_2);

    TEST_ASSERT_EQUAL_UINT(2, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_notify(&event, 10);

    TEST_ASSERT_EQUAL_INT32(2, g_call_count);
    // Note: Callbacks are called in reverse order of subscription (LIFO due to linked list)
    // on_health_changed_2 is called first (sets to 20), then on_health_changed (sets to 10)
    TEST_ASSERT_EQUAL_INT32(10, g_last_int_value); // Last callback executed sets value to 10

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_int_unsubscribe_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed_2);

    TEST_ASSERT_TRUE(HealthChangedEvent_unsubscribe(&event, on_health_changed));
    TEST_ASSERT_EQUAL_UINT(1, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_notify(&event, 5);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count); // Only one callback
    TEST_ASSERT_EQUAL_INT32(10, g_last_int_value); // 5 * 2

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_int_clear_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed_2);

    HealthChangedEvent_clear(&event);

    TEST_ASSERT_EQUAL_UINT(0, HealthChangedEvent_subscriber_count(&event));
    TEST_ASSERT_FALSE(HealthChangedEvent_has_subscribers(&event));

    HealthChangedEvent_notify(&event, 100);

    TEST_ASSERT_EQUAL_INT32(0, g_call_count);

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_int_has_subscribers_test(void) {
    HealthChangedEvent event = HealthChangedEvent_create();

    TEST_ASSERT_FALSE(HealthChangedEvent_has_subscribers(&event));

    HealthChangedEvent_subscribe(&event, on_health_changed);

    TEST_ASSERT_TRUE(HealthChangedEvent_has_subscribers(&event));

    HealthChangedEvent_destroy(&event);
}

// ===== FLOAT TYPE TESTS =====

void seika_event_template_f32_test(void) {
    g_call_count = 0;
    g_last_f32_value = 0.0f;

    DamageTakenEvent event = DamageTakenEvent_create();

    DamageTakenEvent_subscribe(&event, on_damage_taken);
    DamageTakenEvent_notify(&event, 25.5f);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count);
    TEST_ASSERT_EQUAL_FLOAT(25.5f, g_last_f32_value);

    DamageTakenEvent_destroy(&event);
}

// ===== STRUCT TYPE TESTS =====

void seika_event_template_struct_test(void) {
    g_call_count = 0;
    memset(&g_last_player, 0, sizeof(PlayerData));

    PlayerUpdatedEvent event = PlayerUpdatedEvent_create();

    PlayerUpdatedEvent_subscribe(&event, on_player_updated);

    PlayerData player = { .id = 42, .health = 100.0f };
    strcpy(player.name, "TestPlayer");

    PlayerUpdatedEvent_notify(&event, player);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count);
    TEST_ASSERT_EQUAL_INT32(42, g_last_player.id);
    TEST_ASSERT_EQUAL_STRING("TestPlayer", g_last_player.name);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, g_last_player.health);

    PlayerUpdatedEvent_destroy(&event);
}

void seika_event_template_struct_multiple_notifications_test(void) {
    g_call_count = 0;

    PlayerUpdatedEvent event = PlayerUpdatedEvent_create();

    PlayerUpdatedEvent_subscribe(&event, on_player_updated);

    PlayerData p1 = { .id = 1, .health = 50.0f };
    strcpy(p1.name, "Player1");
    PlayerUpdatedEvent_notify(&event, p1);

    PlayerData p2 = { .id = 2, .health = 75.0f };
    strcpy(p2.name, "Player2");
    PlayerUpdatedEvent_notify(&event, p2);

    TEST_ASSERT_EQUAL_INT32(2, g_call_count);
    TEST_ASSERT_EQUAL_INT32(2, g_last_player.id);
    TEST_ASSERT_EQUAL_STRING("Player2", g_last_player.name);

    PlayerUpdatedEvent_destroy(&event);
}

// ===== POINTER TYPE TESTS =====

void seika_event_template_pointer_test(void) {
    g_call_count = 0;
    g_last_entity = NULL;

    EntitySpawnedEvent event = EntitySpawnedEvent_create();

    EntitySpawnedEvent_subscribe(&event, on_entity_spawned);

    Entity entity = { .id = 123 };
    strcpy(entity.name, "Monster");

    EntitySpawnedEvent_notify(&event, &entity);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count);
    TEST_ASSERT_NOT_NULL(g_last_entity);
    TEST_ASSERT_EQUAL_INT32(123, g_last_entity->id);
    TEST_ASSERT_EQUAL_STRING("Monster", g_last_entity->name);

    EntitySpawnedEvent_destroy(&event);
}

void seika_event_template_pointer_null_test(void) {
    g_call_count = 0;
    g_last_entity = (Entity*)0xDEADBEEF; // Set to non-null

    EntitySpawnedEvent event = EntitySpawnedEvent_create();

    EntitySpawnedEvent_subscribe(&event, on_entity_spawned);
    EntitySpawnedEvent_notify(&event, NULL);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count);
    TEST_ASSERT_NULL(g_last_entity);

    EntitySpawnedEvent_destroy(&event);
}

// ===== EDGE CASE TESTS =====

void seika_event_template_no_subscribers_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    // Should not crash
    HealthChangedEvent_notify(&event, 100);

    TEST_ASSERT_EQUAL_INT32(0, g_call_count);

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_unsubscribe_not_subscribed_test(void) {
    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);

    // Try to unsubscribe callback that wasn't subscribed
    bool result = HealthChangedEvent_unsubscribe(&event, on_health_changed_2);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_UINT(1, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_subscribe_same_callback_multiple_times_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed);

    TEST_ASSERT_EQUAL_UINT(3, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_notify(&event, 10);

    // Each subscription is independent
    TEST_ASSERT_EQUAL_INT32(3, g_call_count);

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_unsubscribe_first_of_duplicates_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_subscribe(&event, on_health_changed);

    // Unsubscribe removes first occurrence
    HealthChangedEvent_unsubscribe(&event, on_health_changed);

    TEST_ASSERT_EQUAL_UINT(2, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_notify(&event, 10);

    TEST_ASSERT_EQUAL_INT32(2, g_call_count);

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_clear_then_reuse_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);
    HealthChangedEvent_clear(&event);

    // Resubscribe after clear
    HealthChangedEvent_subscribe(&event, on_health_changed_2);
    HealthChangedEvent_notify(&event, 5);

    TEST_ASSERT_EQUAL_INT32(1, g_call_count);
    TEST_ASSERT_EQUAL_INT32(10, g_last_int_value); // 5 * 2

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_multiple_events_independent_test(void) {
    g_call_count = 0;

    HealthChangedEvent health_event = HealthChangedEvent_create();
    DamageTakenEvent damage_event = DamageTakenEvent_create();

    HealthChangedEvent_subscribe(&health_event, on_health_changed);
    DamageTakenEvent_subscribe(&damage_event, on_damage_taken);

    HealthChangedEvent_notify(&health_event, 50);
    TEST_ASSERT_EQUAL_INT32(1, g_call_count);

    DamageTakenEvent_notify(&damage_event, 25.5f);
    TEST_ASSERT_EQUAL_INT32(2, g_call_count);

    HealthChangedEvent_destroy(&health_event);
    DamageTakenEvent_destroy(&damage_event);
}

// ===== STRESS TESTS =====

void seika_event_template_many_subscribers_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    // Add 100 subscribers
    for (int i = 0; i < 100; i++) {
        HealthChangedEvent_subscribe(&event, on_health_changed);
    }

    TEST_ASSERT_EQUAL_UINT(100, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_notify(&event, 50);

    TEST_ASSERT_EQUAL_INT32(100, g_call_count);

    HealthChangedEvent_destroy(&event);
}

void seika_event_template_many_notifications_test(void) {
    g_call_count = 0;

    HealthChangedEvent event = HealthChangedEvent_create();

    HealthChangedEvent_subscribe(&event, on_health_changed);

    // Send 1000 notifications
    for (i32 i = 0; i < 1000; i++) {
        HealthChangedEvent_notify(&event, i);
    }

    TEST_ASSERT_EQUAL_INT32(1000, g_call_count);
    TEST_ASSERT_EQUAL_INT32(999, g_last_int_value);

    HealthChangedEvent_destroy(&event);
}

// ===== ALLOCATOR TEST =====

void seika_event_template_custom_allocator_test(void) {
    SkaAllocator custom_allocator = ska_mem_get_global_allocator();

    HealthChangedEvent event = HealthChangedEvent_create_a(custom_allocator);

    HealthChangedEvent_subscribe(&event, on_health_changed);

    TEST_ASSERT_EQUAL_UINT(1, HealthChangedEvent_subscriber_count(&event));

    HealthChangedEvent_destroy(&event);
}

// ===== MAIN TEST RUNNER =====

i32 main(i32 argv, char** args) {
    UNITY_BEGIN();

    // Basic int tests
    RUN_TEST(seika_event_template_int_basic_test);
    RUN_TEST(seika_event_template_int_multiple_subscribers_test);
    RUN_TEST(seika_event_template_int_unsubscribe_test);
    RUN_TEST(seika_event_template_int_clear_test);
    RUN_TEST(seika_event_template_int_has_subscribers_test);

    // Float tests
    RUN_TEST(seika_event_template_f32_test);

    // Struct tests
    RUN_TEST(seika_event_template_struct_test);
    RUN_TEST(seika_event_template_struct_multiple_notifications_test);

    // Pointer tests
    RUN_TEST(seika_event_template_pointer_test);
    RUN_TEST(seika_event_template_pointer_null_test);

    // Edge cases
    RUN_TEST(seika_event_template_no_subscribers_test);
    RUN_TEST(seika_event_template_unsubscribe_not_subscribed_test);
    RUN_TEST(seika_event_template_subscribe_same_callback_multiple_times_test);
    RUN_TEST(seika_event_template_unsubscribe_first_of_duplicates_test);
    RUN_TEST(seika_event_template_clear_then_reuse_test);
    RUN_TEST(seika_event_template_multiple_events_independent_test);

    // Stress tests
    RUN_TEST(seika_event_template_many_subscribers_test);
    RUN_TEST(seika_event_template_many_notifications_test);

    // Allocator test
    RUN_TEST(seika_event_template_custom_allocator_test);

    return UNITY_END();
}
