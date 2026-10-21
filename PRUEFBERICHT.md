# Prüfbericht – Rollladensteuerung 1.7.0

Datum: 22.10.2026. Auftrag: Gerätenamen über die Weboberfläche ändern und die Oberfläche auf Englisch umstellen.

## Umsetzung

- Neue englische Einstellung **Device name / hostname** und Schaltfläche **Save name & restart**.
- Neue API `POST /api/name`, gespeichert im vorhandenen `config_name`-Feld. Namen werden getrimmt, kleingeschrieben und als einzelnes DNS-Label mit 1–31 Zeichen geprüft.
- Nur Buchstaben, Ziffern und Bindestriche, kein führender/abschließender Bindestrich, keine Leerzeichen, Punkte oder `.local`-Endung.
- Bei Änderung: Motor stoppen, aktuelle Position und Name erfolgreich speichern, Antwort mit neuem Namen/Links senden, nach 1.200 ms kontrolliert neu starten. Der Neustart sorgt dafür, dass der Name für DHCP/WLAN, mDNS und ArduinoOTA beim Initialisieren verwendet wird; `ArduinoOTA.setHostname` erlaubt in diesem Core keinen nachträglichen Wechsel an der bereits initialisierten Instanz.
- Bei Speicherfehler kein neuer aktiver Name und kein geplanter Neustart. Unveränderter Name ist eine erfolgreiche Anfrage ohne Neustart.
- Während des ausstehenden Neustarts werden Motorbefehle und Änderungen gesperrt, einschließlich neuer Firmware-Uploads.
- Die Webseite zeigt die neue `.local`-Adresse und die IP-Adresse als Links an. Bei Aufruf über den alten Namen kann der Client die Verbindung verlieren; die neuen Links dienen dann zum erneuten Öffnen. Physische Namensauflösung ist nicht getestet.
- Feste englische Weboberfläche einschließlich serverseitiger Speicher-, MQTT-, Befehls- und Firmware-Upload-Fehlermeldungen; keine neue Laufzeitabhängigkeit und kein externer Übersetzungsdienst.
- Min/Max, Position, Laufrichtung, MQTT-Felder/Topics, Flashaufteilung und Bibliotheksversionen bleiben kompatibel. Heap-Lesepufferkorrektur und Bootdiagnose sind weiterhin vorhanden. Vorhandene Logs und alte Build-Artefakte wurden nicht ersetzt.

## Tatsächlich kompilierte Firmware

Isolierter Arduino-CLI-Profilbuild `modern` mit `--warnings all` erfolgreich:

- ESP8266-Core 3.1.2; ArduinoJson 7.4.3; WiFiManager 2.0.17; WebSockets 2.7.2; PubSubClient 2.8.0; Stepper_28BYJ_48 1.0.0.
- FQBN: `esp8266:esp8266:nodemcu:eesz=4M1M,xtal=80` (NodeMCU 0.9 / ESP-12).
- QIO, 40 MHz Flash, 80 MHz CPU, 4 MB Flash / 1 MB SPIFFS.
- Globale/statische RAM-Daten: 36.056 / 80.192 Bytes (44 %).
- IRAM einschließlich 32-KB-Instruktionscache: 61.007 / 65.536 Bytes (93 %), davon 28.239 Bytes IRAM-Code.
- Code im Flash: 439.476 / 1.048.576 Bytes (41 %).
- BIN-Dateigröße: 480.016 Bytes.
- BIN-SHA-256: `5e302c330d71b3764bcf14b827b20016f5284df8c7363950b93e9be6bad73f19`.

Keine Buildfehler. Bekannte Warnungsquellen im Softwarestand sind SPIFFS-Deprecation, PubSubClient-Signedness und Python-Escapes in `elf2bin.py`; tatsächliche Warnungen im inkrementellen Log hängen vom Übersetzungscache ab. Keine Fremdbibliotheken wurden dafür geändert oder Warnungen pauschal unterdrückt.

## Erfolgreiche Softwareprüfungen

### Native Tests unter AddressSanitizer/UndefinedBehaviorSanitizer

- `device_name_test.cpp`: echte Namensvalidierung, zulässige Namen, 31-/32-Zeichen-Grenze, ungültige/unsichere Zeichen, Bindestrichpositionen, UTF-8 und Null-/Leereingaben.
- `helper_test.cpp`: tatsächlicher Speicher-/MQTT-Helfer mit ArduinoJson 7 und Host-Peripheriemocks; Name und beide Positionsgrenzen nach Speichern in frischer Helferinstanz unverändert gelesen. Weiterhin Schreibfehler, Backup-Wiederherstellung, Speicherinitialisierung und MQTT-Wiederverbindungsbegrenzung.
- `motion_test.cpp`: bestehende Positionslogik und 5-Sekunden-MQTT-Timer einschließlich `millis()`-Überlauf.
- `firmware_upload_test.cpp`: bestehender tatsächlicher Streaming-Controller mit Fake-Flash-Writer; außerdem die neu kompilierte BIN vollständig durch den Controller gestreamt und byteidentisch verglichen.

### Browserprüfungen: echte Seite, simulierte Geräte-API

- `name_browser_test.cjs`: `lang=en`, englische Abschnitte/Status, gespeicherter Name und Links; ungültige Eingaben blockiert; Entwürfe bei Hintergrundaktualisierung erhalten; englischer Speicherfehler erhält alten Namen und Entwurf; unveränderter Name ohne Neustart; neuer Name kleingeschrieben; Motor/Einstellungen/Upload während Neustart gesperrt; erst neue Bootkennung löst Reload aus; Grenzen und MQTT-Einstellungen bleiben sichtbar.
- `browser_test.cjs`: vollständige bestehende Steuerungs-/Positions-/Richtungs-/MQTT-Bedienung einschließlich Speicherfehler und Wiederverbindung gegen die englische Seite bestanden.
- `firmware_browser_test.cjs`: bestehender Web-Upload einschließlich Multipart-Daten, Dateifilter, Sperre, Fehler, Fortschritt und Neustart-Erkennung gegen die englische Seite bestanden. Fortschritt im Interceptiontest simuliert.
- Keine JavaScript-Laufzeitfehler und kein horizontaler Überlauf bei 420 px Mobilbreite. Screenshot der englischen Seite mit geändertem Namen zusätzlich angesehen.

## Nicht getestet

Kein Flashen, keine physische Motorfahrt, kein realer ESP-Neustart und keine echte DHCP-/mDNS-/Router-/ArduinoOTA-Namensübernahme. Die Browserprüfung simuliert Antwort und Neustart; der Speichertest verwendet Host-Mocks statt echtem SPIFFS. Erfolgreiche Kompilierung ist kein Hardware-Kompatibilitätsnachweis. MQTT/WLAN/OTA und Erhalt der Daten auf dem Gerät sind nicht durch einen neuen Hardwareversuch bestätigt.

## Dateien und Lieferung

- Neues Abbild: `build/esp8266.esp8266.nodemcu-1.7.0/motor_on_a_roller_blind-ws.ino.bin`.
- Der vollständige Ausgangsstand liegt lokal unter `../.tmp/roller-blind-pre-name-english/`.
- Build und Logs: `../.tmp/roller-blind-firmware-name-english/` und `../.tmp/roller-blind-tools/`.
- Das neue ZIP enthält Quellen, Tests, Anleitung/Prüfbericht, Buildprofil und nur die neue BIN. Keine alten Abbilder, seriellen Logs oder privaten Konfigurationsdateien.
- Falls 1.6.0 bereits installiert ist, kann die BIN über dessen **Firmware update** eingespielt werden. Andernfalls zunächst wie bisher USB/ArduinoOTA.
