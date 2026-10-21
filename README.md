# Rollladensteuerung – ESP8266, Version 1.7.0

Überarbeitete Fassung von [motor-on-roller-blind-ws](https://github.com/nidayand/motor-on-roller-blind-ws), MIT-Lizenz. Bestehende Motorverdrahtung bleibt unverändert: `Stepper_28BYJ_48(D1, D3, D2, D4)`.

## Neu in 1.7.0: Gerätename und englische Oberfläche

Die gesamte Geräte-Weboberfläche ist jetzt **Englisch**: Beschriftungen, Hilfetexte, Status-/Fehlermeldungen, MQTT- und Firmware-Upload-Dialog. Die Sprache ist fest Englisch, ein Sprachumschalter ist nicht erforderlich. Eigene Gerätenamen und andere gespeicherte Benutzereingaben werden natürlich nicht übersetzt.

Unter **Device name** lässt sich der Name direkt ändern:

1. In **Device name / hostname** den gewünschten Namen eingeben, z. B. `bedroom-blind`.
2. **Save name & restart** drücken. Erlaubt sind 1–31 ASCII-Buchstaben, Ziffern und Bindestriche; keine Leerzeichen, Punkte oder `.local`-Endung und kein Bindestrich am Anfang/Ende. Neue Namen werden kleingeschrieben gespeichert.
3. Bei einer tatsächlichen Änderung wird der Motor gestoppt, die aktuelle Position zusammen mit dem Namen gespeichert und nach der HTTP-Antwort ein kurzer Neustart ausgeführt. So verwenden DHCP/WLAN, mDNS und ArduinoOTA beim nächsten Start denselben Namen. Ein unveränderter Name löst keinen Neustart aus.
4. Die Seite zeigt den neuen **Device address**-Link, z. B. `http://bedroom-blind.local/`, sowie die aktuelle **IP address** als Ausweichadresse an. Wurde die Seite über den alten `.local`-Namen geöffnet, bei Bedarf nach dem Neustart den neuen Link oder die IP verwenden. Ob `.local` aufgelöst wird und wann der Router den DHCP-Namen anzeigt, hängt auch vom Netzwerk/Client ab.

Der Name bleibt im vorhandenen Feld `config_name` in `/config.json` gespeichert und wird nach Neustarts wieder geladen. Positionsgrenzen, Richtung, MQTT-Einstellungen und chipbasierte MQTT-Topics bleiben unverändert. Ein Speicherfehler übernimmt den neuen Namen nicht und löst keinen Neustart aus. Eingabeentwürfe bleiben bei Hintergrundupdates erhalten.

Die Bootdiagnose und die Heap-Pufferkorrektur des aktualisierten Benutzerstands sind weiterhin vorhanden. Bibliotheken, Flashaufteilung und Dateisystemformat wurden für diese Änderung nicht gewechselt.

**Installation:** Wenn bereits 1.6.0 mit Web-Upload auf dem Gerät läuft, kann die neue `motor_on_a_roller_blind-ws.ino.bin` im Bereich **Firmware update** hochgeladen werden. Ohne Web-Upload wie bisher USB/ArduinoOTA verwenden. Das ZIP nicht als Firmware auswählen.

## Neu in 1.6.0: Firmware über die Weboberfläche aktualisieren

Die vorhandenen Funktionen bleiben erhalten. Zusätzlich gibt es unten auf der Hauptseite den Bereich **Firmware update** mit Dateiauswahl, Upload-Fortschritt und automatischer Wiederverbindung nach dem Neustart.

**Einmalige Installation dieser Version:** Die bisherige Firmware ohne Web-Upload kann diese neue Seite noch nicht anbieten. Version 1.6.0 daher einmal wie bisher per Arduino IDE/USB oder über das schon vorhandene ArduinoOTA aufspielen. Danach sind weitere Updates direkt über die Webseite möglich, solange die zukünftig hochgeladene Firmware diese Funktion ebenfalls enthält.

### Bedienung

1. Die normale Geräte-Webseite öffnen und zu „Firmware update“ scrollen. Aktuelle Version und maximal zulässige Dateigröße werden angezeigt.
2. Eine **unkomprimierte `.bin`-Firmware für den verwendeten ESP8266 und dessen Flashlayout** auswählen. Beispielsweise die mitgelieferte Datei `build/esp8266.esp8266.nodemcu-1.7.0/motor_on_a_roller_blind-ws.ino.bin`. Nicht das ZIP-Archiv, kein Dateisystem-Abbild und keine ESP32-Firmware auswählen.
3. Die Checkbox für Motorstopp/Neustart bestätigen und „Upload & install firmware“ drücken.
4. Der Motor wird gestoppt und die aktuelle Position gespeichert. Kann nicht gespeichert werden, wird der Upload mit einer Fehlermeldung abgebrochen.
5. Während der Übertragung sind Motorsteuerung und Einstellungsänderungen gesperrt. Die Stromversorgung nicht unterbrechen.
6. Erst nach vollständiger Übertragung und erfolgreichem Abschluss wird der Neustart ausgelöst. Die Seite wartet auf eine neue Geräte-Bootkennung und lädt sich anschließend neu. 100 % Upload allein wird **nicht** als erfolgreiches Update oder Neustart interpretiert.

Min-/Max-Position, letzte gespeicherte Position, Richtung, MQTT- und WLAN-Einstellungen werden bei diesem **Firmware-only-Upload** nicht gelöscht. Die neue Firmware muss weiterhin dasselbe Dateisystem/Flashlayout verwenden und die gespeicherten Felder verstehen. Eine beliebige andere Firmware kann dieses Verhalten nicht garantieren.

### Verhalten bei Fehlern

- Dateiendung, Mindest-/Maximalgröße und der unkomprimierte ESP8266-Header werden vor dem Schreiben geprüft. Die Flash-Prüfungen des ESP8266-Updaters bleiben aktiv. Das ist keine kryptografische Signaturprüfung oder vollständige Prüfung, dass jedes beliebige Image zur Mechanik und Flashaufteilung dieses Geräts passt.
- Es wird nur der Firmware-Bereich (`U_FLASH`) aktualisiert; ein Dateisystem-Upload ist nicht vorgesehen. Die Datei wird gestreamt, nicht vollständig im ESP-RAM gehalten.
- Der Server vergleicht die tatsächliche Übertragungslänge mit der vorher angekündigten Dateigröße. Ein letztes Byte bleibt bis zum erfolgreichen Abschluss der **gesamten HTTP-Anfrage** zurückgehalten. Dadurch wird auch ein Abbruch nach `FILE_END` oder eine zweite Datei im selben Formular nicht versehentlich zur Installation freigegeben.
- Bei abgebrochenen/unvollständigen Anfragen oder Schreibfehlern erfolgt kein Neustart und die bisherige Firmware bleibt aktiv. Der Motor fährt nicht automatisch weiter.
- Geht nur die HTTP-Erfolgsantwort verloren, ist der Ausgang aus Browsersicht unbestätigt: erst Verbindung/Geräteversion prüfen, nicht blind erneut hochladen.
- Der Upload nutzt dieselbe lokale, unverschlüsselte Weboberfläche wie die Steuerung. Ein pro Start erzeugter Token und benutzerdefinierte Upload-Header verhindern einfache fremde Browser-Formularanfragen; es gibt dadurch aber keine zusätzliche Benutzeranmeldung oder signierte Firmware. Für das vertraute lokale Netz vorgesehen.

Deine zuletzt aktualisierte Heap-Pufferkorrektur beim Konfigurationslesen sowie die Bootdiagnose wurden beibehalten. Bibliotheken, Pins, Schritt-Einheit, Flashprofil und SPIFFS-Format wurden für dieses Feature nicht geändert.

## Aktualisierung auf aktuelle Bibliotheken (1.5.0)

Der Quellcode ist auf **ESP8266-Core 3.1.2 und ArduinoJson 7.4.3** migriert. Alte `JsonBuffer`-/`parseObject`-/`printTo`-Schnittstellen sind durch `JsonDocument`, `deserializeJson` und `serializeJson` ersetzt. WiFiManager 2.0.17 und WebSockets 2.7.2 werden verwendet. PubSubClient 2.8.0 und Stepper_28BYJ_48 1.0.0 sind bereits die neuesten stabilen Releases und bleiben deshalb unverändert. Stand: 21.10.2026.

Das neue `sketch.yaml` enthält ein reproduzierbares Arduino-CLI-Profil mit allen getesteten Versionen und **explizit 4 MB Flash / 1 MB FS**. Die alten Bibliotheken sind nicht mehr erforderlich. Die fünf funktionalen Erweiterungen aus 1.4.0 bleiben erhalten.

**Bewusste Kompatibilitätsausnahme: SPIFFS.** Der aktuelle ESP8266-Core enthält SPIFFS weiterhin, markiert es aber als veraltet. Es bleibt hier erhalten, weil ein bloßer Wechsel auf LittleFS die vorhandenen Konfigurationsdateien nicht lesen könnte. Deshalb können beim Kompilieren SPIFFS-Deprecation-Warnungen erscheinen; sie sind kein ArduinoJson-Kompatibilitätsfehler. Es findet keine automatische LittleFS-Konvertierung statt. Siehe [ESP8266-Dateisystem-Dokumentation](https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html).

## Funktionen

- Deutsche, mobil nutzbare Weboberfläche ohne externe Skripte, Stylesheets oder Internetverbindung.
- Anzeige der **zuletzt erfolgreich gespeicherten Werte**, getrennt von der laufenden Schrittposition: Anfang, Ende, Position, Schließrichtung, MQTT-Broker, Port, Benutzername und Passwortstatus.
- Anfang und Ende jederzeit direkt in Schritten ändern oder die aktuelle Position per Taste übernehmen. Vor dem Speichern wird der Motor gestoppt. Das Speichern startet keine Fahrt.
- Schließrichtung (CW/CCW) nachträglich ohne Neustart ändern; der Motor wird vorher gestoppt.
- MQTT-Host, Port und Zugangsdaten über dieselbe Oberfläche ändern, ohne WLAN-Einstellungen zurückzusetzen und ohne Neustart. Leerer Host deaktiviert MQTT.
- MQTT-Status bei Start und Stopp, bei Verbindung sowie **alle 5 Sekunden während einer Fahrt**. Die Oberfläche erhält während einer Fahrt zusätzlich ungefähr jede Sekunde einen WebSocket-Status.
- Behobene Altfehler: ungültige Lebensdauer der geladenen JSON-Daten, fehlende Nullterminierung, Laden erst nach dem WLAN-Portal, falsche Vorgaben für MQTT-Benutzer/Passwort, Überläufe/Division durch null in Positionsberechnungen und endlos blockierende MQTT-Verbindungsversuche.

## Bedienung und Positionen

1. Falls das Gerät noch kein WLAN kennt, mit dem Access Point **F1** verbinden. Das bestehende AP-Passwort bleibt `12345678`. WLAN über das Konfigurationsportal einrichten.
2. Im normalen WLAN `http://<IP-Adresse>/` oder `http://<Gerätename>.local/` öffnen. Standardname bei neuer Konfiguration: `roller-blind`.
3. Unter „Adjust start and end positions“ manuell zur offenen Position fahren, stoppen und „Set current as start (open)“ drücken. Danach zur geschlossenen Position fahren, stoppen und „Set current as end (closed)“ drücken.
4. Alternativ beide Werte direkt bearbeiten und „Save limits“ drücken. Anfang muss kleiner als Ende sein. Zulässig sind ganze Schrittkoordinaten von −1.000.000.000 bis +1.000.000.000.
5. Die normalen Öffnen-/Schließen-Tasten und der Positionsregler verwenden die gespeicherten Grenzen. 0 % bedeutet offen/Anfang, 100 % geschlossen/Ende.

**Koordinaten bleiben erhalten:** Wird eine andere Stelle als Anfang gespeichert, wird die aktuelle Position nicht auf null zurückgesetzt; der Endpunkt verschiebt sich nicht mit. Prozentwerte werden aus `(Position − Anfang) / (Ende − Anfang)` berechnet. Beide Endpunkte können dadurch unabhängig geändert werden. Bei ungünstigen alten Grenzen beide Zahlen gemeinsam eingeben.

Die Schritt-Einheit ist dieselbe wie im Altprojekt: ein Aufruf von `Stepper_28BYJ_48.step(1)` (die Bibliothek führt dabei acht Halbschritte aus).

**Manuelles Fahren ist wie bisher unbegrenzt** und dient zur Kalibrierung. Es gibt keine Hardware-Endschalter oder Positionssensoren. Deshalb manuelle Fahrten beobachten und stoppen; nach einer Richtungsumstellung die Endpunkte prüfen. Die Prozentanzeige wird auf 0–100 % begrenzt, während die rohe Schrittkoordinate auch außerhalb der gespeicherten Grenzen sichtbar bleibt.

## Speicherung und Übernahme bestehender Werte

- Weiterhin SPIFFS und `/config.json`, keine Umstellung auf LittleFS. Ein fehlgeschlagener Mount formatiert **keine vorhandenen Daten**. Nur ein vollständig als gelöscht (alle Bytes `0xFF`) geprüfter Flash-Dateisystembereich wird beim Erstbetrieb initialisiert. Bei falschem Flashlayout oder beschädigtem vorhandenen Dateisystem bleibt die Speicherung gesperrt und die Oberfläche meldet den Fehler.
- Alte Konfigurationen werden übernommen; fehlt `startPosition`, gilt wie bisher Anfang = 0. WLAN-Zugangsdaten bleiben separat im ESP erhalten.
- Flash-Schreiben nur beim Stoppen, Erreichen eines Ziels oder Speichern einer Einstellung – nicht bei jedem Schritt oder 5-Sekunden-Update.
- Erst wird eine vollständige `/config.tmp` geschrieben und geschlossen, dann die alte Datei nach `/config.bak` verschoben und die neue installiert. Bei fehlender/defekter Hauptdatei wird die Sicherung gelesen. Das ist eine Wiederherstellungsstrategie, keine Garantie gegen jeden Spannungsabfall.
- Fehler beim Speichern werden an die Oberfläche gemeldet; neue Einstellungen werden erst nach erfolgreichem Schreiben wirksam. Die bisher gespeicherten Werte bleiben erhalten. Der Motor bleibt gestoppt.
- Ohne Positionssensor kann nach Stromausfall während einer Fahrt nur die letzte gespeicherte Position wiederhergestellt werden; dann gegebenenfalls neu kalibrieren.

## MQTT

- Anmeldung/Registrierung: `/raw/esp8266/register` mit Chip-ID und IP-Adresse.
- Befehle: `/raw/esp8266/<chip-id>/in`.
- Status: `/raw/esp8266/<chip-id>/out`.
- Die Topics werden in der Oberfläche angezeigt und sind weiterhin chipbasiert, nicht frei konfigurierbar.

### Befehle (auch weiterhin per WebSocket auf Port 81)

| Nachricht | Wirkung |
| --- | --- |
| `(start)` | Stoppen und aktuelle Schrittkoordinate als Anfang speichern |
| `(max)` | Stoppen und aktuelle Schrittkoordinate als Ende speichern |
| `(0)` | Stoppen und Position speichern |
| `(-1)` | Manuell öffnen, ohne Endbegrenzung |
| `(1)` | Manuell schließen, ohne Endbegrenzung |
| `0` bis `100` | Automatisch zur Prozentposition zwischen Anfang und Ende fahren |
| `(update)` | Aktuellen Status anfordern |
| `(ping)` | Keine Bewegung |

Ungültige Nachrichten lösen keine Fahrt aus. `set` und `position` bleiben Prozentwerte, zusätzliche Felder enthalten Schrittkoordinaten und Bewegungszustand:

```json
{"type":"status","set":100,"position":40,"currentPosition":500,"targetPosition":1100,"startPosition":100,"maxPosition":1100,"moving":true,"direction":"down","clockwise":false,"mqttConnected":true,"configRevision":1,"saveError":""}
```

Die 5-Sekunden-Meldungen setzen eine bestehende MQTT-Verbindung voraus. Bei Ausfall läuft die Motorsteuerung weiter. Neue MQTT-Verbindungsversuche erfolgen höchstens alle 5 Sekunden und nur im Stillstand, weil der TCP-Verbindungsaufbau synchron ist; eine bestehende Verbindung wird auch während der Fahrt bedient. PubSubClient-Socket-Timeout: 1 Sekunde. Das Intervall ist per `millis()` geplant, kein harter Echtzeit-Takt bei Netzstörungen. Normale QoS-0-Statusnachrichten, nicht retained.

Ein leeres Passwortfeld beim Speichern **behält das bisherige Passwort**. Zum Entfernen die gesonderte Checkbox setzen. Das Passwort wird weder in Statusdaten noch in der Konfigurationsantwort oder seriellen Konfigurationslogs ausgegeben. Wie bisher liegen Zugangsdaten im SPIFFS-Flash; HTTP, WebSocket und MQTT sind unverschlüsselt und ohne zusätzliche Web-Anmeldung für das vertraute lokale Netz vorgesehen.

## Bauen und Aufspielen

Die vollständige Firmware wurde mit diesen aktuellen stabilen Releases kompiliert:

| Komponente | Getestete Version | Offizielle Quelle |
| --- | --- | --- |
| ESP8266 Arduino Core | **3.1.2** | [Releases](https://github.com/esp8266/Arduino/releases/tag/3.1.2) |
| ArduinoJson | **7.4.3**, nicht mehr 5.x | [Release](https://github.com/bblanchon/ArduinoJson/releases/tag/v7.4.3) |
| Stepper_28BYJ_48 | 1.0.0 (weiterhin aktuell) | [Projekt](https://github.com/thomasfredericks/Stepper_28BYJ_48) |
| PubSubClient | 2.8.0 (weiterhin aktuell) | [Release](https://github.com/knolleary/pubsubclient/releases/tag/v2.8) |
| WiFiManager | **2.0.17** | [Release](https://github.com/tzapu/WiFiManager/releases/tag/v2.0.17) |
| WebSockets / Links2004 | **2.7.2** | [Release](https://github.com/Links2004/arduinoWebSockets/releases/tag/2.7.2) |

### Arduino IDE

1. In der Boardverwaltung das Paket **esp8266 by ESP8266 Community 3.1.2** installieren/aktualisieren. Falls nötig diese zusätzliche Boardverwalter-URL eintragen: `https://arduino.esp8266.com/stable/package_esp8266com_index.json`.
2. Über die Bibliotheksverwaltung die Versionen aus der Tabelle installieren bzw. aktualisieren. Alte ArduinoJson-5-Kopien im Sketchordner oder doppelte manuelle Bibliotheksinstallationen entfernen/auslagern, wenn die IDE sonst die falsche Version auswählt.
3. Den Ordner `motor_on_a_roller_blind-ws` mit allen Dateien zusammen verwenden und `motor_on_a_roller_blind-ws.ino` öffnen.
4. Board **NodeMCU 0.9 (ESP-12 Module)**, **80 MHz**, Flash Size **4MB (FS:1MB OTA:~1019KB)** und Erase Flash **Only Sketch** wählen. Der aktuelle Core verwendet standardmäßig teilweise 2 MB FS – für dieses vorhandene Gerät deshalb die 1-MB-Einstellung ausdrücklich wählen.
5. Kompilieren und bei Bedarf regulär hochladen. Kein Dateisystem-Image hochladen und keinen vollständigen Flash-Erase durchführen, wenn vorhandene Einstellungen erhalten bleiben sollen.

Die HTML-Seite liegt in der Firmware. Die Motorpins und die logische Schritt-Einheit wurden nicht verändert.

### Arduino CLI – reproduzierbarer Build

Mit einer aktuellen Arduino CLI im Projektordner:

```sh
arduino-cli compile --profile modern --warnings all --output-dir build/esp8266.esp8266.nodemcu-1.7.0 .
```

Das Profil in `sketch.yaml` installiert die festgelegten aktuellen Versionen bei Bedarf isoliert und wählt das richtige Flashlayout. Es wurde tatsächlich mit diesem Profil gebaut; es sind keine absoluten Workspace-Pfade nötig.

Neue Firmware: `build/esp8266.esp8266.nodemcu-1.7.0/motor_on_a_roller_blind-ws.ino.bin`. Der Build verwendet QIO/40 MHz Flash, 80 MHz CPU, 4 MB Flash und 1 MB SPIFFS. Die alten vorhandenen Build-Verzeichnisse wurden nicht überschrieben; im neuen Lieferarchiv liegt nur die neue Firmware.

### Kompatibilität des gespeicherten Dateisystems

Der alte Linkerbereich war `0x40500000` bis `0x405fb000`, im aktuellen 1-MB-FS-Profil ist es `0x40500000` bis `0x405fa000`. Anfang, Seitengröße (256 Bytes) und Blockgröße (8192 Bytes) sind identisch. Beide Bereiche ergeben **125 vollständige SPIFFS-Blöcke**; der alte zusätzliche 4-KB-Rest war kein zusätzlicher vollständiger Block.

Zusätzlich wurde ein Konfigurationsabbild mit dem SPIFFS-Werkzeug des alten 2.4.2-Cores erzeugt, auf den aktuellen Bereich begrenzt und mit dem aktuellen Core-Werkzeug ausgelesen: `config.json` war byteidentisch. Das ist ein Offline-Abbildnachweis, kein Hardwaretest. Die Firmware deaktiviert das automatische Formatieren bei Mountfehlern, sodass eine falsche Layoutauswahl bestehende Daten nicht stillschweigend löscht.

ArduinoOTA bleibt zusätzlich zum Web-Upload erhalten. Es wurde **kein Upload auf ein echtes Gerät oder physischer Motorversuch** ausgeführt. Nach einem Core-Sprung ist ein erster regulärer USB-Upload die einfachste Vorgehensweise; ein OTA-Upgrade über die vorhandene Altfirmware wurde nicht getestet.

### HTTP-Schnittstellen

- `GET /api/status`: laufende Position und Zustand.
- `GET /api/config`: gespeicherte Einstellungen; nur Passwortstatus, kein Passwort.
- `POST /api/command`: `command`, z. B. `(0)` oder `50`.
- `POST /api/name`: `name` (normalisierter Hostname); liefert gespeicherte Konfiguration mit `restartPending` und `ipAddress`.
- `POST /api/limits`: `startPosition`, `maxPosition`.
- `POST /api/direction`: `clockwise=true|false`.
- `POST /api/mqtt`: `mqtt_server`, `mqtt_port`, `mqtt_uid`, optional `mqtt_pwd` und `clear_password=true`.
- `POST /api/firmware`: `multipart/form-data` mit genau einer Datei im Feld `firmware`; Header `X-Firmware-Size` (Dateibytes) und `X-Firmware-Token` (aus `GET /api/config`). Nur für Firmware, nicht für Dateisysteme.

Einstellungs-/Befehls-POSTs als `application/x-www-form-urlencoded`, Firmware als `multipart/form-data`. Erfolgsantwort: JSON; ungültige Werte: HTTP 400; Speicherfehler: HTTP 500.

## Prüfungen

Zusätzlich für Gerätenamen/Englisch:

- `tests/device_name_test.cpp`: tatsächlich verwendete Namensvalidierung mit gültigen Namen, Grenzlängen, Leerzeichen, Umlauten, Punkten, Bindestrichpositionen und unsicheren Zeichen.
- `tests/name_browser_test.cjs`: tatsächliche englische Seite gegen simulierte API; Feld/Links, Eingabeschutz, Entwürfe, Speicherfehler, unveränderter Name, Kleinschreibung, Neustartsperre und Wiederverbindung/Reload. Bestätigt auch, dass Grenzen und MQTT-Daten danach sichtbar bleiben.
- Der Speichertest lädt den Namen zusammen mit beiden Grenzen in eine frische Helferinstanz. Das ist ein Hosttest, keine reale ESP-SPIFFS-/DHCP-/mDNS-/OTA-Prüfung.


Zusätzlich für den Web-Upload:

- `tests/firmware_upload_test.cpp`: tatsächlicher Streaming-Controller mit Fake-Flash-Writer; fragmentierte Header und Blöcke, exakte Bytefolge, Größenlimits, falsche Header/ESP32, unvollständige/zu lange Anfragen, Abbruch vor/nach `FILE_END`, mehrere Dateien, Wiederholung nach Fehlern sowie Schreib-/Abschlussfehler. Optional wird die tatsächlich erzeugte BIN-Datei vollständig durch den Controller gestreamt und byteweise verglichen.
- `tests/firmware_browser_test.cjs`: echte Oberfläche und XMLHttpRequest gegen simulierte Geräte-API; Freigabe, Dateifilter, Multipart-Datei/Upload-Header, Fortschrittsanzeige, Bediensperre, Fehler, unbestätigte Netzwerkabbrüche und Neustart-Erkennung mit Seiten-Neuladen. Das Fortschrittsereignis ist im Interceptiontest ebenfalls simuliert.


- Reale Firmware-Kompilierung für den ESP8266 mit obigen Versionen.
- `tests/motion_test.cpp`: echte Positions-/Timerlogik, Befehlsvalidierung, Null-/Umkehrgrenzen, nichtnullbasierte Koordinaten, große Werte, Stoppen sowie 5-Sekunden-Timer einschließlich `millis()`-Überlauf.
- `tests/helper_test.cpp`: sichere Speicherinitialisierung (bestehendes Volume, nichtleerer Flash, Lesefehler und frischer gelöschter Bereich) sowie echter Speicher-/MQTT-Helfer mit ArduinoJson 7.4.3 und simulierten Peripheriegeräten; JSON-Lebensdauer, Lesen/Schreiben, Sonderzeichen, Schreibfehler, Backup-Wiederherstellung und begrenzte Wiederverbindungsversuche.
- `tests/browser_test.cjs`: echte Seite in headless Chrome mit **simulierter Geräte-API**; gespeicherte/live Werte, Kalibrierung, Richtungswechsel, MQTT/Passwortbehandlung, Speicherfehler, Wiederverbindung, Erhalt von Formularentwürfen, mobiles Layout und HTML-sichere Darstellung.

Beispiel für Hosttests (Ausgaben in temporäres Verzeichnis):

```sh
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=undefined,address tests/motion_test.cpp -o /tmp/blind-motion-test
/tmp/blind-motion-test
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=undefined,address -I tests/host -I /pfad/zu/ArduinoJson/src tests/helper_test.cpp NidayandHelper.cpp -o /tmp/blind-helper-test
/tmp/blind-helper-test
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=undefined,address tests/firmware_upload_test.cpp -o /tmp/blind-upload-test
/tmp/blind-upload-test build/esp8266.esp8266.nodemcu-1.7.0/motor_on_a_roller_blind-ws.ino.bin
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=undefined,address tests/device_name_test.cpp -o /tmp/blind-name-test
/tmp/blind-name-test
PLAYWRIGHT_MODULE=/pfad/zu/playwright-core node tests/name_browser_test.cjs
PLAYWRIGHT_MODULE=/pfad/zu/playwright-core node tests/firmware_browser_test.cjs
PLAYWRIGHT_MODULE=/pfad/zu/playwright-core node tests/browser_test.cjs
```

Nicht am Gerät geprüft: tatsächliche Drehrichtung/Mechanik, WLAN/OTA und MQTT über eine reale Funk-/Brokerverbindung. Der Browsertest allein ist kein Nachweis für physische Bewegung oder dauerhafte Flash-Speicherung; dafür dienen ergänzend die getrennten Logik-/Helfertests, ein Hardwareversuch steht aber noch aus.

## Originalprojekt

[Quellprojekt](https://github.com/nidayand/motor-on-roller-blind-ws) · [3D-Druckteile und Bauanleitung](https://www.thingiverse.com/thing:2392856) · [Stepper-Bibliothek](https://github.com/thomasfredericks/Stepper_28BYJ_48). Lizenz siehe `LICENSE`.
