#line 1 "G:\\Meine Ablage\\Sicherungen\\Raspi-Sicherungen\\Node MCU ESP8266\\motor-on-roller-blind-ws-master\\v2\\motor_on_a_roller_blind-ws\\index_html.h"
#ifndef INDEX_HTML_H
#define INDEX_HTML_H
#include <pgmspace.h>
// Served directly from flash; no CDN, jQuery or internet access required.
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Roller blind controller</title>
<style>
:root{font:16px system-ui,sans-serif;color:#e7eef8;background:#101824;color-scheme:dark}*{box-sizing:border-box}body{max-width:850px;margin:auto;padding:18px}h1{font-size:1.55rem}h2{font-size:1.15rem;margin-top:0}section{background:#1b293b;border:1px solid #34455e;border-radius:12px;padding:20px;margin:16px 0}label{display:block;margin:12px 0 6px}input,select,button{font:inherit;padding:10px;border:1px solid #64758e;border-radius:7px}input:not([type=checkbox]),select{width:100%;background:#101824;color:inherit}input[type=range]{padding:0}button{background:#286aac;color:white;cursor:pointer;margin:6px 4px 6px 0}button.stop{background:#a63040}button.secondary{background:#34455e}button:disabled{opacity:.45;cursor:default}.row{display:flex;gap:12px;flex-wrap:wrap}.row>div{flex:1;min-width:180px}progress{width:100%;height:24px}dl{display:grid;grid-template-columns:minmax(130px,1fr) 2fr;gap:9px;margin-bottom:0}dt{color:#acbdd3}dd{margin:0;overflow-wrap:anywhere}small,.hint{color:#acbdd3;line-height:1.5}#notice{position:sticky;top:8px;background:#203a53;border:1px solid #718ba9;padding:12px;border-radius:8px;z-index:1;overflow-wrap:anywhere}#notice[hidden]{display:none}#notice.error{background:#642737}#connection{font-size:.9rem;color:#a8d3fa}.live{font-size:1.7rem;font-weight:bold}code{overflow-wrap:anywhere}a{color:#9bd1ff}footer{color:#acbdd3;margin:20px 0}
</style>
</head>
<body>
<header><h1 id="device-name">Roller blind controller</h1><div id="connection" role="status">Connecting …</div></header>
<div id="notice" role="status" aria-live="polite" hidden></div>
<section><h2>Position &amp; control</h2>
<div class="live"><span id="position">–</span> %</div>
<p id="motion-state">Loading status …</p><progress id="progress" max="100" value="0" aria-label="Current position"></progress>
<p>Current: <strong id="current-steps">–</strong> steps · Target: <span id="target-steps">–</span></p>
<label for="position-slider">Target position: <output id="slider-value">0</output> % (0 = open, 100 = closed)</label>
<input id="position-slider" type="range" min="0" max="100" value="0" data-control disabled>
<div><button type="button" data-command="0" data-control disabled>Open</button><button type="button" data-command="(0)" class="stop" data-control disabled>Stop</button><button type="button" data-command="100" data-control disabled>Close</button></div>
<p id="mqtt-state" class="hint">MQTT: –</p>
</section>
<section><h2>Saved settings</h2><p id="persist-state" class="hint">Loading configuration …</p>
<dl id="saved-values"></dl><p class="hint">The current position is saved when the motor stops, not on every step. This reduces flash wear.</p></section>
<section><h2>Device name</h2>
<form id="name-form">
<label for="device-name-input">Device name / hostname</label>
<input id="device-name-input" name="name" maxlength="31" pattern="[A-Za-z0-9]([A-Za-z0-9\-]{0,29}[A-Za-z0-9])?" placeholder="living-room-blind" autocomplete="off" spellcheck="false" required data-control disabled>
<button data-control disabled>Save name &amp; restart</button>
<p class="hint">Use 1-31 letters, digits or hyphens, without spaces or a .local suffix. No leading or trailing hyphen. Names are saved in lowercase. Saving a changed name stops the motor and restarts the device.</p>
</form>
<p id="name-status" role="status" aria-live="polite"></p>
<p>Device address: <a id="device-local-link" hidden></a><span id="device-no-local">not available</span><br>IP address: <a id="device-ip-link" hidden></a><span id="device-no-ip">not available</span></p>
<p class="hint">If you opened this page using the old device name, reconnect using the new link or the IP address after the restart. MQTT topics stay unchanged.</p>
</section>
<section><h2>Adjust start and end positions</h2>
<p class="hint">Move manually to the desired position, stop and save it as the start or end. Manual movement has no end limits - watch the motor. Alternatively, enter both step values directly. Saving changes stops the motor.</p>
<div><button type="button" data-command="(-1)" data-control disabled>Manual up ↑</button><button type="button" data-command="(0)" class="stop" data-control disabled>Stop</button><button type="button" data-command="(1)" data-control disabled>Manual down ↓</button></div>
<div><button type="button" data-command="(start)" data-control disabled>Set current as start (open)</button><button type="button" data-command="(max)" data-control disabled>Set current as end (closed)</button></div>
<form id="limits-form"><div class="row"><div><label for="start-position">Start position in steps</label><input id="start-position" name="startPosition" type="number" min="-1000000000" max="1000000000" step="1" required data-control disabled></div><div><label for="end-position">End position in steps</label><input id="end-position" name="maxPosition" type="number" min="-1000000000" max="1000000000" step="1" required data-control disabled></div></div>
<button data-control disabled>Save limits</button><p class="hint">The start must be less than the end. Saving does not reset the step coordinate or start movement.</p></form></section>
<section><h2>Rotation direction</h2><form id="direction-form">
<label for="clockwise">Rotation direction when closing</label><select id="clockwise" name="clockwise" data-control disabled><option value="false">Counterclockwise (CCW)</option><option value="true">Clockwise (CW)</option></select>
<button data-control disabled>Save rotation direction</button><p class="hint">The motor stops before changing direction. Check the direction and end positions afterwards; no movement starts automatically.</p></form></section>
<section><h2>MQTT settings</h2><form id="mqtt-form" autocomplete="off">
<label for="mqtt-server">Broker (hostname or IPv4; leave blank to disable MQTT)</label><input id="mqtt-server" name="mqtt_server" maxlength="39" placeholder="192.168.1.10" data-control disabled>
<div class="row"><div><label for="mqtt-port">Port</label><input id="mqtt-port" name="mqtt_port" type="number" min="1" max="65535" step="1" required value="1883" data-control disabled></div><div><label for="mqtt-user">Username</label><input id="mqtt-user" name="mqtt_uid" maxlength="39" autocomplete="off" data-control disabled></div></div>
<label for="mqtt-password">New password</label><input id="mqtt-password" name="mqtt_pwd" type="password" maxlength="39" autocomplete="new-password" placeholder="Leave blank to keep the saved password" data-control disabled>
<p id="password-state" class="hint">Password status: –</p><label><input id="clear-password" type="checkbox" name="clear_password" value="true" data-control disabled> Delete saved password</label>
<button data-control disabled>Save MQTT &amp; reconnect</button>
<p class="hint">No restart required. While moving, status is sent every 5 seconds if connected, and on start/stop. New connection attempts are made only while stopped.</p>
</form><p>Commands: <code id="input-topic">–</code><br>Status: <code id="output-topic">–</code></p></section>
<section><h2>Firmware update</h2>
<p id="firmware-info" class="hint">Loading update information …</p>
<form id="firmware-form">
<label for="firmware-file">New ESP8266 firmware (.bin)</label>
<input id="firmware-file" type="file" accept=".bin,application/octet-stream" required data-control disabled>
<label><input id="firmware-confirm" type="checkbox" required data-control disabled> Start update: the motor will stop and the device will restart afterwards.</label>
<button id="firmware-submit" data-control disabled>Upload &amp; install firmware</button>
</form>
<progress id="firmware-progress" max="100" value="0" aria-label="Firmware upload" hidden></progress>
<p id="firmware-status" role="status" aria-live="polite"></p>
<p class="hint">Select firmware that matches this device, not a ZIP archive or filesystem image. Do not disconnect power during the update. Saved positions and MQTT settings are retained by this firmware-only upload.</p>
</section>
<footer><span id="version"></span> · ESP8266 · <a href="https://github.com/nidayand/motor-on-roller-blind-ws">Original project (MIT)</a></footer>
<script>
'use strict';
const $ = id => document.getElementById(id);
let socket, reconnectTimer, pollBusy = false, loaded = false, online = false, revision = -1, refreshPromise;
let deviceBusy = false, waitForDeviceBoot = false, firmwareMaxSize = 0, firmwareToken = '', expectedBootToken = '', rebootCheckStarted = 0;
let busyReason = 'firmware';
const dirty = {limits: false, direction: false, mqtt: false, name: false};
function notice(text, error = false) {
  $('notice').textContent = text; $('notice').classList.toggle('error', error); $('notice').hidden = !text;
}
function setOnline(value) {
  online = value;
  document.querySelectorAll('[data-control]').forEach(el => { el.disabled = !value || !loaded || deviceBusy; });
  $('connection').textContent = deviceBusy ? (waitForDeviceBoot ? (busyReason === 'name' ? 'Name saved - waiting for restart …' : 'Firmware accepted - waiting for restart …') : busyReason === 'name' ? 'Saving device name …' : 'Uploading firmware …') : value ? 'Connected to device' : 'Disconnected - reconnecting …';
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
  catch (error) { setOnline(false); throw new Error('Device unreachable. Command was not confirmed.'); }
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || 'Request failed');
  return data;
}
function renderConfig(c) {
  revision = c.configRevision;
  loaded = true;
  $('device-name').textContent = c.name;
  document.title = c.name;
  if (!dirty.name) $('device-name-input').value = c.name;
  const validHost = /^[a-z0-9](?:[a-z0-9-]{0,29}[a-z0-9])?$/i.test(c.name);
  $('device-local-link').hidden = !validHost; $('device-no-local').hidden = validHost;
  if (validHost) { $('device-local-link').href = 'http://' + c.name + '.local/'; $('device-local-link').textContent = c.name + '.local'; }
  const validIp = /^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$/.test(c.ipAddress || '') && c.ipAddress.split('.').every(x => Number(x) <= 255);
  $('device-ip-link').hidden = !validIp; $('device-no-ip').hidden = validIp;
  if (validIp) { $('device-ip-link').href = 'http://' + c.ipAddress + '/'; $('device-ip-link').textContent = c.ipAddress; }
  $('version').textContent = 'Firmware ' + c.version;
  firmwareMaxSize = Number(c.firmwareMaxSize) || 0;
  firmwareToken = c.firmwareToken || '';
  $('firmware-info').textContent = 'Installed: ' + c.version + (firmwareMaxSize ? ' · maximum ' + Math.floor(firmwareMaxSize / 1024) + ' KiB per firmware file.' : ' · Firmware upload is not available on this device.');
  $('persist-state').textContent = c.persisted ? 'Last successfully saved to flash:' : 'Default settings - not yet saved to flash.';
  const entries = [
    ['Device name', c.name],
    ['Start (open)', c.startPosition + ' steps'], ['End (closed)', c.maxPosition + ' steps'],
    ['Last saved position', c.currentPosition + ' steps'],
    ['Closing direction', c.clockwise ? 'Clockwise (CW)' : 'Counterclockwise (CCW)'],
    ['MQTT broker', c.mqtt_server || 'Disabled'], ['MQTT port', c.mqtt_port], ['MQTT username', c.mqtt_uid || 'No authentication'],
    ['MQTT password', c.mqtt_password_set ? 'Set (not displayed)' : 'Not set']
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
  $('password-state').textContent = c.mqtt_password_set ? 'A password is saved. Leave the field blank to keep it.' : 'No password saved.';
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
  $('motion-state').textContent = s.moving ? (s.direction === 'down' ? 'Moving down / closing' : 'Moving up / opening') : 'Motor stopped';
  $('mqtt-state').textContent = s.mqttConnected ? 'MQTT: connected' : 'MQTT: disconnected / disabled';
  if (document.activeElement !== $('position-slider')) {
    $('position-slider').value = s.set; $('slider-value').textContent = s.set;
  }
  if (s.saveError && !deviceBusy) notice(s.saveError, true);
  if (!deviceBusy && s.configRevision !== revision) refreshConfig().catch(e => notice(e.message, true));
}
async function command(value) {
  if (deviceBusy) return;
  try {
    renderStatus(await api('/api/command', {command: value}));
    if (['(start)', '(max)', '(0)'].includes(value)) await refreshConfig();
    notice(value === '(0)' ? 'Stopped and position saved.' : ['(start)', '(max)'].includes(value) ? 'Position saved.' : 'Movement command confirmed.');
  } catch (e) { notice(e.message, true); }
}
async function saveForm(event, section, path) {
  event.preventDefault();
  if (deviceBusy) return;
  const values = new URLSearchParams(new FormData(event.target));
  if (section === 'limits' && Number(values.get('startPosition')) >= Number(values.get('maxPosition'))) {
    notice('Start must be less than end.', true); return;
  }
  if (section === 'mqtt' && $('clear-password').checked && $('mqtt-password').value) {
    notice('Either enter a new password or delete the saved one.', true); return;
  }
  try {
    const result = await api(path, values);
    dirty[section] = false; renderConfig(result);
    notice('Saved. The motor is stopped.');
    renderStatus(await api('/api/status'));
  } catch (e) { notice(e.message, true); }
}
for (const section of ['limits', 'direction', 'mqtt', 'name']) {
  const form = $(section + '-form');
  form.addEventListener('input', () => { dirty[section] = true; });
  form.addEventListener('change', () => { dirty[section] = true; });
  form.addEventListener('submit', event => { if (section === 'name') { saveDeviceName(event); return; } saveForm(event, section, '/api/' + (section === 'limits' ? 'limits' : section === 'direction' ? 'direction' : 'mqtt')); });
}
document.querySelectorAll('[data-command]').forEach(button => button.addEventListener('click', () => command(button.dataset.command)));
$('position-slider').addEventListener('input', event => { $('slider-value').textContent = event.target.value; });
$('position-slider').addEventListener('change', event => command(event.target.value));
function connect() {
  clearTimeout(reconnectTimer);
  if (deviceBusy) return;
  try { socket = new WebSocket('ws://' + location.hostname + ':81/'); }
  catch (e) { reconnectTimer = setTimeout(connect, 3000); return; }
  socket.onopen = () => { setOnline(true); if (!deviceBusy) refreshConfig().catch(e => notice(e.message, true)); };
  socket.onmessage = event => { try { renderStatus(JSON.parse(event.data)); } catch (e) { notice('Could not read device status.', true); } };
  socket.onclose = () => { setOnline(false); reconnectTimer = setTimeout(connect, 3000); };
  socket.onerror = () => socket.close();
}
// HTTP fallback also detects network loss if WebSocket events fail to arrive.
async function poll() {
  if (pollBusy || (deviceBusy && !waitForDeviceBoot)) return;
  pollBusy = true;
  if (waitForDeviceBoot) {
    try {
      const c = await api('/api/config');
      if (c.firmwareToken && c.firmwareToken !== expectedBootToken) { location.reload(); return; }
    } catch (e) { /* The device is expected to be offline briefly during reboot. */ }
    finally { pollBusy = false; }
    if (Date.now() - rebootCheckStarted > 90000) {
      if (busyReason === 'name') $('name-status').textContent = 'Name saved, but reconnection is not confirmed. Use the new device address or IP link above.';
      else $('firmware-status').textContent = 'Upload accepted, but restart is not yet confirmed. Check the device connection and reload this page.';
    }
    return;
  }
  try { renderStatus(await api('/api/status')); }
  catch (e) { setOnline(false); }
  finally { pollBusy = false; }
}
async function saveDeviceName(event) {
  event.preventDefault();
  if (deviceBusy || !online || !loaded) return;
  const name = $('device-name-input').value.trim().toLowerCase();
  if (!/^[a-z0-9](?:[a-z0-9-]{0,29}[a-z0-9])?$/.test(name)) {
    notice('Use 1-31 letters, digits or hyphens, with no leading or trailing hyphen.', true); return;
  }
  expectedBootToken = firmwareToken;
  busyReason = 'name'; deviceBusy = true; setOnline(online);
  $('name-status').textContent = 'Saving device name …';
  try {
    const result = await api('/api/name', {name});
    dirty.name = false; renderConfig(result);
    if (result.restartPending) {
      $('name-status').textContent = 'Name saved. The device is restarting. Reconnect using the new address or IP link below.';
      notice('Device name saved - waiting for restart …');
      waitForDeviceBoot = true; rebootCheckStarted = Date.now(); setOnline(false);
    } else {
      deviceBusy = false; setOnline(true);
      $('name-status').textContent = 'The device name is unchanged. No restart required.';
      notice('Device name is unchanged.');
    }
  } catch (e) {
    deviceBusy = false; waitForDeviceBoot = false;
    $('name-status').textContent = e.message; notice(e.message, true); setOnline(online);
  }
}
function updateFirmwareButton() {
  const file = $('firmware-file').files[0];
  $('firmware-submit').disabled = !online || !loaded || deviceBusy || !firmwareToken || firmwareMaxSize < 32 || !file || !$('firmware-confirm').checked;
}
function firmwareFailed(message) {
  deviceBusy = false; waitForDeviceBoot = false;
  $('firmware-status').textContent = message;
  notice(message, true); setOnline(online);
}
$('firmware-file').addEventListener('change', updateFirmwareButton);
$('firmware-confirm').addEventListener('change', updateFirmwareButton);
$('firmware-form').addEventListener('submit', async event => {
  event.preventDefault();
  if (deviceBusy || !online || !loaded) return;
  const file = $('firmware-file').files[0];
  if (!file || !/\.bin$/i.test(file.name)) { notice('Select an uncompressed firmware file (.bin).', true); return; }
  if (file.size < 32 || file.size > firmwareMaxSize) { notice('File is empty, too small or larger than the available update space.', true); return; }
  if (!$('firmware-confirm').checked || !firmwareToken) return;
  busyReason = 'firmware'; deviceBusy = true; setOnline(online);
  try {
    const header = new Uint8Array(await file.slice(0, 8).arrayBuffer());
    const entry = new DataView(header.buffer).getUint32(4, true);
    if (header[0] !== 0xE9 || header[1] < 1 || header[1] > 16 || entry < 0x40100000 || entry >= 0x40110000) throw new Error('Not a valid ESP8266 firmware file. Do not use ZIP, filesystem or ESP32 images.');
  } catch (e) { firmwareFailed(e.message || 'Could not read the file.'); return; }
  expectedBootToken = firmwareToken;
  const data = new FormData(); data.append('firmware', file, file.name);
  const request = new XMLHttpRequest();
  request.open('POST', '/api/firmware');
  request.setRequestHeader('X-Firmware-Size', String(file.size));
  request.setRequestHeader('X-Firmware-Token', firmwareToken);
  request.timeout = 180000;
  $('firmware-progress').hidden = false; $('firmware-progress').value = 0;
  $('firmware-status').textContent = 'Uploading firmware …'; notice('Update in progress. Do not disconnect power.');
  request.upload.onprogress = event => {
    if (!event.lengthComputable) return;
    const percent = Math.min(100, Math.round(event.loaded * 100 / event.total));
    $('firmware-progress').value = percent;
    $('firmware-status').textContent = percent < 100 ? 'Upload: ' + percent + ' %' : 'Upload: 100 % - verifying firmware …';
  };
  request.onload = () => {
    let result = {};
    try { result = JSON.parse(request.responseText); } catch (e) { /* Unconfirmed response. */ }
    if (request.status !== 200 || result.ok !== true || result.rebooting !== true) {
      firmwareFailed(result.error || 'Update not confirmed. Check the device firmware version before uploading again.'); return;
    }
    $('firmware-progress').value = 100;
    $('firmware-status').textContent = 'Firmware accepted. The device is restarting; this page will reload after reconnection.';
    notice('Firmware accepted - waiting for restart …');
    waitForDeviceBoot = true; rebootCheckStarted = Date.now(); setOnline(false);
  };
  request.onerror = request.ontimeout = request.onabort = () => {
    setOnline(false);
    firmwareFailed('No update confirmation received. Check the connection and firmware version before uploading again.');
  };
  request.send(data);
});
connect(); poll(); setInterval(poll, 2000);
</script>
</body></html>)HTML";
#endif
