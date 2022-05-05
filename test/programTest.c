#include "unity.h"
#include "../src/program.h"

void setUp(void) {}

void tearDown(void) {}

void test_calc(void) {
    TEST_ASSERT_EQUAL_INT(-2, calc(1, 1));
    TEST_ASSERT_EQUAL_INT(3, calc(3, 0));
}

void test_greater(void) {
    TEST_ASSERT_GREATER_THAN_INT(12, 14);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_calc);
    RUN_TEST(test_greater);
    return UNITY_END();
}