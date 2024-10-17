#include <unity.h>
#include <string.h>

#include "seika1/memory.h"
#include "seika1/data_structures/array_list.h"
#include "seika1/data_structures/id_queue.h"

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp(void) {}
void tearDown(void) {}

void seika_mem_test(void);
void seika_array_list_test(void);
void seika_id_queue_test(void);

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_mem_test);
    RUN_TEST(seika_array_list_test);
    RUN_TEST(seika_id_queue_test);
    return UNITY_END();
}

void seika_mem_test(void) {
    int* testInt = SKA_ALLOC(int);
    TEST_ASSERT_NOT_NULL(testInt);
    *testInt = 5;
    TEST_ASSERT_EQUAL_INT(5, *testInt);
    TEST_ASSERT_TRUE(ska_mem_report_leaks());
    SKA_FREE(testInt);
    TEST_ASSERT_FALSE(ska_mem_report_leaks());
}

void seika_array_list_test(void) {
    int listValues[] = { 8, 2, 3, 5 };
    SkaArrayList* arrayList = ska_array_list_create_default_capacity(sizeof(int));
    TEST_ASSERT_TRUE(ska_array_list_is_empty(arrayList));

    for (int i = 0; i < 4; i++) {
        ska_array_list_push_back(arrayList, &listValues[i]);
    }
    TEST_ASSERT_FALSE(ska_array_list_is_empty(arrayList));
    int index = 0;
    SKA_ARRAY_LIST_FOR_EACH(arrayList, int, i) {
        const int value = (int)*(int*)ska_array_list_get(arrayList, index);
        TEST_ASSERT_EQUAL_INT(listValues[index], value);
        index++;
    }

    ska_array_list_remove_by_index(arrayList, 1);
    int oneValue = (int)*(int*)ska_array_list_get(arrayList, 1);
    TEST_ASSERT_EQUAL_INT(3, oneValue);

    ska_array_list_remove(arrayList, &(int){3});
    oneValue = (int)*(int*)ska_array_list_get(arrayList, 1);
    TEST_ASSERT_EQUAL_INT(5, oneValue);

    ska_array_list_clear(arrayList);
    TEST_ASSERT_TRUE(ska_array_list_is_empty(arrayList));
    ska_array_list_destroy(arrayList);
}

void seika_id_queue_test(void) {
    SkaIdQueue* idQueue = ska_id_queue_create(10);

    uint32 value;
    for (uint32 i = 0; i < 10; i++) {
        ska_id_queue_dequeue(idQueue, &value);
        TEST_ASSERT_EQUAL_UINT32(i, value);
    }

    TEST_ASSERT_TRUE(ska_id_queue_is_empty(idQueue));

    //
    ska_id_queue_enqueue(idQueue, 4);
    ska_id_queue_enqueue(idQueue, 6);
    ska_id_queue_enqueue(idQueue, 5);
    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(4, value);
    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(6, value);
    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(5, value);

    ska_id_queue_dequeue(idQueue, &value);
    TEST_ASSERT_EQUAL_UINT32(10, value);
    TEST_ASSERT_EQUAL_size_t(20, idQueue->capacity);
}
