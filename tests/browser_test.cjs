// Browser/UI integration against a simulated device API, NOT a hardware test.
// PLAYWRIGHT_MODULE=/path/to/playwright-core node tests/browser_test.cjs
const assert = require('node:assert/strict');
const fs = require('node:fs');
const http = require('node:http');
const path = require('node:path');
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright-core');

(async () => {
  const html = fs.readFileSync(path.join(__dirname, '../index_html.h'), 'utf8').split('R"HTML(')[1].split(')HTML"')[0];
  const c = {persisted: true, configRevision: 1, name: 'Test-Rollladen', version: '1.5.0', currentPosition: 125,
    startPosition: 100, maxPosition: 1100, clockwise: false, mqtt_server: 'broker.local', mqtt_port: '1883', mqtt_uid: 'test-user',
    mqtt_password_set: true, inputTopic: '/raw/esp8266/123/in', outputTopic: '/raw/esp8266/123/out'};
  const s = {type: 'status', set: 20, position: 20, currentPosition: 300, targetPosition: 300, startPosition: 100, maxPosition: 1100,
    moving: false, direction: 'stopped', clockwise: false, mqttConnected: true, configRevision: 1, saveError: ''};
  const calls = [];
  let offline = false, failSave = false;
  const server = http.createServer((req, res) => { res.writeHead(200, {'Content-Type':'text/html; charset=utf-8'}); res.end(html); });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  const browser = await chromium.launch({executablePath: process.env.CHROME_PATH || '/usr/bin/google-chrome', headless: true, args: ['--no-sandbox']});
  try {
    const page = await browser.newPage({viewport: {width: 420, height: 900}});
    const errors = [];
    page.on('pageerror', e => errors.push(e.message));
    await page.addInitScript(() => {
      window.WebSocket = class { constructor() { queueMicrotask(() => this.onopen?.()); } close() {} };
    });
    await page.route('**/api/**', async route => {
      if (offline) return route.abort();
      const req = route.request(), endpoint = new URL(req.url()).pathname;
      const values = Object.fromEntries(new URLSearchParams(req.postData() || ''));
      if (req.method() === 'POST') {
        calls.push({endpoint, values});
        if (failSave) return route.fulfill({status:500, json:{error:'Speichern im Flash fehlgeschlagen.'}});
        if (endpoint === '/api/limits') { c.startPosition = Number(values.startPosition); c.maxPosition = Number(values.maxPosition); }
        if (endpoint === '/api/direction') c.clockwise = values.clockwise === 'true';
        if (endpoint === '/api/mqtt') {
          c.mqtt_server = values.mqtt_server; c.mqtt_port = values.mqtt_port; c.mqtt_uid = values.mqtt_uid;
          if (values.mqtt_pwd) c.mqtt_password_set = true;
          if (values.clear_password === 'true') c.mqtt_password_set = false;
        }
        if (endpoint === '/api/command') {
          const v = values.command;
          if (v === '(start)') c.startPosition = s.currentPosition;
          else if (v === '(max)') c.maxPosition = s.currentPosition;
          else if (v === '(1)' || v === '(-1)') { s.moving = true; s.direction = v === '(1)' ? 'down' : 'up'; }
          else if (/^\d+$/.test(v)) { s.set = Number(v); s.moving = true; s.direction = 'down'; }
          else { s.moving = false; s.direction = 'stopped'; }
        }
        if (endpoint !== '/api/command' || ['(0)', '(start)', '(max)'].includes(values.command)) {
          c.currentPosition = s.currentPosition; c.configRevision++; s.configRevision = c.configRevision;
          s.moving = false; s.direction = 'stopped';
        }
      }
      await route.fulfill({json: endpoint === '/api/status' || endpoint === '/api/command' ? s : c});
    });
    await page.goto(`http://127.0.0.1:${server.address().port}`);
    await page.waitForFunction(() => document.querySelector('#mqtt-server').value === 'broker.local');
    assert.equal(await page.locator('#position').textContent(), '20');
    assert.match(await page.locator('#saved-values').textContent(), /125 Schritte/);
    assert.equal(await page.locator('#mqtt-user').inputValue(), 'test-user');
    assert.equal(await page.locator('#mqtt-password').inputValue(), '');
    assert.match(await page.locator('#password-state').textContent(), /Passwort ist gespeichert/);
    console.log('PASS: loaded saved values, live position and masked password');

    await page.locator('#start-position').fill('200');
    c.startPosition = 150; c.configRevision++; s.configRevision = c.configRevision;
    await page.waitForFunction(() => document.querySelector('#saved-values').textContent.includes('150 Schritte'));
    assert.equal(await page.locator('#start-position').inputValue(), '200');
    await page.locator('#end-position').fill('100');
    const before = calls.length;
    await page.locator('#limits-form button').click();
    assert.match(await page.locator('#notice').textContent(), /Anfang muss kleiner/);
    assert.equal(calls.length, before);
    await page.locator('#end-position').fill('900');
    await page.locator('#limits-form button').click();
    await page.waitForFunction(() => document.querySelector('#saved-values').textContent.includes('900 Schritte'));
    assert.deepEqual(calls.at(-1), {endpoint:'/api/limits', values:{startPosition:'200', maxPosition:'900'}});
    assert.equal(s.currentPosition, 300);
    await page.getByRole('button', {name: 'Aktuell als Anfang (offen)', exact:true}).click();
    await page.waitForFunction(() => document.querySelector('#start-position').value === '300');
    assert.equal(c.startPosition, 300); // No rebasing to zero.
    console.log('PASS: endpoint editing, validation, dirty drafts, set-current calibration');

    await page.locator('#clockwise').selectOption('true');
    await page.locator('#direction-form button').click();
    await page.waitForFunction(() => document.querySelector('#saved-values').textContent.includes('Im Uhrzeigersinn'));
    assert.equal(c.clockwise, true); assert.equal(s.moving, false);
    console.log('PASS: runtime direction setting');

    await page.locator('#mqtt-server').fill('192.168.1.50');
    await page.locator('#mqtt-port').fill('1884');
    await page.locator('#mqtt-form button').click();
    await page.waitForFunction(() => document.querySelector('#saved-values').textContent.includes('192.168.1.50'));
    assert.equal(c.mqtt_password_set, true); assert.equal(calls.at(-1).values.mqtt_pwd, '');
    await page.locator('#mqtt-password').fill('test-only-password');
    await page.locator('#mqtt-form button').click();
    await page.waitForFunction(() => document.querySelector('#mqtt-password').value === '');
    assert.equal(calls.at(-1).values.mqtt_pwd, 'test-only-password');
    assert.ok(!(await page.locator('body').textContent()).includes('test-only-password'));
    await page.locator('#clear-password').check();
    await page.locator('#mqtt-form button').click();
    await page.waitForFunction(() => document.querySelector('#password-state').textContent.includes('Kein Passwort'));
    assert.equal(c.mqtt_password_set, false);
    console.log('PASS: MQTT settings, password keep/replace/clear without echo');

    await page.getByRole('button', {name: 'Manuell auf ↑', exact:true}).click();
    await page.waitForFunction(() => document.querySelector('#motion-state').textContent.includes('Fährt auf'));
    await page.getByRole('button', {name: 'Stopp', exact:true}).first().click();
    await page.waitForFunction(() => document.querySelector('#motion-state').textContent === 'Antrieb steht');
    await page.locator('#position-slider').fill('50');
    await page.locator('#position-slider').dispatchEvent('change');
    await page.waitForFunction(() => document.querySelector('#motion-state').textContent.includes('Fährt ab'));
    assert.equal(calls.at(-1).values.command, '50');
    console.log('PASS: manual move, stop and automatic percentage command');

    failSave = true;
    await page.locator('#start-position').fill('350');
    await page.locator('#limits-form button').click();
    await page.waitForFunction(() => document.querySelector('#notice').textContent.includes('fehlgeschlagen'));
    assert.equal(c.startPosition, 300);
    assert.equal(await page.locator('#start-position').inputValue(), '350');
    failSave = false;
    console.log('PASS: flash-save error is visible and unsaved draft remains');

    offline = true;
    await page.waitForFunction(() => document.querySelector('#position-slider').disabled);
    offline = false;
    await page.waitForFunction(() => !document.querySelector('#position-slider').disabled);
    assert.equal(await page.locator('#start-position').inputValue(), '350');
    console.log('PASS: disconnect/reconnect with preserved draft');

    c.name = '<img src=x onerror=alert(1)>';
    c.configRevision++; s.configRevision = c.configRevision;
    await page.waitForFunction(() => document.querySelector('#device-name').textContent.startsWith('<img'));
    assert.equal(await page.locator('#device-name img').count(), 0);
    assert.equal(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth), true);
    c.name = 'Test-Rollladen';
    await page.reload();
    await page.waitForFunction(() => document.querySelector('#mqtt-server').value === '192.168.1.50');
    assert.equal(await page.locator('#mqtt-port').inputValue(), '1884');
    assert.equal(await page.locator('#clockwise').inputValue(), 'true');
    assert.equal(await page.locator('#start-position').inputValue(), '300');
    assert.deepEqual(errors, []);
    if (process.env.SCREENSHOT_PATH) await page.screenshot({path:process.env.SCREENSHOT_PATH, fullPage:true});
    console.log('PASS: reload reflects saved settings, safe text rendering, mobile layout, no JS errors');
  } finally { await browser.close(); await new Promise(resolve => server.close(resolve)); }
})().catch(e => { console.error(e); process.exitCode = 1; });
