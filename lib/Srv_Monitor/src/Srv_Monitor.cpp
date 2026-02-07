#include "Srv_Monitor.hpp"

#include "App_Core.hpp"
#include "Mcal_System.hpp"
#include "esp_log.h"

static const char* TAG = "SRV_MON";

namespace Srv {

static uint32_t cycleCounter = 0;

void Monitor::runCycle() {
    auto tick = Mcal::System::getSystemTick();

    App::Core::run(std::optional<uint32_t>{tick});

    ++cycleCounter;
    if (cycleCounter >= App::Config::kHeartbeatIntervalCycles) {
        ESP_LOGI(TAG, "Heartbeat | tick=%lu | cycles=%lu",
                 static_cast<unsigned long>(tick),
                 static_cast<unsigned long>(cycleCounter));
        cycleCounter = 0;
    }
}

}  // namespace Srv
