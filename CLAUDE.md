# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**AutomotiveDemo** — QM (Quality Managed) Demonstrator for Automotive Software Engineering on ESP32-S3. Applies ISO 26262 methodology (V-Modell, MISRA C++ alignment, static analysis) on COTS hardware. The software architecture is designed to be portable to ASIL-certified MCUs (e.g., Infineon Aurix, NXP S32K).

**Target:** Seeed Studio XIAO ESP32-S3 (Dual-Core Xtensa LX7)
**Framework:** ESP-IDF with FreeRTOS (not Arduino)
**Languages:** C++17 (Application/Service layers), C11 (MCAL layer)

## Build & Analysis Commands

```bash
pio run                    # Build firmware (first build takes 2-5 min, subsequent ~5s)
pio run --target upload    # Flash to board via USB-C
pio check                  # Static analysis (cppcheck + clang-tidy) — mandatory quality gate
pio device monitor         # Serial monitor (115200 baud, with exception decoder)
pio test                   # All unit tests (Unity framework, requires physical board)
pio test -f test_app_core  # Run single test suite by folder name
```

## Architecture (3-Layer Model)

Strict layered architecture inspired by AUTOSAR. Each layer communicates only with the layer directly below. No callbacks, no events, no layer-bypassing.

```
Application Layer (App_*)  →  Pure C++17 logic, NO esp_/freertos includes, portable to x86
Service Layer    (Srv_*)   →  RTE equivalent, 10ms cyclic execution, may include esp_log.h
MCAL             (Mcal_*)  →  Hardware abstraction, wraps ESP-IDF/FreeRTOS drivers
```

**Modules:**
- `App_Core` — Application logic, state tracking, sensor fault reporting (`std::optional` API)
- `Srv_Monitor` — Cyclic 10ms execution, coordinates App and MCAL layers, owns logging
- `Mcal_System` — Hardware init, system tick (wraps `xTaskGetTickCount`)

**Call chain:** `app_main()` → `Mcal::System::init()` → `xTaskCreate(SafetyTask)` → loop: `Srv::Monitor::runCycle()` → `App::Core::run()` every 10ms

**Module structure convention:** Each module lives in `lib/<Prefix>_<Name>/` with separate `include/` and `src/` directories. The prefix (`Mcal_`, `Srv_`, `App_`) encodes the layer — architecture violations are visible at the `#include` statement.

**Layer include rules:**
- `App_*` files must **never** include `esp_`, `freertos/`, or `Mcal_` headers
- `Srv_*` files include `Mcal_` and `App_` headers, plus `esp_log.h` for logging
- `Mcal_*` files include ESP-IDF/FreeRTOS headers

## Coding Constraints

- **`-Werror` policy:** Zero warnings allowed. Every compiler warning breaks the build.
- **No heap after init:** No `malloc`/`new` after `app_main()` returns. Static or stack allocation only.
- **No C++ exceptions:** Disabled via ESP-IDF config. Use `std::optional` or error codes instead.
- **MISRA-aligned casts:** Use `static_cast<>()` never C-style casts. Cast unused parameters with `(void)param`.
- **Constants:** `static constexpr` with `k` prefix (e.g., `kHeartbeatIntervalCycles`), never `#define`.
- **printf with uint32_t:** Use `static_cast<unsigned long>(val)` with `%lu` format specifier for portability.
- **Include order:** Own header first, then project headers, then system headers (alphabetically sorted by `.clang-format`).
- **Deterministic tasks:** Fixed priorities, fixed stack sizes, no dynamic task creation at runtime.
- **Formatting:** `.clang-format` enforces Google-based style with 4-space indent, 100-char column limit, no single-line blocks/functions/ifs/loops (safety rules).

## Static Analysis Configuration

Both tools are configured in `platformio.ini` and `.clang-tidy`. Must pass with zero High/Medium findings:

- **cppcheck:** `--enable=warning,style,performance,portability`
- **clang-tidy:** `cert-*`, `clang-analyzer-*`, `bugprone-*`, `performance-*`, `readability-*` (disabled: `readability-magic-numbers`, `readability-identifier-length`, `bugprone-easily-swappable-parameters`)

Suppressions require inline justification (`// cppcheck-suppress <id> -- <reason>` or `// NOLINT(<check>) -- <reason>`).

## Testing

Unit tests use the **Unity** framework (PlatformIO default for ESP-IDF) and run on the ESP32-S3 target:

- `test/test_mcal_system/` — MCAL layer tests (init, tick monotonicity)
- `test/test_app_core/` — Application layer tests (state, config validation)
- `test/test_srv_monitor/` — Service layer tests (runCycle stability)

Each test suite has its own `extern "C" void app_main()` entry point with `UNITY_BEGIN()`/`UNITY_END()` and runs independently. Tests require a physically connected board.

## CI

GitHub Actions workflow (`.github/workflows/build.yml`) runs `pio run` + `pio check` on push/PR to `main`.

## Documentation

All project documentation lives in `docs/`:
- `01_Systemdokumentation.md` — Architecture, interfaces, design constraints
- `02_Benutzerhandbuch.md` — Build setup, static analysis workflow
- `03_Abschlussbericht.md` — Gap analysis (Prototype vs. Series), ASIL classification
- `04_Cpp17_Herleitung.md` — C++17 rationale for safety-critical embedded systems
