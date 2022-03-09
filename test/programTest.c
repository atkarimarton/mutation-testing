#include "unity.h"
#include "../src/program.h"

void setUp(void) {}

void tearDown(void) {}

void testFunc(void) {
    TEST_ASSERT_EQUAL_UINT(2, func(2, 3));
    TEST_ASSERT_EQUAL_UINT(2, func2(2, 3));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(testFunc);
    return UNITY_END();
}