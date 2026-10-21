#ifndef INDEX_HTML_H
#define INDEX_HTML_H
#include <pgmspace.h>
// Served directly from flash; no CDN, jQuery or internet access required.
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Rollladensteuerung</title>
<style>
:root{font:16px system-ui,sans-serif;color:#e7eef8;background:#101824;color-scheme:dark}*{box-sizing:border-box}body{max-width:850px;margin:auto;padding:18px}h1{font-size:1.55rem}h2{font-size:1.15rem;margin-top:0}section{background:#1b293b;border:1px solid #34455e;border-radius:12px;padding:20px;margin:16px 0}label{display:block;margin:12px 0 6px}input,select,button{font:inherit;padding:10px;border:1px solid #64758e;border-radius:7px}input:not([type=checkbox]),select{width:100%;background:#101824;color:inherit}input[type=range]{padding:0}button{background:#286aac;color:white;cursor:pointer;margin:6px 4px 6px 0}button.stop{background:#a63040}button.secondary{background:#34455e}button:disabled{opacity:.45;cursor:default}.row{display:flex;gap:12px;flex-wrap:wrap}.row>div{flex:1;min-width:180px}progress{width:100%;height:24px}dl{display:grid;grid-template-columns:minmax(130px,1fr) 2fr;gap:9px;margin-bottom:0}dt{color:#acbdd3}dd{margin:0;overflow-wrap:anywhere}small,.hint{color:#acbdd3;line-height:1.5}#notice{position:sticky;top:8px;background:#203a53;border:1px solid #718ba9;padding:12px;border-radius:8px;z-index:1;overflow-wrap:anywhere}#notice[hidden]{display:none}#notice.error{background:#642737}#connection{font-size:.9rem;color:#a8d3fa}.live{font-size:1.7rem;font-weight:bold}code{overflow-wrap:anywhere}a{color:#9bd1ff}footer{color:#acbdd3;margin:20px 0}
</style>
</head>
<body>
<header><h1 id="device-name">Rollladensteuerung</h1><div id="connection" role="status">Verbinde …</div></header>
<div id="notice" role="status" aria-live="polite" hidden></div>
<section><h2>Position &amp; Steuerung</h2>
<div class="live"><span id="position">–</span> %</div>
<p id="motion-state">Status wird geladen …</p><progress id="progress" max="100" value="0" aria-label="Aktuelle Position"></progress>
<p>Aktuell: <strong id="current-steps">–</strong> Schritte · Ziel: <span id="target-steps">–</span></p>
<label for="position-slider">Zielposition: <output id="slider-value">0</output> % (0 = offen, 100 = geschlossen)</label>
<input id="position-slider" type="range" min="0" max="100" value="0" data-control disabled>
<div><button type="button" data-command="0" data-control disabled>Öffnen</button><button type="button" data-command="(0)" class="stop" data-control disabled>Stopp</button><button type="button" data-command="100" data-control disabled>Schließen</button></div>
<p id="mqtt-state" class="hint">MQTT: –</p>
</section>
<section><h2>Gespeicherte Werte</h2><p id="persist-state" class="hint">Lade Konfiguration …</p>
<dl id="saved-values"></dl><p class="hint">Die aktuelle Position wird beim Stoppen gespeichert, nicht während jedes Motorschritts. Das schont den Flash.</p></section>
<section><h2>Anfang und Ende anpassen</h2>
<p class="hint">Manuell zur gewünschten Position fahren, stoppen und als Anfang oder Ende übernehmen. Diese Tasten fahren ohne Endbegrenzung – den Antrieb dabei beobachten. Alternativ beide Schrittwerte direkt bearbeiten. Änderungen stoppen den Antrieb.</p>
<div><button type="button" data-command="(-1)" data-control disabled>Manuell auf ↑</button><button type="button" data-command="(0)" class="stop" data-control disabled>Stopp</button><button type="button" data-command="(1)" data-control disabled>Manuell ab ↓</button></div>
<div><button type="button" data-command="(start)" data-control disabled>Aktuell als Anfang (offen)</button><button type="button" data-command="(max)" data-control disabled>Aktuell als Ende (geschlossen)</button></div>
<form id="limits-form"><div class="row"><div><label for="start-position">Anfang in Schritten</label><input id="start-position" name="startPosition" type="number" min="-1000000000" max="1000000000" step="1" required data-control disabled></div><div><label for="end-position">Ende in Schritten</label><input id="end-position" name="maxPosition" type="number" min="-1000000000" max="1000000000" step="1" required data-control disabled></div></div>
<button data-control disabled>Grenzen speichern</button><p class="hint">Anfang muss kleiner als Ende sein. Das Speichern verändert die Schrittkoordinate nicht und startet keine Fahrt.</p></form></section>
<section><h2>Laufrichtung</h2><form id="direction-form">
<label for="clockwise">Drehrichtung zum Schließen</label><select id="clockwise" name="clockwise" data-control disabled><option value="false">Gegen den Uhrzeigersinn (CCW)</option><option value="true">Im Uhrzeigersinn (CW)</option></select>
<button data-control disabled>Laufrichtung speichern</button><p class="hint">Der Motor wird vor der Umstellung gestoppt. Danach Richtung und Endpositionen kontrollieren; es erfolgt keine automatische Fahrt.</p></form></section>
<section><h2>MQTT-Einstellungen</h2><form id="mqtt-form" autocomplete="off">
<label for="mqtt-server">Broker (Hostname oder IPv4; leer = MQTT aus)</label><input id="mqtt-server" name="mqtt_server" maxlength="39" placeholder="192.168.1.10" data-control disabled>
<div class="row"><div><label for="mqtt-port">Port</label><input id="mqtt-port" name="mqtt_port" type="number" min="1" max="65535" step="1" required value="1883" data-control disabled></div><div><label for="mqtt-user">Benutzername</label><input id="mqtt-user" name="mqtt_uid" maxlength="39" autocomplete="off" data-control disabled></div></div>
<label for="mqtt-password">Neues Passwort</label><input id="mqtt-password" name="mqtt_pwd" type="password" maxlength="39" autocomplete="new-password" placeholder="Leer lassen = gespeichertes Passwort behalten" data-control disabled>
<p id="password-state" class="hint">Passwortstatus: –</p><label><input id="clear-password" type="checkbox" name="clear_password" value="true" data-control disabled> Gespeichertes Passwort löschen</label>
<button data-control disabled>MQTT speichern &amp; neu verbinden</button>
<p class="hint">Kein Neustart nötig. Während einer Fahrt wird bei bestehender Verbindung alle 5 Sekunden ein Status gesendet, zusätzlich beim Start und Stopp. Neue Verbindungsversuche erfolgen im Stillstand.</p>
</form><p>Befehle: <code id="input-topic">–</code><br>Status: <code id="output-topic">–</code></p></section>
<section><h2>Firmware-Update</h2>
<p id="firmware-info" class="hint">Update-Informationen werden geladen …</p>
<form id="firmware-form">
<label for="firmware-file">Neue ESP8266-Firmware (.bin)</label>
<input id="firmware-file" type="file" accept=".bin,application/octet-stream" required data-control disabled>
<label><input id="firmware-confirm" type="checkbox" required data-control disabled> Update starten: Der Motor wird gestoppt und das Gerät anschließend neu gestartet.</label>
<button id="firmware-submit" data-control disabled>Firmware hochladen &amp; installieren</button>
</form>
<progress id="firmware-progress" max="100" value="0" aria-label="Firmware-Upload" hidden></progress>
<p id="firmware-status" role="status" aria-live="polite"></p>
<p class="hint">Nur die zum Gerät passende Firmware-Datei wählen, kein ZIP-Archiv oder Dateisystem-Abbild. Stromversorgung während des Updates nicht unterbrechen. Die gespeicherten Positionen und MQTT-Einstellungen bleiben bei diesem Firmware-Upload erhalten.</p>
</section>
<footer><span id="version"></span> · ESP8266 · <a href="https://github.com/nidayand/motor-on-roller-blind-ws">Originalprojekt (MIT)</a></footer>
<script>
'use strict';
const $ = id => document.getElementById(id);
let socket, reconnectTimer, pollBusy = false, loaded = false, online = false, revision = -1, refreshPromise;
let firmwareBusy = false, waitForFirmwareBoot = false, firmwareMaxSize = 0, firmwareToken = '', firmwareBootToken = '', rebootCheckStarted = 0;
const dirty = {limits: false, direction: false, mqtt: false};
function notice(text, error = false) {
  $('notice').textContent = text; $('notice').classList.toggle('error', error); $('notice').hidden = !text;
}
function setOnline(value) {
  online = value;
  document.querySelectorAll('[data-control]').forEach(el => { el.disabled = !value || !loaded || firmwareBusy; });
  $('connection').textContent = firmwareBusy ? (waitForFirmwareBoot ? 'Firmware angenommen – warte auf Neustart …' : 'Firmware wird übertragen …') : value ? 'Mit Gerät verbunden' : 'Verbindung unterbrochen – verbinde erneut …';
  updateFirmwareButton();
}
async function api(path, values) {
  const options = {cache: 'no-store'};
  if (values !== undefined) {
    options.method = 'POST';
    options.headers = {'Content-Type': 'application/x-www-form-urlencoded'};
    options.body = new URLSearchParams(values);
  }
  let response;
  try { response = await fetch(path, options); }
  catch (error) { setOnline(false); throw new Error('Gerät nicht erreichbar. Befehl wurde nicht bestätigt.'); }
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || 'Anfrage fehlgeschlagen');
  return data;
}
function renderConfig(c) {
  revision = c.configRevision;
  loaded = true;
  $('device-name').textContent = c.name;
  document.title = c.name;
  $('version').textContent = 'Firmware ' + c.version;
  firmwareMaxSize = Number(c.firmwareMaxSize) || 0;
  firmwareToken = c.firmwareToken || '';
  $('firmware-info').textContent = 'Installiert: ' + c.version + (firmwareMaxSize ? ' · maximal ' + Math.floor(firmwareMaxSize / 1024) + ' KiB pro Firmware-Datei.' : ' · Firmware-Upload hier noch nicht verfügbar.');
  $('persist-state').textContent = c.persisted ? 'Zuletzt erfolgreich im Flash gespeichert:' : 'Standardwerte – noch nicht im Flash gespeichert.';
  const entries = [
    ['Anfang (offen)', c.startPosition + ' Schritte'], ['Ende (geschlossen)', c.maxPosition + ' Schritte'],
    ['Letzte gespeicherte Position', c.currentPosition + ' Schritte'],
    ['Schließrichtung', c.clockwise ? 'Im Uhrzeigersinn (CW)' : 'Gegen den Uhrzeigersinn (CCW)'],
    ['MQTT-Broker', c.mqtt_server || 'Deaktiviert'], ['MQTT-Port', c.mqtt_port], ['MQTT-Benutzer', c.mqtt_uid || 'Ohne Anmeldung'],
    ['MQTT-Passwort', c.mqtt_password_set ? 'Gesetzt (wird nicht angezeigt)' : 'Nicht gesetzt']
  ];
  $('saved-values').replaceChildren();
  entries.forEach(([key, value]) => {
    const dt = document.createElement('dt'), dd = document.createElement('dd');
    dt.textContent = key; dd.textContent = value; $('saved-values').append(dt, dd);
  });
  if (!dirty.limits) { $('start-position').value = c.startPosition; $('end-position').value = c.maxPosition; }
  if (!dirty.direction) $('clockwise').value = String(c.clockwise);
  if (!dirty.mqtt) {
    $('mqtt-server').value = c.mqtt_server; $('mqtt-port').value = c.mqtt_port; $('mqtt-user').value = c.mqtt_uid;
    $('mqtt-password').value = ''; $('clear-password').checked = false;
  }
  $('password-state').textContent = c.mqtt_password_set ? 'Ein Passwort ist gespeichert. Leere Eingabe behält es bei.' : 'Kein Passwort gespeichert.';
  $('input-topic').textContent = c.inputTopic; $('output-topic').textContent = c.outputTopic;
  setOnline(online);
}
async function refreshConfig() {
  if (!refreshPromise) refreshPromise = api('/api/config').then(renderConfig).finally(() => { refreshPromise = null; });
  return refreshPromise;
}
function renderStatus(s) {
  if (s.error) { notice(s.error, true); return; }
  setOnline(true);
  $('position').textContent = s.position; $('progress').value = s.position;
  $('current-steps').textContent = s.currentPosition; $('target-steps').textContent = s.targetPosition;
  $('motion-state').textContent = s.moving ? (s.direction === 'down' ? 'Fährt ab / schließt' : 'Fährt auf / öffnet') : 'Antrieb steht';
  $('mqtt-state').textContent = s.mqttConnected ? 'MQTT: verbunden' : 'MQTT: nicht verbunden / deaktiviert';
  if (document.activeElement !== $('position-slider')) {
    $('position-slider').value = s.set; $('slider-value').textContent = s.set;
  }
  if (s.saveError && !firmwareBusy) notice(s.saveError, true);
  if (!firmwareBusy && s.configRevision !== revision) refreshConfig().catch(e => notice(e.message, true));
}
async function command(value) {
  if (firmwareBusy) return;
  try {
    renderStatus(await api('/api/command', {command: value}));
    if (['(start)', '(max)', '(0)'].includes(value)) await refreshConfig();
    notice(value === '(0)' ? 'Gestoppt und Position gespeichert.' : ['(start)', '(max)'].includes(value) ? 'Position gespeichert.' : 'Fahrbefehl bestätigt.');
  } catch (e) { notice(e.message, true); }
}
async function saveForm(event, section, path) {
  event.preventDefault();
  if (firmwareBusy) return;
  const values = new URLSearchParams(new FormData(event.target));
  if (section === 'limits' && Number(values.get('startPosition')) >= Number(values.get('maxPosition'))) {
    notice('Anfang muss kleiner als Ende sein.', true); return;
  }
  if (section === 'mqtt' && $('clear-password').checked && $('mqtt-password').value) {
    notice('Entweder ein neues Passwort eingeben oder das gespeicherte löschen.', true); return;
  }
  try {
    const result = await api(path, values);
    dirty[section] = false; renderConfig(result);
    notice('Gespeichert. Der Antrieb steht.');
    renderStatus(await api('/api/status'));
  } catch (e) { notice(e.message, true); }
}
for (const section of ['limits', 'direction', 'mqtt']) {
  const form = $(section + '-form');
  form.addEventListener('input', () => { dirty[section] = true; });
  form.addEventListener('change', () => { dirty[section] = true; });
  form.addEventListener('submit', event => { saveForm(event, section, '/api/' + (section === 'limits' ? 'limits' : section === 'direction' ? 'direction' : 'mqtt')); });
}
document.querySelectorAll('[data-command]').forEach(button => button.addEventListener('click', () => command(button.dataset.command)));
$('position-slider').addEventListener('input', event => { $('slider-value').textContent = event.target.value; });
$('position-slider').addEventListener('change', event => command(event.target.value));
function connect() {
  clearTimeout(reconnectTimer);
  if (firmwareBusy) return;
  try { socket = new WebSocket('ws://' + location.hostname + ':81/'); }
  catch (e) { reconnectTimer = setTimeout(connect, 3000); return; }
  socket.onopen = () => { setOnline(true); if (!firmwareBusy) refreshConfig().catch(e => notice(e.message, true)); };
  socket.onmessage = event => { try { renderStatus(JSON.parse(event.data)); } catch (e) { notice('Status konnte nicht gelesen werden.', true); } };
  socket.onclose = () => { setOnline(false); reconnectTimer = setTimeout(connect, 3000); };
  socket.onerror = () => socket.close();
}
// HTTP fallback also detects network loss if WebSocket events fail to arrive.
async function poll() {
  if (pollBusy || (firmwareBusy && !waitForFirmwareBoot)) return;
  pollBusy = true;
  if (waitForFirmwareBoot) {
    try {
      const c = await api('/api/config');
      if (c.firmwareToken && c.firmwareToken !== firmwareBootToken) { location.reload(); return; }
    } catch (e) { /* The device is expected to be offline briefly during reboot. */ }
    finally { pollBusy = false; }
    if (Date.now() - rebootCheckStarted > 90000) $('firmware-status').textContent = 'Upload angenommen, aber Neustart noch nicht bestätigt. Geräteverbindung prüfen und diese Seite neu laden.';
    return;
  }
  try { renderStatus(await api('/api/status')); }
  catch (e) { setOnline(false); }
  finally { pollBusy = false; }
}
function updateFirmwareButton() {
  const file = $('firmware-file').files[0];
  $('firmware-submit').disabled = !online || !loaded || firmwareBusy || !firmwareToken || firmwareMaxSize < 32 || !file || !$('firmware-confirm').checked;
}
function firmwareFailed(message) {
  firmwareBusy = false; waitForFirmwareBoot = false;
  $('firmware-status').textContent = message;
  notice(message, true); setOnline(online);
}
$('firmware-file').addEventListener('change', updateFirmwareButton);
$('firmware-confirm').addEventListener('change', updateFirmwareButton);
$('firmware-form').addEventListener('submit', async event => {
  event.preventDefault();
  if (firmwareBusy || !online || !loaded) return;
  const file = $('firmware-file').files[0];
  if (!file || !/\.bin$/i.test(file.name)) { notice('Bitte eine unkomprimierte Firmware-Datei (.bin) auswählen.', true); return; }
  if (file.size < 32 || file.size > firmwareMaxSize) { notice('Datei leer, zu klein oder größer als der freie Update-Speicher.', true); return; }
  if (!$('firmware-confirm').checked || !firmwareToken) return;
  firmwareBusy = true; setOnline(online);
  try {
    const header = new Uint8Array(await file.slice(0, 8).arrayBuffer());
    const entry = new DataView(header.buffer).getUint32(4, true);
    if (header[0] !== 0xE9 || header[1] < 1 || header[1] > 16 || entry < 0x40100000 || entry >= 0x40110000) throw new Error('Keine passende ESP8266-Firmware. Keine ZIP-, Dateisystem- oder ESP32-Datei verwenden.');
  } catch (e) { firmwareFailed(e.message || 'Datei konnte nicht gelesen werden.'); return; }
  firmwareBootToken = firmwareToken;
  const data = new FormData(); data.append('firmware', file, file.name);
  const request = new XMLHttpRequest();
  request.open('POST', '/api/firmware');
  request.setRequestHeader('X-Firmware-Size', String(file.size));
  request.setRequestHeader('X-Firmware-Token', firmwareToken);
  request.timeout = 180000;
  $('firmware-progress').hidden = false; $('firmware-progress').value = 0;
  $('firmware-status').textContent = 'Firmware wird übertragen …'; notice('Update läuft. Stromversorgung nicht unterbrechen.');
  request.upload.onprogress = event => {
    if (!event.lengthComputable) return;
    const percent = Math.min(100, Math.round(event.loaded * 100 / event.total));
    $('firmware-progress').value = percent;
    $('firmware-status').textContent = percent < 100 ? 'Übertragung: ' + percent + ' %' : 'Übertragung: 100 % – Firmware wird geprüft …';
  };
  request.onload = () => {
    let result = {};
    try { result = JSON.parse(request.responseText); } catch (e) { /* Unconfirmed response. */ }
    if (request.status !== 200 || result.ok !== true || result.rebooting !== true) {
      firmwareFailed(result.error || 'Update nicht bestätigt. Geräteversion prüfen, bevor erneut hochgeladen wird.'); return;
    }
    $('firmware-progress').value = 100;
    $('firmware-status').textContent = 'Firmware angenommen. Gerät startet neu; die Seite wird nach Wiederverbindung neu geladen.';
    notice('Firmware angenommen – warte auf Neustart …');
    waitForFirmwareBoot = true; rebootCheckStarted = Date.now(); setOnline(false);
  };
  request.onerror = request.ontimeout = request.onabort = () => {
    setOnline(false);
    firmwareFailed('Keine Update-Bestätigung erhalten. Verbindung und Geräteversion prüfen, bevor erneut hochgeladen wird.');
  };
  request.send(data);
});
connect(); poll(); setInterval(poll, 2000);
</script>
</body></html>)HTML";
#endif
