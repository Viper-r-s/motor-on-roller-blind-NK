// Real HTML and XMLHttpRequest against a simulated device API; no real flashing.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const http = require('node:http');
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright-core');
(async () => {
  const html = fs.readFileSync(path.join(__dirname,'../index_html.h'),'utf8').split('R"HTML(')[1].split(')HTML"')[0];
  const c = {persisted:true,configRevision:1,name:'Update-Test',version:'1.6.0',currentPosition:125,startPosition:100,maxPosition:1100,
    clockwise:false,mqtt_server:'broker.local',mqtt_port:'1883',mqtt_uid:'test-user',mqtt_password_set:true,
    inputTopic:'/test/in',outputTopic:'/test/out',firmwareMaxSize:2048,firmwareToken:'test-boot-1'};
  const s = {set:20,position:20,currentPosition:300,targetPosition:300,moving:false,direction:'stopped',mqttConnected:true,configRevision:1,saveError:''};
  let attempts=0, mode='error', finishRequest, heldResolve;
  const heldRequest = new Promise(resolve => { heldResolve = resolve; });
  const server = http.createServer((req,res)=>{res.writeHead(200,{'Content-Type':'text/html; charset=utf-8'});res.end(html);});
  await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
  const browser = await chromium.launch({executablePath:process.env.CHROME_PATH||'/usr/bin/google-chrome',headless:true,args:['--no-sandbox']});
  try {
    const page=await browser.newPage({viewport:{width:420,height:900}}), errors=[];
    page.on('pageerror',e=>errors.push(e.message));
    await page.addInitScript(()=>{
      window.WebSocket=class{constructor(){queueMicrotask(()=>this.onopen?.());}close(){}};
      const RealXHR = window.XMLHttpRequest;
      window.XMLHttpRequest = class extends RealXHR { send(...args) { window.__firmwareXHR = this; return super.send(...args); } };
    });
    await page.route('**/api/**',async route=>{
      const req=route.request(), endpoint=new URL(req.url()).pathname;
      if(endpoint==='/api/firmware') {
        attempts++;
        assert.equal(req.headers()['x-firmware-token'],'test-boot-1');
        assert.equal(req.headers()['x-firmware-size'],'1024');
        assert.match(req.headers()['content-type'],/^multipart\/form-data; boundary=/);
        const body=req.postDataBuffer();
        assert.ok(body.includes(Buffer.from('name="firmware"; filename="firmware.bin"')));
        assert.ok(body.includes(Buffer.from([0xE9,1,0,0x40,0xB8,0xF0,0x10,0x40])));
        if(mode==='network') return route.abort();
        if(mode==='error') return route.fulfill({status:500,json:{error:'Position konnte nicht gespeichert werden. Firmware-Update abgebrochen.'}});
        if(mode==='hold') await new Promise(resolve=>{finishRequest=resolve;heldResolve();});
        return route.fulfill({json:{ok:true,rebooting:true}});
      }
      return route.fulfill({json:endpoint==='/api/config'?c:s});
    });
    await page.goto(`http://127.0.0.1:${server.address().port}`);
    await page.waitForFunction(()=>document.querySelector('#firmware-info').textContent.includes('2 KiB'));
    const image=Buffer.alloc(1024,0xAB);Buffer.from([0xE9,1,0,0x40,0xB8,0xF0,0x10,0x40]).copy(image);
    const select=async(name,buffer)=>page.locator('#firmware-file').setInputFiles({name,mimeType:'application/octet-stream',buffer});
    await select('firmware.bin',image);
    assert.equal(await page.locator('#firmware-submit').isDisabled(),true);
    await page.locator('#firmware-confirm').check();
    assert.equal(await page.locator('#firmware-submit').isEnabled(),true);
    console.log('PASS: visible firmware version/capacity and explicit confirmation');

    await select('archive.zip',image);await page.locator('#firmware-submit').click();
    assert.match(await page.locator('#notice').textContent(),/\.bin/);assert.equal(attempts,0);
    await select('firmware.bin',Buffer.alloc(3000));await page.locator('#firmware-submit').click();
    assert.match(await page.locator('#notice').textContent(),/größer/);assert.equal(attempts,0);
    await select('firmware.bin',Buffer.alloc(1024));await page.locator('#firmware-submit').click();
    await page.waitForFunction(()=>document.querySelector('#notice').textContent.includes('Keine passende ESP8266'));
    assert.equal(attempts,0);
    console.log('PASS: ZIP, oversize and invalid binary rejected before upload');

    await select('firmware.bin',image);await page.locator('#firmware-submit').click();
    await page.waitForFunction(()=>document.querySelector('#firmware-status').textContent.includes('Position konnte nicht gespeichert'));
    assert.equal(attempts,1);assert.equal(await page.locator('#position-slider').isEnabled(),true);
    assert.equal(await page.locator('#firmware-submit').isEnabled(),true);
    console.log('PASS: multipart bytes/headers and firmware save error shown, controls released');

    mode='network';await page.locator('#firmware-submit').click();
    await page.waitForFunction(()=>document.querySelector('#firmware-status').textContent.includes('Keine Update-Bestätigung'));
    assert.equal(attempts,2);
    await page.waitForFunction(()=>!document.querySelector('#firmware-submit').disabled);
    console.log('PASS: disconnected upload is unconfirmed, not falsely successful');

    mode='hold';await page.locator('#firmware-submit').click();
    await heldRequest;
    // Intercepted requests do not transmit, so inject the browser progress event.
    await page.evaluate(() => window.__firmwareXHR.upload.dispatchEvent(new ProgressEvent('progress', {lengthComputable:true,loaded:1024,total:1024})));
    await page.waitForFunction(()=>document.querySelector('#firmware-progress').value===100);
    assert.equal(attempts,3);
    assert.equal(await page.locator('#position-slider').isDisabled(),true);
    assert.equal(await page.locator('#mqtt-server').isDisabled(),true);
    assert.match(await page.locator('#firmware-status').textContent(),/Firmware wird geprüft/);
    const before=await page.evaluate(()=>performance.timeOrigin);
    finishRequest();
    await page.waitForFunction(()=>document.querySelector('#firmware-status').textContent.includes('Gerät startet neu'));
    assert.equal(await page.locator('#position-slider').isDisabled(),true);
    // Same token means the old process is still serving: do not falsely confirm reboot.
    await page.waitForFunction(()=>document.querySelector('#connection').textContent.includes('warte auf Neustart'));
    await page.waitForResponse(r=>new URL(r.url()).pathname==='/api/config');
    assert.equal(await page.evaluate(()=>performance.timeOrigin),before);
    c.firmwareToken='test-boot-2';c.version='1.6.1';
    await page.waitForFunction(()=>document.querySelector('#firmware-info').textContent.includes('1.6.1'));
    assert.notEqual(await page.evaluate(()=>performance.timeOrigin),before);
    assert.equal(await page.locator('#position-slider').isEnabled(),true);
    assert.match(await page.locator('#saved-values').textContent(),/100 Schritte/);
    assert.match(await page.locator('#saved-values').textContent(),/1100 Schritte/);
    assert.deepEqual(errors,[]);
    assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),true);
    if(process.env.SCREENSHOT_PATH)await page.screenshot({path:process.env.SCREENSHOT_PATH,fullPage:true});
    console.log('PASS: progress, motion/settings lock, success response, new-boot detection, page reload, preserved settings and mobile layout');
  } finally {if(finishRequest)finishRequest();await browser.close();await new Promise(resolve=>server.close(resolve));}
})().catch(e=>{console.error(e);process.exitCode=1;});
