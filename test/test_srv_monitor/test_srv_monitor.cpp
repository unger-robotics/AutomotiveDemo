#include <unity.h>

#include "Mcal_System.hpp"
#include "Srv_Monitor.hpp"

void setUp(void) {
    // Wird vor jedem Test aufgerufen
}

void tearDown(void) {
    // Wird nach jedem Test aufgerufen
}

void test_run_cycle_no_crash(void) {
    Mcal::System::init();
    Srv::Monitor::runCycle();
    TEST_ASSERT_TRUE(true);
}

void test_run_cycle_multiple(void) {
    Mcal::System::init();
    for (int i = 0; i < 10; ++i) {
        Srv::Monitor::runCycle();
    }
    TEST_ASSERT_TRUE(true);
}

extern "C" void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_run_cycle_no_crash);
    RUN_TEST(test_run_cycle_multiple);
    UNITY_END();
}
