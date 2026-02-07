#pragma once

#include <cstdint>
#include <optional>

/**
 * @brief Application Layer (Hardware-Agnostisch)
 * Reine C++17 Applikationslogik. Keine esp_ oder freertos Includes erlaubt.
 * Portierbar auf x86/PC fuer Host-Tests.
 */
namespace App {

/**
 * @brief Typsichere Sensor-Identifikation.
 */
enum class SensorId : uint8_t {
    kSystemTick = 0,
    kTemperature = 1,
    kVoltage = 2
};

/**
 * @brief Compile-Time Konfiguration der Applikation.
 */
namespace Config {
    static constexpr uint32_t kHeartbeatIntervalCycles = 1000;
    static constexpr uint32_t kCycleTimeMs = 10;
    static constexpr uint32_t kMaxTickValue = UINT32_MAX;

    static_assert(kHeartbeatIntervalCycles > 0, "Heartbeat interval must be positive");
    static_assert(kCycleTimeMs > 0, "Cycle time must be positive");
}  // namespace Config

/**
 * @brief Zentrale Applikationsklasse (statische Methoden, kein Heap).
 */
class Core {
   public:
    /**
     * @brief Hauptzyklus - wird alle 10ms vom Service Layer aufgerufen.
     * @param systemTick Aktueller System-Tick (optional fuer Fehlerfall).
     */
    static void run(std::optional<uint32_t> systemTick);

    /**
     * @brief Meldet einen Sensorfehler.
     * @param sensor Identifikation des fehlerhaften Sensors.
     */
    static void reportSensorFault(SensorId sensor);

    /**
     * @brief Gibt den letzten verarbeiteten Tick zurueck.
     * @return std::nullopt wenn noch kein Tick verarbeitet wurde.
     */
    static std::optional<uint32_t> getLastTick();
};

}  // namespace App
