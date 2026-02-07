#include <unity.h>

#include "Mcal_System.hpp"

void setUp(void) {
    // Wird vor jedem Test aufgerufen
}

void tearDown(void) {
    // Wird nach jedem Test aufgerufen
}

void test_system_init_no_crash(void) {
    Mcal::System::init();
    TEST_ASSERT_TRUE(true);
}

void test_system_tick_returns_value(void) {
    uint32_t tick = Mcal::System::getSystemTick();
    TEST_ASSERT_GREATER_OR_EQUAL(0, tick);
}

void test_system_tick_monotonic(void) {
    uint32_t tick1 = Mcal::System::getSystemTick();
    uint32_t tick2 = Mcal::System::getSystemTick();
    TEST_ASSERT_GREATER_OR_EQUAL(tick1, tick2);
}

extern "C" void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_system_init_no_crash);
    RUN_TEST(test_system_tick_returns_value);
    RUN_TEST(test_system_tick_monotonic);
    UNITY_END();
}
