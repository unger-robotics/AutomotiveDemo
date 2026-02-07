#pragma once
#include <cstdint>

/**
 * @brief Microcontroller Abstraction Layer (MCAL)
 * Kapselt Hardware-spezifische Initialisierungen (Clocks, Watchdogs).
 */
namespace Mcal {
class System {
   public:
    static void init();
    static uint32_t getSystemTick();
};
}  // namespace Mcal
