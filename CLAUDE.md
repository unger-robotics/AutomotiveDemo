# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Projektuebersicht

**AutomotiveDemo** — QM-Demonstrator (Quality Managed) fuer Automotive Software Engineering auf ESP32-S3. Wendet ISO 26262-Methodik an (V-Modell, MISRA C++ Ausrichtung, statische Analyse) auf COTS-Hardware. Die Software-Architektur ist portierbar auf ASIL-zertifizierte MCUs (z.B. Infineon Aurix, NXP S32K).

**Zielplattform:** Seeed Studio XIAO ESP32-S3 (Dual-Core Xtensa LX7)
**Framework:** ESP-IDF mit FreeRTOS (nicht Arduino)
**Sprachen:** C++17 (Application/Service Layer), C11 (MCAL Layer)

## Build- und Analyse-Befehle

```bash
pio run                    # Firmware bauen (erster Build 2-5 Min, danach ~5s)
pio run --target upload    # Auf Board flashen via USB-C
pio check                  # Statische Analyse (cppcheck + clang-tidy) — verpflichtendes Quality Gate
pio device monitor         # Serielle Ausgabe (115200 Baud, mit Exception Decoder)
pio test                   # Alle Unit Tests (Unity Framework, Board muss angeschlossen sein)
pio test -f test_app_core  # Einzelne Test-Suite nach Ordnername ausfuehren
pio debug                  # GDB-Debugging via USB-JTAG (oder F5 in VS Code)
clang-format -i src/*.cpp lib/*/src/*.cpp lib/*/include/*.hpp  # Code formatieren
```

## Architektur (3-Schichten-Modell)

Strikte Schichtenarchitektur nach AUTOSAR-Vorbild. Jede Schicht kommuniziert nur mit der direkt darunterliegenden. Keine Callbacks, keine Events, kein Layer-Bypassing.

```
Application Layer (App_*)  →  Reine C++17-Logik, KEINE esp_/freertos Includes, portierbar auf x86
Service Layer    (Srv_*)   →  RTE-Aequivalent, 10ms zyklische Ausfuehrung, darf esp_log.h nutzen
MCAL             (Mcal_*)  →  Hardware-Abstraktion, kapselt ESP-IDF/FreeRTOS-Treiber
```

**Module:**
- `App_Core` — Applikationslogik, Zustandsverwaltung, Sensor-Fehlerreporting (`std::optional` API)
- `Srv_Monitor` — Zyklische 10ms-Ausfuehrung, koordiniert App und MCAL, verantwortlich fuer Logging
- `Mcal_System` — Hardware-Init, System-Tick (kapselt `xTaskGetTickCount`)

**Aufrufkette:** `app_main()` → `Mcal::System::init()` → `xTaskCreate(SafetyTask)` → Schleife: `Srv::Monitor::runCycle()` → `App::Core::run()` alle 10ms

**Modulstruktur-Konvention:** Jedes Modul liegt in `lib/<Praefix>_<Name>/` mit separaten `include/` und `src/` Verzeichnissen. Das Praefix (`Mcal_`, `Srv_`, `App_`) kodiert die Schicht — Architekturverletzungen sind am `#include`-Statement sichtbar.

**Include-Regeln pro Schicht:**
- `App_*` Dateien duerfen **niemals** `esp_`, `freertos/` oder `Mcal_` Header inkludieren
- `Srv_*` Dateien inkludieren `Mcal_` und `App_` Header, dazu `esp_log.h` fuer Logging
- `Mcal_*` Dateien inkludieren ESP-IDF/FreeRTOS Header

## Coding-Regeln

- **`-Werror` Policy:** Null Warnungen erlaubt. Jede Compiler-Warnung bricht den Build.
- **Kein Heap nach Init:** Kein `malloc`/`new` nach Rueckkehr von `app_main()`. Nur statische oder Stack-Allokation.
- **Keine C++ Exceptions:** Deaktiviert via ESP-IDF Konfiguration. Stattdessen `std::optional` oder Fehlercodes verwenden.
- **MISRA-konforme Casts:** `static_cast<>()` verwenden, niemals C-Style Casts. Ungenutzte Parameter mit `(void)param` casten.
- **Konstanten:** `static constexpr` mit `k`-Praefix (z.B. `kHeartbeatIntervalCycles`), niemals `#define`.
- **printf mit uint32_t:** `static_cast<unsigned long>(val)` mit `%lu` Format-Specifier fuer Portabilitaet.
- **Include-Reihenfolge:** Eigener Header zuerst, dann Projekt-Header, dann System-Header (alphabetisch sortiert durch `.clang-format`).
- **Deterministische Tasks:** Feste Prioritaeten, feste Stack-Groessen, keine dynamische Task-Erzeugung zur Laufzeit.
- **Formatierung:** `.clang-format` erzwingt Google-basierten Stil mit 4-Space Einrueckung, 100 Zeichen Zeilenlimit, keine einzeiligen Bloecke/Funktionen/Ifs/Schleifen (Safety-Regeln).

## Statische Analyse

Beide Tools sind in `platformio.ini` und `.clang-tidy` konfiguriert. Muessen mit null High/Medium Findings bestehen:

- **cppcheck:** `--enable=warning,style,performance,portability`
- **clang-tidy:** `cert-*`, `clang-analyzer-*`, `bugprone-*`, `performance-*`, `readability-*` (deaktiviert: `readability-magic-numbers`, `readability-identifier-length`, `bugprone-easily-swappable-parameters`)

Unterdrueckungen erfordern Inline-Begruendung (`// cppcheck-suppress <id> -- <Grund>` oder `// NOLINT(<check>) -- <Grund>`).

## Tests

Unit Tests nutzen das **Unity** Framework (PlatformIO-Standard fuer ESP-IDF) und laufen auf dem ESP32-S3 Target:

- `test/test_mcal_system/` — MCAL-Layer Tests (Init, Tick-Monotonie)
- `test/test_app_core/` — Application-Layer Tests (Zustand, Config-Validierung)
- `test/test_srv_monitor/` — Service-Layer Tests (runCycle-Stabilitaet)

Jede Test-Suite hat einen eigenen `extern "C" void app_main()` Einstiegspunkt mit `UNITY_BEGIN()`/`UNITY_END()` und laeuft unabhaengig. Tests erfordern ein physisch angeschlossenes Board.

## CI

GitHub Actions Workflow (`.github/workflows/build.yml`) fuehrt `pio run` + `pio check` bei Push/PR auf `main` aus.

## Bekannte Plattform-Probleme

- **macOS esptool/intelhex:** `ModuleNotFoundError: No module named 'intelhex'` — Fix: `pip3 install --target=$(echo ~/.platformio/packages/tool-esptoolpy) intelhex`
- **Flash-Groesse:** Warnung `Expected 8MB, found 2MB` ist kosmetisch und beeinflusst den Build nicht.
- **Erster Build:** Dauert 2-5 Minuten (kompiliert gesamtes ESP-IDF Framework), danach ~5 Sekunden.

## Dokumentation

Gesamte Projektdokumentation liegt in `docs/` und verwendet **ASCII-Umschreibungen** (ae/oe/ue/ss statt Umlaute/Eszett) fuer konsistente Darstellung unabhaengig vom Editor:
- `01_Systemdokumentation.md` — Architektur, Schnittstellen, Design-Constraints
- `02_Benutzerhandbuch.md` — Build-Setup, Analyse-Workflow
- `03_Abschlussbericht.md` — Gap-Analyse (Prototyp vs. Serie), ASIL-Klassifikation
- `04_Cpp17_Herleitung.md` — C++17-Rationale fuer sicherheitskritische Systeme
- `PRESENTATION.md` — 6-Folien-Praesentation mit Mermaid-Diagrammen und Sprechernotizen
