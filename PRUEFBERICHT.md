# Prüfbericht – Rollladensteuerung 1.6.0 (Web-Firmware-Update)

Datum: 21.10.2026. Basis: die von Nico erneut aktualisierten Dateien im Workspace, nicht der frühere ZIP-Stand.

## Erhaltener Benutzerstand

- Die Heap-Pufferkorrektur in `NidayandHelper.cpp` (statt 2-KB-Lesepuffer auf dem kleinen ESP-Stack) wurde unverändert übernommen.
- Die ergänzten Boot-/Resetdiagnosen im Sketch bleiben erhalten.
- `NidayandHelper.h/.cpp`, `BlindMotion.h`, `StatusSchedule.h` und `sketch.yaml` sind byteidentisch zur Sicherung des eingegangenen Benutzerstands.
- Vorhandene Build-Dateien und Logs wurden nicht überschrieben. Sicherung des vollständigen Eingangsstands lokal: `../.tmp/roller-blind-pre-web-update/`.

## Neue Funktion

Die Hauptseite enthält einen Firmware-Upload mit Version/Maximalgröße, Dateiauswahl, Bestätigung, Fortschritt und Erkennung des Geräte-Neustarts.

Der neue Streaming-Endpunkt akzeptiert nur Firmware (`U_FLASH`), stoppt vor dem Update den Motor und speichert dessen Position. Speicherfehler verhindern das Update. Während Upload/Neustart sind Fahrbefehle und Einstellungsänderungen gesperrt. Es wird kein Dateisystem-Upload angeboten.

Die tatsächliche Dateigröße wird gegen die angekündigte Größe geprüft. Ein Byte bleibt bis zum vollständigen HTTP-Abschluss zurückgehalten: unvollständige, abgebrochene oder mehrteilige Datei-Anfragen werden dadurch nicht versehentlich für eboot freigegeben. Erst nach erfolgreichem `Update.end(false)` wird geantwortet und zeitversetzt neu gestartet. Ein neuer Boot-Token dient im Browser zur Neustart-Erkennung; ein reiner Upload-Fortschritt von 100 % genügt nicht.

Header-/Längen- und Core-Flashprüfung sind keine kryptografische Signaturprüfung und kein vollständiger Nachweis, dass ein beliebiges Image das richtige Board-/Dateisystemlayout verwendet. Es muss eine passende, vertrauenswürdige Firmware gewählt werden.

## Firmware-Build

Erfolgreich mit dem tatsächlich verwendeten isolierten Arduino-CLI-Profil `modern` und aktivierten vollständigen Compilerwarnungen:

```sh
arduino-cli compile --profile modern --warnings all --output-dir build/esp8266.esp8266.nodemcu-1.6.0 .
```

- ESP8266-Core 3.1.2; ArduinoJson 7.4.3; WiFiManager 2.0.17; WebSockets 2.7.2; PubSubClient 2.8.0; Stepper_28BYJ_48 1.0.0.
- Board: `esp8266:esp8266:nodemcu:eesz=4M1M,xtal=80` (NodeMCU 0.9 / ESP-12).
- 4 MB Flash / 1 MB SPIFFS, QIO, Flash 40 MHz, CPU 80 MHz.
- RAM, globale/statische Daten: 35.972 / 80.192 Bytes (44 %).
- IRAM einschließlich Instruktionscache: 61.007 / 65.536 Bytes (93 %); 32.768 Bytes Cache und 28.239 Bytes IRAM-Code.
- Code im Flash: 435.776 / 1.048.576 Bytes (41 %).
- BIN-Dateigröße: 476.240 Bytes.
- BIN-SHA-256: `416bde5a1b4f35d35d040f807ca51ad7bfe7177fe21e44a72864505eb5454e2c`.

Keine Buildfehler. Bekannte Warnungsquellen sind der absichtlich beibehaltene SPIFFS-Unterbau (wenn der Helfer neu übersetzt wird), Signedness in PubSubClient sowie Python-Escape-Syntax im Core-Werkzeug `elf2bin.py`. Keine Fremdbibliotheken wurden geändert oder Warnungen pauschal unterdrückt.

## Erfolgreiche Prüfungen

### Tatsächlicher Streaming-Controller mit Fake-Flash-Writer

`tests/firmware_upload_test.cpp`, mit AddressSanitizer und UndefinedBehaviorSanitizer:

- Fragmentierung in 1-, 3-, 8-, 97- und 1024-Byte-Blöcke, einschließlich aufgeteilter Header.
- Byteidentische Ausgabe, erst nach HTTP-Abschluss freigegebener letzter Byte-/Commit-Schritt.
- Dateinamen und Größenlimits, zu lange/unvollständige Übertragung.
- Falscher Header und ESP32-Header abgewiesen, bevor Firmware-Bytes an den Writer gehen.
- Abbruch auch nach `FILE_END`, mehrere Dateien, erneuter Versuch nach Fehlern.
- Fehler bei Start, Flash-Schreiben und finaler Core-Prüfung.
- Zusätzlich wurde die tatsächlich kompilierte 476.240-Byte-BIN durch den Controller gestreamt und byteidentisch verglichen. Das ist ein Test mit Fake-Flash-Writer, kein Flashen.

### Echte HTML-Seite in Chrome mit simulierter Geräte-API

`tests/firmware_browser_test.cjs`:

- Version/Maximalgröße, Dateiauswahl und explizite Bestätigung.
- ZIP, zu große und ungültige Binärdateien vor dem Upload abgewiesen.
- Tatsächliche XMLHttpRequest-Multipart-Struktur, Dateibytes, Größen- und Boot-Token-Header geprüft.
- Fortschrittsdarstellung (Ereignis im Interceptiontest simuliert), Motor-/Einstellungssperre und Fehleranzeige.
- Fehlende Netzwerkbestätigung wird nicht als Erfolg angezeigt.
- Erfolgsantwort, Sperre während Neustart, alter Boot-Token gilt nicht als Neustart; neuer Token führt zum Neuladen.
- Gespeicherte Grenzwerte nach simuliertem Neustart weiter sichtbar; keine JavaScript-Fehler und kein horizontaler Mobil-Überlauf. Mobile Ansicht zusätzlich anhand Screenshot angesehen.

### Regressionen

- `tests/browser_test.cjs`: gespeicherte/live Werte, Endpunkte, Laufrichtung, MQTT-Passwort behalten/ersetzen/löschen, Fahrbefehle, Speicherfehler, Wiederverbindung und Entwürfe weiterhin bestanden.
- `tests/helper_test.cpp`: tatsächlicher aktualisierter Speicher-/MQTT-Helfer mit ArduinoJson 7 und Host-Mocks unter ASan/UBSan bestanden.
- `tests/motion_test.cpp`: Positions-/Grenzenlogik und 5-Sekunden-MQTT-Timer einschließlich `millis()`-Überlauf unter ASan/UBSan bestanden.

## Grenzen der Prüfung

Kein Upload auf einen echten ESP, kein physischer Neustart, kein echter MQTT-/WLAN-/OTA- oder Motorversuch. Die Webtests verwenden simulierte HTTP-Antworten; die nativen Streamingtests testen den echten Controller, aber nicht echte Flash-Hardware oder den ESP-HTTP-Parser. Die bestehende SPIFFS-/Flashaufteilung wird beibehalten. Der Erhalt physisch gespeicherter Einstellungen beim echten Upgrade ist nicht durch einen Hardwareversuch belegt.

Die neue Version muss einmal wie bisher installiert werden; erst dann steht der Web-Upload auf dem Gerät bereit. Nachfolgende Images müssen die Funktion ebenfalls enthalten, damit sie verfügbar bleibt.

## Arbeitsdaten

Build-/Testlogs und Screenshot liegen im ursprünglichen Workspace unter `../.tmp/roller-blind-tools/`, der neue Arbeitsbuild unter `../.tmp/roller-blind-firmware-webupdate/`. Der Liefer-ZIP enthält nur Quellen, Tests, Anleitung/Prüfbericht, Buildprofil und die neue BIN; keine Bootlogs, alten Binärdateien oder lokalen Werkzeuge.
