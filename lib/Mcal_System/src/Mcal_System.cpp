#include "Mcal_System.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "MCAL";

namespace Mcal {
    void System::init() {
        // Simuliert Hardware-Init
        ESP_LOGI(TAG, "Hardware Initialized (QM Level).");
    }
    uint32_t System::getSystemTick() {
        return static_cast<uint32_t>(xTaskGetTickCount());
    }
}
