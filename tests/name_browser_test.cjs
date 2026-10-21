// Real English page, mocked name/config API; no real DNS, DHCP or device reboot.
const assert=require('node:assert/strict'),fs=require('node:fs'),http=require('node:http'),path=require('node:path');
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright-core');
(async()=>{
  const html=fs.readFileSync(path.join(__dirname,'../index_html.h'),'utf8').split('R"HTML(')[1].split(')HTML"')[0];
  const c={persisted:true,configRevision:1,name:'roller-blind',version:'1.7.0',ipAddress:'192.168.1.22',restartPending:false,
    currentPosition:321,startPosition:-100,maxPosition:1900,clockwise:true,mqtt_server:'broker.local',mqtt_port:'1883',mqtt_uid:'test-user',mqtt_password_set:true,
    inputTopic:'/raw/esp8266/123/in',outputTopic:'/raw/esp8266/123/out',firmwareMaxSize:1000000,firmwareToken:'boot-before-rename'};
  const s={set:21,position:21,currentPosition:321,targetPosition:321,moving:false,direction:'stopped',mqttConnected:true,configRevision:1,saveError:''};
  let failure=false,calls=0,completeBoot;
  const server=http.createServer((req,res)=>{res.writeHead(200,{'Content-Type':'text/html; charset=utf-8'});res.end(html);});
  await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
  const browser=await chromium.launch({executablePath:process.env.CHROME_PATH||'/usr/bin/google-chrome',headless:true,args:['--no-sandbox']});
  try {
    const page=await browser.newPage({viewport:{width:420,height:900}}),errors=[];
    page.on('pageerror',e=>errors.push(e.message));
    await page.addInitScript(()=>{window.WebSocket=class{constructor(){queueMicrotask(()=>this.onopen?.());}close(){}};});
    await page.route('**/api/**',async route=>{
      const req=route.request(),endpoint=new URL(req.url()).pathname;
      if(endpoint==='/api/name'){
        calls++;const fields=Object.fromEntries(new URLSearchParams(req.postData()));
        assert.equal(fields.name,fields.name.toLowerCase());
        if(failure)return route.fulfill({status:500,json:{error:'Failed to save to flash. Previous settings have been retained.'}});
        if(!/^[a-z0-9](?:[a-z0-9-]{0,29}[a-z0-9])?$/.test(fields.name))return route.fulfill({status:400,json:{error:'Invalid device name.'}});
        if(fields.name!==c.name){c.name=fields.name;c.configRevision++;c.restartPending=true;s.configRevision=c.configRevision;}
        return route.fulfill({json:c});
      }
      if(endpoint==='/api/config'&&c.restartPending){
        if(completeBoot){c.firmwareToken='boot-after-rename';c.restartPending=false;}
      }
      return route.fulfill({json:endpoint==='/api/status'?s:c});
    });
    await page.goto(`http://127.0.0.1:${server.address().port}`);
    await page.waitForFunction(()=>document.querySelector('#device-name-input').value==='roller-blind');
    assert.equal(await page.locator('html').getAttribute('lang'),'en');
    const headings=await page.locator('h2').allTextContents();
    assert.deepEqual(headings,['Position & control','Saved settings','Device name','Adjust start and end positions','Rotation direction','MQTT settings','Firmware update']);
    assert.equal(await page.locator('#device-local-link').getAttribute('href'),'http://roller-blind.local/');
    assert.equal(await page.locator('#device-ip-link').getAttribute('href'),'http://192.168.1.22/');
    assert.match(await page.locator('#saved-values').textContent(),/Device nameroller-blind/);
    assert.equal(await page.locator('#motion-state').textContent(),'Motor stopped');
    console.log('PASS: English page, saved name, hostname/IP links and control labels');

    await page.locator('#device-name-input').fill('draft-name');
    c.configRevision++;s.configRevision=c.configRevision;
    await page.waitForResponse(r=>new URL(r.url()).pathname==='/api/config');
    assert.equal(await page.locator('#device-name-input').inputValue(),'draft-name');
    for(const invalid of ['two rooms','room.local','-room','room-','roller_blind']){
      await page.locator('#device-name-input').fill(invalid);
      await page.locator('#name-form button').click();
      assert.equal(await page.locator('#device-name-input').evaluate(el=>el.validity.patternMismatch),true);
    }
    assert.equal(calls,0);
    console.log('PASS: background updates preserve the draft; invalid hostnames blocked');

    failure=true;await page.locator('#device-name-input').fill('unsaved-name');await page.locator('#name-form button').click();
    await page.waitForFunction(()=>document.querySelector('#name-status').textContent.includes('Failed to save'));
    assert.equal(c.name,'roller-blind');assert.equal(c.restartPending,false);
    assert.equal(await page.locator('#device-name-input').inputValue(),'unsaved-name');
    assert.equal(await page.locator('#position-slider').isEnabled(),true);
    console.log('PASS: name-save failure is English, keeps old name, draft and controls');

    failure=false;await page.locator('#device-name-input').fill('roller-blind');await page.locator('#name-form button').click();
    await page.waitForFunction(()=>document.querySelector('#name-status').textContent.includes('unchanged'));
    assert.equal(c.restartPending,false);assert.equal(await page.locator('#position-slider').isEnabled(),true);
    console.log('PASS: unchanged name does not require restart');

    await page.locator('#device-name-input').fill('Bedroom-Blind');
    const before=await page.evaluate(()=>performance.timeOrigin);
    await page.locator('#name-form button').click();
    await page.waitForFunction(()=>document.querySelector('#name-status').textContent.includes('device is restarting'));
    assert.equal(c.name,'bedroom-blind');assert.equal(await page.locator('#device-name').textContent(),'bedroom-blind');
    assert.equal(await page.locator('#device-local-link').getAttribute('href'),'http://bedroom-blind.local/');
    assert.equal(await page.locator('#position-slider').isDisabled(),true);
    assert.equal(await page.locator('#firmware-file').isDisabled(),true);
    assert.equal(await page.locator('#mqtt-server').isDisabled(),true);
    await page.waitForResponse(r=>new URL(r.url()).pathname==='/api/config');
    assert.equal(await page.evaluate(()=>performance.timeOrigin),before);
    completeBoot=true;
    await page.waitForFunction(()=>document.querySelector('#device-name-input').value==='bedroom-blind'&&!document.querySelector('#position-slider').disabled);
    assert.notEqual(await page.evaluate(()=>performance.timeOrigin),before);
    assert.equal(await page.locator('#start-position').inputValue(),'-100');
    assert.equal(await page.locator('#end-position').inputValue(),'1900');
    assert.equal(await page.locator('#input-topic').textContent(),'/raw/esp8266/123/in');
    assert.equal(await page.locator('#mqtt-server').inputValue(),'broker.local');
    assert.deepEqual(errors,[]);
    assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth),true);
    if(process.env.SCREENSHOT_PATH)await page.screenshot({path:process.env.SCREENSHOT_PATH,fullPage:true});
    console.log('PASS: lowercase saved name, restart lock, new-boot reload, retained limits/MQTT and mobile layout');
  }finally{await browser.close();await new Promise(resolve=>server.close(resolve));}
})().catch(e=>{console.error(e);process.exitCode=1;});
