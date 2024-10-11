#include <unity.h>
#include <string.h>

#include "seika1/memory.h"

#define RESOURCES_PATH "test/resources"
#define RESOURCES_PACK_PATH "test/resources/test.pck"

void setUp(void) {}
void tearDown(void) {}

void seika_mem_test(void);

int32 main(int32 argv, char** args) {
    UNITY_BEGIN();
    RUN_TEST(seika_mem_test);
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
