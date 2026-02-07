#include "App_Core.hpp"

namespace App {

static std::optional<uint32_t> lastTick = std::nullopt;
static uint32_t cycleCount = 0;

void Core::run(std::optional<uint32_t> systemTick) {
    if (systemTick.has_value()) {
        lastTick = systemTick;
    }
    ++cycleCount;
}

void Core::reportSensorFault(SensorId sensor) {
    (void)sensor;  // MISRA: Cast unused parameter
    // TODO: Implement DTC (Diagnostic Trouble Code) reporting
}

std::optional<uint32_t> Core::getLastTick() {
    return lastTick;
}

}  // namespace App
