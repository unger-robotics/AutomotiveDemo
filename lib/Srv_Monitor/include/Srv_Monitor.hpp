#pragma once

/**
 * @brief Service Layer (RTE-Ersatz)
 * Koordiniert Applikation und Hardware.
 */
namespace Srv {
class Monitor {
   public:
    static void runCycle();
};
}  // namespace Srv
