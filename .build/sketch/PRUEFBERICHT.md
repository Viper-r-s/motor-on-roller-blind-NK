#line 1 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\PRUEFBERICHT.md"
# Prüfbericht – Rollladensteuerung 1.5.0

Datum: 21.10.2026. Auftrag: Bibliotheken und ESP8266-Unterbau aktualisieren, Funktionen und gespeicherte Daten erhalten.

## Verwendete Versionen

Aktuelle stabile Releases laut aktualisiertem Arduino-Bibliotheksindex und offiziellen Releases:

- ESP8266 Arduino Core 3.1.2.
- ArduinoJson 7.4.3.
- WiFiManager 2.0.17.
- WebSockets 2.7.2.
- PubSubClient 2.8.0 und Stepper_28BYJ_48 1.0.0 sind weiterhin aktuell; keine neueren stabilen Releases vorhanden.

Die alten JSON-5-Schnittstellen wurden vollständig ersetzt. Gespeicherte JSON-Feldnamen und MQTT-Protokoll bleiben kompatibel. Der aktuelle WiFiManager erhält den Hostnamen über seine aktuelle Schnittstelle; der mDNS-Responder wird in der Hauptschleife bedient.

## Reale Firmware-Kompilierung

Erfolgreich mit dem mitgelieferten `sketch.yaml`, Profil `modern`, und `--warnings all`:

```sh
arduino-cli compile --profile modern --warnings all --output-dir build/esp8266.esp8266.nodemcu-1.5.0 .
```

Das Profil installiert die angegebenen Abhängigkeiten isoliert. Damit wurde nicht versehentlich gegen noch vorhandene alte Bibliotheken gebaut.

- Ziel: `esp8266:esp8266:nodemcu:eesz=4M1M,xtal=80` (NodeMCU 0.9 / ESP-12).
- Flashmodus QIO, Flashfrequenz 40 MHz, CPU 80 MHz.
- RAM, globale/statische Daten: 34.860 / 80.192 Bytes (43 %).
- IRAM inklusive reserviertem Instruktionscache: 61.007 / 65.536 Bytes (93 %); davon 32.768 Bytes Cache und 28.239 Bytes IRAM-Code.
- Code im Flash: 427.172 / 1.048.576 Bytes (40 %).
- BIN-Abbildgröße: 466.560 Bytes.
- BIN-SHA-256: `6f263699ea05e02d76c78a43be1d4a7ce4bbe74bac4b8e06a1586378b437d63c`.

Keine Buildfehler. Bei aktivierten vollständigen Warnungen: 15 SPIFFS-Deprecation-Hinweise (bewusst für Alt-Daten beibehalten), eine Signedness-Warnung in PubSubClient 2.8 und zwei Python-SyntaxWarnings im Core-Werkzeug `elf2bin.py`. Diese stammen nicht aus der JSON-Migration. Es wurden keine Fremdbibliotheken geändert oder Warnungen pauschal unterdrückt.

## Speicherkompatibilität

Das bisherige Dateisystem bleibt SPIFFS, nicht LittleFS. Ein Wechsel des On-Flash-Formats ohne Datenmigration würde bestehende Konfigurationen gefährden.

- Vorhandener alter Bereich: `0x40500000` bis `0x405fb000`.
- Neues 1-MB-FS-Profil: `0x40500000` bis `0x405fa000`.
- Gleicher Anfang, 256 Bytes Seitengröße und 8192 Bytes Blockgröße.
- Beide Bereiche umfassen 125 vollständige SPIFFS-Blöcke; der alte zusätzliche 4-KB-Rest ist kein zusätzlicher Block.

**Offline-Abbildprüfung:** Mit dem originalen `mkspiffs` aus dem 2.4.2-Paket wurde ein Abbild einschließlich Legacy-`config.json` erzeugt. Die zusätzlichen 4096 Bytes waren vollständig gelöscht (`0xFF`). Nach Begrenzung auf den neuen Bereich wurde es mit dem `mkspiffs` des 3.1.2-Cores gelesen; die extrahierte `config.json` war byteidentisch. Dies ist kein Mounttest auf echter Hardware.

Die neue Speicherinitialisierung schaltet automatisches Formatieren bei Mountfehlern aus. Nur wenn der gesamte ausgewählte Bereich erfolgreich als gelöscht gelesen wurde, wird ein frisches Dateisystem initialisiert. Vorhandene oder nicht lesbare Daten werden nicht formatiert. Bei falscher Flashaufteilung zeigt die Oberfläche einen Speicherfehler.

## Erfolgreiche Softwareprüfungen

### Native C++-Tests, AddressSanitizer und UndefinedBehaviorSanitizer

- Tatsächliche `BlindMotion`- und `StatusSchedule`-Klassen: strikte Befehlsprüfung, ungültige Grenzen, Offset-Koordinaten, automatische Ziele in beide Richtungen, manuelles Stoppen, große Werte und Rechenüberläufe.
- 5.000-ms-MQTT-Termin während der Fahrt, keine periodischen Termine im Stillstand, unabhängige Webupdates und `millis()`-Überlauf.
- Tatsächliche `NidayandHelper.cpp` mit ArduinoJson **7.4.3** und Host-Peripheriemocks: Lebensdauer der geparsten Daten, alte Feldnamen, Speichern/Laden, Sonderzeichen, begrenzte Dateigrößen, Zurückweisen nicht objektförmiger JSON-Wurzeln, Backup-Wiederherstellung und fehlgeschlagene/partielle Schreibvorgänge.
- Speicherinitialisierung: vorhandenes Volume einbinden, nichtleeren Flash nicht formatieren, Lesefehler/ungültige Grenzen abweisen, vollständig gelöschten Bereich initialisieren.
- MQTT-Wiederverbindungsbegrenzung, Verhalten ohne WLAN, Anmeldung und Topic-Subscription.

### Browserprüfung

Tatsächliche HTML-Seite in headless Chrome, mobile Ansicht (420 px), mit **simulierter Geräte-API**:

- Gespeicherte und laufende Werte getrennt anzeigen.
- Grenzen direkt ändern, aktuelle Position übernehmen, ungültige Eingaben abweisen.
- Entwürfe bei Hintergrundupdates und Wiederverbindung erhalten.
- Laufrichtung ändern.
- MQTT-Host/Port/Benutzer ändern; Passwort behalten/ersetzen/löschen, ohne es anzuzeigen.
- Manuelle Befehle, Stoppen, Prozentziel, Speicherfehler, Unterbrechung/Wiederverbindung und Neuladen gespeicherter Werte.
- HTML-sichere Gerätenamen, kein horizontaler Mobil-Überlauf und keine JavaScript-Laufzeitfehler.

## Nicht durchgeführt

Kein Flashen, keine physische Motorfahrt und kein Test über einen echten MQTT-Broker, OTA oder das reale WLAN. API-/Browserprüfungen verwenden einen simulierten Server, Flash-/Netzwerkprüfungen des Helfers Host-Mocks. Die Zeitplanung ist keine Echtzeitgarantie bei Netzstörungen. Die Gerätekompatibilität nach einem Core-Sprung muss beim tatsächlichen Aufspielen kontrolliert werden.

## Erhaltener Altbestand und Arbeitsdaten

Die ursprünglich vorhandenen Build-Dateien und Version 1.4.0 wurden nicht überschrieben.

- Original-BIN-SHA-256: `8a0393b7a91dac3e2403af67608a65ae2a26e3e0e2796b376fd14c543d5f2f26`.
- 1.4.0-BIN-SHA-256: `deee1486c6a237e477c000bb263cfb1ed4854cb9813cad5608c7b4c274453dd3`.
- Quelltext-Sicherung vor der Modernisierung im ursprünglichen Workspace: `../.tmp/roller-blind-v1.4.0/`.
- Werkzeuge, Logs, Screenshot und SPIFFS-Abbildprüfung: `../.tmp/roller-blind-tools/`.
- Lieferarchiv enthält nur den neuen Quellstand und die neue BIN, keine alten Build-Abbilder und keine temporären Werkzeuge.
