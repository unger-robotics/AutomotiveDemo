# AutomotiveDemo (QM Demonstrator)

![Standard](https://img.shields.io/badge/Standard-C%2B%2B17-blue)
![Compliance](https://img.shields.io/badge/Methodik-ISO26262_Anlehnung-green)
![Platform](https://img.shields.io/badge/Platform-ESP32--S3-orange)
![Build](https://github.com/unger-robotics/AutomotiveDemo/actions/workflows/build.yml/badge.svg)

Ein **QM-Demonstrator fuer Automotive Software Engineering** auf COTS-Hardware (Seeed Studio XIAO ESP32-S3). Zeigt die praktische Anwendung von ISO 26262-Methodik: Schichtenarchitektur, statische Analyse, MISRA-konformes C++17 und deterministische Echtzeit-Tasks.

Die Software-Architektur ist so aufgebaut, dass ein Plattformwechsel auf ASIL-zertifizierte MCUs (z.B. Infineon Aurix, NXP S32K) nur Aenderungen in der MCAL-Schicht erfordert.

## Architektur

```
Application Layer (App_Core)   Reine C++17 Logik, portierbar auf x86
        |
Service Layer    (Srv_Monitor)  Zyklische Ablaufsteuerung (10ms)
        |
MCAL             (Mcal_System)  Hardware-Abstraktion (ESP-IDF/FreeRTOS)
```

Strikte Schichtentrennung nach AUTOSAR-Vorbild. Jede Schicht kommuniziert nur mit der direkt darunterliegenden. Details in [`docs/01_Systemdokumentation.md`](docs/01_Systemdokumentation.md).

## Voraussetzungen

- [PlatformIO CLI](https://platformio.org/install/cli) oder PlatformIO IDE (VS Code Extension)
- Seeed Studio XIAO ESP32-S3 (fuer Flash und Tests)

## Quickstart

```bash
pio run                    # Firmware bauen
pio run --target upload    # Auf Board flashen (USB-C)
pio device monitor         # Serielle Ausgabe (115200 Baud)
pio check                  # Statische Analyse (cppcheck + clang-tidy)
pio test                   # Unit Tests (Board muss angeschlossen sein)
```

## Quality Gates

| Gate | Tool | Konfiguration |
|------|------|---------------|
| Compiler | GCC Xtensa | `-Wall -Wextra -Werror` (Zero Warnings) |
| Statische Analyse | cppcheck + clang-tidy | `pio check` |
| Formatierung | clang-format | Google-basiert, 4-Space Indent |
| Unit Tests | Unity Framework | `pio test` (auf Target) |
| CI | GitHub Actions | Build + Analyse bei Push/PR |

## Dokumentation

| Dokument | Inhalt |
|----------|--------|
| [`01_Systemdokumentation.md`](docs/01_Systemdokumentation.md) | Architektur, Schnittstellen, Design-Constraints |
| [`02_Benutzerhandbuch.md`](docs/02_Benutzerhandbuch.md) | Build-Setup, Analyse-Workflow |
| [`03_Abschlussbericht.md`](docs/03_Abschlussbericht.md) | Gap-Analyse (Prototyp vs. Serie), ASIL-Klassifikation |
| [`04_Cpp17_Herleitung.md`](docs/04_Cpp17_Herleitung.md) | C++17-Rationale fuer sicherheitskritische Systeme |

## Lizenz

Dieses Projekt ist ein Lehrdemontstrator und unterliegt keiner Open-Source-Lizenz.
