# Belegarbeit im Kurs Programmierung von Mikrocontrollern (I-223, Sommersemester 2025)
## Das Tic-Tac-Toe-Spiel
![Image](https://github.com/user-attachments/assets/423a0515-ce39-466c-907d-76c6e42309c8)

[Präsentationsvideo](https://www.informatik.htw-dresden.de/~s87413/mc/tic-tac-toe.webm) (webm-Muxer, vp9-Videocodec, opus-Audiocodec)

- Spielmodus: zwei Menschen spielen gegeneinander
  - der Algorithmus zum Spielen gegen den Mikrocontroller wurde nicht implementiert
- Benutzereingaben und Darstellung des Spielstandes:
  - Eingabe über ein auf der Schaltplatine montiertes 16-Tasten-Feld.
  - Ausgabe auf der eingebauten 8x12 LED-Matrix.
  - ein Teil der Statusmeldungen wird auf dem LCD-Display angezeigt.
  - Alle Ein- und Ausgaben sind auch über einen Webbrowser möglich.

## Eingesetzte Hardware
- Hardwareplatform: Arduino Uno R4 WiFi
  - Microcontroller: Renesas RA4M1 (Arm® Cortex®-M4) (48 MHz, 32 kB RAM, 256 kB Flash)
  - Wi-Fi/Bluetooth-Modul: ESP32-S3-MINI-1-N8
  - 12x8 LED-Matrix
  - [weitere Informationen auf arduino.cc](https://docs.arduino.cc/hardware/uno-r4-wifi/)
- 16-Tasten-Feld
  - verknüpft zu vier Zeilen auf vier Spalten
- 16x2 LCD-Display mit dem HD44780 Controller
- Soundmodul: 2-Pin-Buzzer

## Verwendete Bibliotheken
- [Arduino_LED_Matrix](https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/)
- [LiquidCrystal](https://docs.arduino.cc/libraries/liquidcrystal/)
- [WiFiS3](https://docs.arduino.cc/tutorials/uno-r4-wifi/r4-wifi-getting-started/)

## Projekt-Format
In dem Github-Repositorium ist das Verzeichnissstruktur unseres VSCode-PlatformIO-Projektes. Das Projekt lässt sich auch im Arduino-IDE öffnen indem man die `s87413_mc_beleg.ino` Datei als Sketch öffnet. Die Code befindet sich in der src/main.cpp Datei, die durch einblenden des Arduino-IDE-Explorers (Linux Ctrl+Shift+P, Windows: ?...) sichtbar wird.

## Tastatur-Interrupt
Das [R2R-Netzwerk](https://de.wikipedia.org/wiki/R2R-Netzwerk) aus den Resistoren ordnet jeder der 16 Tasten eine Spannung im Bereich von 0.5 bis 5 Volt. Über einen ADC-Pin des Arduinos wird die Spannung wird die Spannung abgelesen und in eine Tasten-Code übersetzt. Um bei Betätigen einer Taste einen Interrupt auszulösen wird die Spannung am Ausgang des Tastenfeldes kontinuirlich mit einer Referenzspannung von 0.3 Volt verglichen und beim Übersteigen ein Interrupt ausgelöst.
Als Comparator wird der im Arduino eingebauter Operationsverstärker verwendet

![Image](https://github.com/user-attachments/assets/6425c811-b0a6-446b-832c-18845320f380)

aktiviert wird er mit:
```
#include <OPAMP.h>

void setup () {
  OPAMP.begin(OPAMP_SPEED_HIGHSPEED);
}
```

## WiFi
Das WiFi-Modul des Arduino-Uno-R4 verbindet sich mit einem vorher definierten Hotspot (seine IP-Adresse wird auf dem LCD-Display angezeigt) und fängt an auf die HTTP-Anfragen auf dem Port 80 zu antworten.
Der Web-Client (im Videobeispiel ist das ein Handy) lädt vom Arduino eine Webseite herunter, die mithilfe von Ajax-Technik Benutzereingaben abfragt und den Spielstatus aktualisiert




