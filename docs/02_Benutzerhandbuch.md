# 2. Benutzerhandbuch

**Zielgruppe:** Entwickler, Integratoren
**Zweck:** Reproduzierbarkeit der Build-Umgebung sicherstellen und den vollstaendigen Workflow von der Installation bis zur statischen Analyse dokumentieren.

---

## 2.1 Voraussetzungen (Prerequisites)

### Hardware

| Komponente | Beschreibung |
|---|---|
| **Board** | Seeed Studio XIAO ESP32-S3 |
| **USB-Kabel** | USB-C Datenkabel (kein reines Ladekabel) |
| **Verbindung** | USB-C direkt am Entwicklungsrechner (kein Hub empfohlen) |

### Software

| Komponente | Version / Hinweis |
|---|---|
| **VS Code** | Aktuelle stabile Version (>= 1.85) |
| **PlatformIO IDE** | VS Code Extension (installiert Toolchain automatisch) |
| **Espressif 32 Platform** | v6.x (wird durch PlatformIO aufgeloest) |
| **GCC Toolchain** | >= v11 mit C++17-Unterstuetzung (automatisch durch PlatformIO) |
| **Python** | >= 3.8 (Voraussetzung fuer PlatformIO Core) |

### Betriebssystem-Anforderungen

- **macOS:** >= 12 (Monterey) -- USB-Treiber nativ vorhanden
- **Linux:** >= Ubuntu 22.04 oder vergleichbar -- ggf. `udev`-Regeln fuer ESP32 setzen
- **Windows:** >= Windows 10 -- USB-Treiber werden durch PlatformIO installiert

> **Hinweis:** Unter Linux muessen ggf. Berechtigungen fuer den seriellen Port gesetzt werden:
> ```bash
> sudo usermod -aG dialout $USER
> ```
> Anschliessend ist ein Neustart der Sitzung erforderlich.

---

## 2.2 Installation und Build

### Schritt 1: Repository klonen

Klonen Sie das Repository in ein lokales Verzeichnis:

```bash
git clone <REPOSITORY_URL> AutomotiveDemo
cd AutomotiveDemo
```

### Schritt 2: Projekt in VS Code oeffnen

Oeffnen Sie das Projektverzeichnis in VS Code:

```bash
code AutomotiveDemo
```

Warten Sie, bis PlatformIO die Indexierung abgeschlossen hat. In der Statusleiste erscheint das PlatformIO-Symbol. Beim ersten Oeffnen laedt PlatformIO automatisch:

- Die Espressif 32 Platform (v6.x)
- Die GCC-Toolchain (>= v11)
- Das ESP-IDF Framework
- Alle projektspezifischen Abhaengigkeiten

Dieser Vorgang kann beim ersten Mal mehrere Minuten dauern.

### Schritt 3: Build ausfuehren

Fuehren Sie den Build ueber das Terminal aus:

```bash
pio run
```

**Erwartetes Ergebnis:**

```
Environment       Status    Duration
----------------  --------  ----------
esp32s3-automotive  SUCCESS   XX.XXs
========================= 1 succeeded in XXs =========================
```

Jeder Build-Lauf muss mit `SUCCESS` abschliessen. Ein Fehlschlag ist ein Blocker und muss vor dem Fortfahren behoben werden.

### Schritt 4: Firmware flashen

Verbinden Sie das Board per USB-C und flashen Sie die Firmware:

```bash
pio run --target upload
```

> **Hinweis:** Falls das Board nicht erkannt wird, pruefen Sie den USB-Port mit `pio device list`.

### Schritt 5: Seriellen Monitor starten

Starten Sie den seriellen Monitor zur Laufzeitbeobachtung:

```bash
pio device monitor
```

Die Monitor-Konfiguration ist in `platformio.ini` definiert:

| Parameter | Wert | Bedeutung |
|---|---|---|
| `monitor_speed` | `115200` | Baudrate der seriellen Schnittstelle |
| `monitor_filters` | `direct` | Rohdaten ohne Transformation |
| | `esp32_exception_decoder` | Automatische Dekodierung von Stack-Traces bei Abstuerzen |

Der `esp32_exception_decoder`-Filter ist besonders wertvoll: Bei einem Absturz (Panic, Stack Overflow, etc.) werden die Speicheradressen im Stack-Trace automatisch in Dateinamen und Zeilennummern aufgeloest.

Beenden Sie den Monitor mit `Ctrl+C`.

### Build-Konfiguration im Detail

Die Build-Konfiguration in `platformio.ini` setzt bewusst strenge Compiler-Einstellungen:

```ini
build_unflags = -std=gnu++11 -std=gnu++14 -std=c11
build_flags =
    -std=c11
    -std=c++17
    -DCORE_DEBUG_LEVEL=3
    -Wall -Wextra -Werror
    -Wno-unused-parameter
    -fstack-protector-strong
```

| Flag | Bedeutung |
|---|---|
| `-std=c11` | C-Quellen werden im ISO C11 Standard kompiliert |
| `-std=c++17` | C++-Quellen werden im ISO C++17 Standard kompiliert |
| `-DCORE_DEBUG_LEVEL=3` | ESP-IDF Debug-Level auf `INFO` (0=None, 1=Error, 2=Warn, 3=Info, 4=Debug, 5=Verbose) |
| `-Wall` | Aktiviert alle gaengigen Compiler-Warnungen |
| `-Wextra` | Aktiviert zusaetzliche Warnungen ueber `-Wall` hinaus |
| `-Werror` | Behandelt **jede Warnung als Fehler** -- der Build schlaegt bei Warnungen fehl |
| `-Wno-unused-parameter` | Ausnahme: Unbenutzte Parameter werden toleriert (notwendig fuer FreeRTOS-Task-Signaturen) |
| `-fstack-protector-strong` | Aktiviert Stack-Schutz gegen Buffer-Overflow-Angriffe zur Laufzeit |

Die Kombination aus `-Wall -Wextra -Werror` erzwingt sauberen Code: Kein Commit darf Warnungen enthalten. Die `build_unflags`-Direktive entfernt zuvor gesetzte GNU-Standards, damit die expliziten ISO-Standards (`-std=c11`, `-std=c++17`) greifen.

`-fstack-protector-strong` fuegt Laufzeit-Pruefungen in Funktionen mit lokalen Puffern ein. Bei einem erkannten Stack-Buffer-Overflow wird die Ausfuehrung kontrolliert abgebrochen, anstatt undefiniertes Verhalten zuzulassen.

---

## 2.3 Statische Analyse ausfuehren (Quality Gate)

Die statische Analyse ist ein verpflichtender Qualitaets-Gate: Kein Code darf integriert werden, der High- oder Medium-Befunde erzeugt.

### Analyse starten

Fuehren Sie die statische Analyse aus:

```bash
pio check
```

### Konfiguration

Die Analyse-Werkzeuge sind in `platformio.ini` konfiguriert:

```ini
check_tool = cppcheck, clangtidy
check_flags =
    cppcheck: --std=c11 --std=c++17 --inline-suppr --enable=warning,style,performance,portability
    clangtidy: --checks=-*,cert-*,clang-analyzer-*,bugprone-*,performance-*,readability-*
```

#### cppcheck

| Flag | Bedeutung |
|---|---|
| `--std=c11 --std=c++17` | Analyse gegen die verwendeten Sprachstandards |
| `--inline-suppr` | Erlaubt Unterdrueckung einzelner Befunde direkt im Quellcode |
| `--enable=warning` | Erkennung potentieller Fehler |
| `--enable=style` | Stilverbesserungen und Best Practices |
| `--enable=performance` | Performance-Probleme (z.B. unnoetige Kopien) |
| `--enable=portability` | Portabilitaetsprobleme zwischen Plattformen/Compilern |

#### clang-tidy

Die Pruefungen sind nach Kategorien aktiviert:

| Kategorie | Bedeutung |
|---|---|
| `cert-*` | Regeln nach CERT C/C++ Coding Standard (Sicherheitskritische Konstrukte) |
| `clang-analyzer-*` | Clang Static Analyzer (Pfadbasierte Analyse fuer Null-Pointer, Memory Leaks, etc.) |
| `bugprone-*` | Erkennung fehleranfaelliger Muster (z.B. unbeabsichtigte Kopien, falsche Vergleiche) |
| `performance-*` | Performance-Anti-Patterns (z.B. unnoetige Kopien, ineffiziente Container-Nutzung) |
| `readability-*` | Lesbarkeitsregeln (z.B. konsistente Benennung, Magic Numbers) |

Das Praefix `-*` deaktiviert zuerst alle Pruefungen, anschliessend werden gezielt die genannten Kategorien aktiviert. Dieses Whitelist-Prinzip stellt sicher, dass nur beherrschbare Regelsaetze aktiv sind.

### Erwartetes Ergebnis

```
Checking esp32s3-automotive > cppcheck ...
Checking esp32s3-automotive > clangtidy ...
========================= [PASSED] Took XX.XXs =========================
```

### Interpretation der Ergebnisse

PlatformIO klassifiziert Befunde in drei Schweregrade:

| Schweregrad | Bewertung | Aktion |
|---|---|---|
| **High** | Kritischer Fehler oder Sicherheitsrisiko | Muss behoben werden -- Blocker |
| **Medium** | Potentieller Fehler oder schlechte Praxis | Muss behoben werden -- Blocker |
| **Low** | Stilistischer Hinweis oder Verbesserungsvorschlag | Sollte behoben werden -- kein Blocker |

**Regel:** Kein Code mit High- oder Medium-Befunden darf integriert werden.

### Fehlerbehebung und Unterdrueckung

#### Vorgehen bei Befunden

1. **Analysieren:** Lesen Sie die Meldung und den betroffenen Code
2. **Beheben:** Korrigieren Sie den Code, wenn der Befund berechtigt ist
3. **Erneut pruefen:** Fuehren Sie `pio check` erneut aus
4. **Unterdruecken:** Nur wenn der Befund ein False Positive ist (siehe unten)

#### Unterdrueckung mit Begruendung

In seltenen Faellen liefern die Analyse-Werkzeuge False Positives -- z.B. bei Hardware-nahen Konstrukten, die das Werkzeug nicht korrekt interpretieren kann. In solchen Faellen ist eine gezielte Unterdrueckung erlaubt, aber **nur mit dokumentierter Begruendung**.

**cppcheck** -- Unterdrueckung im Quellcode mit `// cppcheck-suppress`:

```cpp
// cppcheck-suppress missingReturn  // False Positive: noreturn-Funktion, Endlosschleife beabsichtigt
void run_safety_task(void* pvParameters) {
    while (true) {
        // ...
    }
}
```

**clang-tidy** -- Unterdrueckung im Quellcode mit `// NOLINT`:

```cpp
void app_main(void) {  // NOLINT(readability-identifier-naming) -- ESP-IDF API-Konvention
    // ...
}
```

> **Regel:** Jede Unterdrueckung muss eine Begruendung im Kommentar enthalten. Unterdrueckungen ohne Begruendung werden im Code-Review abgelehnt. Die Begruendung muss erklaeren, warum der Befund ein False Positive ist oder warum die Abweichung technisch notwendig ist.
