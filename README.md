# Time-critical Data Transmission – Ampelsteuerung mit Pico & FreeRTOS

Dieses Projekt demonstriert eine zeitkritische Datenübertragung mit einem Raspberry Pi Pico, bei der der Status einer Ampelschaltung via SPI an ein externes Gerät (Ampel) übertragen wird. Bei Übertragungsfehlern wird ein Fehlerzustand (Gelb blinkend) aktiviert.

## Inhalte

- **Ampelsteuerung:**  
  Simuliert die klassischen Ampelzustände: Rot, Rot-Gelb, Grün, Grün blinkend, Gelb und Gelb blinkend.  
  Jeder Zustand wird durch einen fest definierten 4-Bit-Code repräsentiert:
  - Rot: `1110`
  - Rot-Gelb: `1101`
  - Grün: `0010`
  - Grün blinkend: `0101`
  - Gelb: `1000`
  - Gelb blinkend: `0001`

- **SPI-Kommunikation:**  
  Übertragung der Ampelzustände über einen SPI-Bus (MOSI, MISO, SCLK, CS).

- **Fehlerüberwachung:**  
  Ein dedizierter Check-Task überwacht den SPI-Bus. Falls für mehr als 60 ms kein HIGH-Signal detektiert wird, schaltet sich die Ampel in den Fehlerzustand (Gelb blinkend).

- **FreeRTOS Integration:**  
  Zwei FreeRTOS-Tasks realisieren den Ampelzyklus und die SPI-Bus-Überwachung. Interrupts und ein Watchdog-Mechanismus stellen die Einhaltung der Zeitvorgaben sicher.


## Voraussetzungen

- **Hardware:** Raspberry Pi Pico (oder Pico W)  
- **Toolchain:** [pico-sdk](https://github.com/raspberrypi/pico-sdk)  
- **FreeRTOS:** Integriert im Projekt (siehe Ordner `freertos/FreeRTOS/`)  
- **CMake und Make:** Für Build Prozess

## Build & Deployment

1. **Repository klonen:**
   ```bash
   git clone https://github.com/TGM-HIT/syt5-gek1011-timecritical-datatransmission-ruf_stransky
   ```

2. **Build-Verzeichnis anlegen und konfigurieren:**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. **Kompilieren:**
   ```bash
   make
   ```

4. **Flashen:**
   - BOOTSEL-Button am pico drücken 

## Weiterführende Informationen

- **SPI-Bus:**  
  Lese mehr über die Architektur und Vorteile von SPI in der [offiziellen Dokumentation](https://www.ti.com/lit/an/spraac/spraac.pdf).

- **FreeRTOS:**  
  Detaillierte Informationen zu FreeRTOS findest du in der [FreeRTOS-Dokumentation](https://www.freertos.org/).

## Lizenz

Dieses Projekt steht unter der MIT-Lizenz – siehe [LICENSE](LICENSE) für Details.
