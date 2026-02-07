/**
 * @file main.cpp
 * @brief OS Entry Point (Startup Code)
 * * Konfiguration:
 * - Standard: C++17
 * - Compliance: MISRA C++ (Partial Coverage)
 */

#include "Mcal_System.hpp"
#include "Srv_Monitor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static_assert(sizeof(void*) == 4, "Error: Target must be 32-bit");

// Task-Konfiguration (deterministische Werte, keine Magic Numbers)
static constexpr uint32_t kSafetyTaskStackSize = 4096;
static constexpr UBaseType_t kSafetyTaskPriority = 5;

extern "C" {
void app_main(void);
}

// Task Wrapper für C++ Aufrufe
void run_safety_task(void* pvParameters) {
    (void)pvParameters;

    while (true) {
        // Zyklischer Aufruf des "RTE" / Service Layers
        Srv::Monitor::runCycle();

        // 10ms Zykluszeit (Soft Real-Time)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    // 1. Hardware Initialisierung (MCAL)
    Mcal::System::init();

    // 2. Erstellen der OS Tasks
    xTaskCreate(run_safety_task, "SafetyTask", kSafetyTaskStackSize, nullptr, kSafetyTaskPriority,
                nullptr);

    // app_main endet hier, FreeRTOS Scheduler übernimmt.
}
