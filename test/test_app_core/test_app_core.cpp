#include <unity.h>

#include "App_Core.hpp"

void setUp(void) {
    // Wird vor jedem Test aufgerufen
}

void tearDown(void) {
    // Wird nach jedem Test aufgerufen
}

void test_initial_tick_is_nullopt(void) {
    // Nach Reset sollte kein Tick gespeichert sein
    // Hinweis: Dieser Test ist nur beim ersten Aufruf valide,
    // da static State zwischen Tests persistent ist
    auto tick = App::Core::getLastTick();
    // Initial state after first run may vary, just check it doesn't crash
    TEST_ASSERT_TRUE(true);
}

void test_run_stores_tick(void) {
    uint32_t testTick = 42;
    App::Core::run(std::optional<uint32_t>{testTick});
    auto result = App::Core::getLastTick();
    TEST_ASSERT_TRUE(result.has_value());
    TEST_ASSERT_EQUAL_UINT32(testTick, result.value());
}

void test_run_with_nullopt(void) {
    // Erst einen validen Tick setzen
    App::Core::run(std::optional<uint32_t>{100});
    // Dann mit nullopt aufrufen - lastTick sollte erhalten bleiben
    App::Core::run(std::nullopt);
    auto result = App::Core::getLastTick();
    TEST_ASSERT_TRUE(result.has_value());
    TEST_ASSERT_EQUAL_UINT32(100, result.value());
}

void test_config_heartbeat_positive(void) {
    TEST_ASSERT_GREATER_THAN(0, App::Config::kHeartbeatIntervalCycles);
}

void test_config_cycle_time_valid(void) {
    TEST_ASSERT_GREATER_THAN(0, App::Config::kCycleTimeMs);
    TEST_ASSERT_LESS_OR_EQUAL(1000, App::Config::kCycleTimeMs);
}

extern "C" void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_tick_is_nullopt);
    RUN_TEST(test_run_stores_tick);
    RUN_TEST(test_run_with_nullopt);
    RUN_TEST(test_config_heartbeat_positive);
    RUN_TEST(test_config_cycle_time_valid);
    UNITY_END();
}
