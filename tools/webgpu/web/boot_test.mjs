// Boots the browser build in headless Chromium with a software (SwiftShader)
// WebGPU adapter, prints the console and takes screenshots.
//
// Usage (with serve.py running):
//   node tools/webgpu/web/boot_test.mjs [URL] [WAIT_MS] [SCREENSHOT] [STEPS_JSON]
// STEPS_JSON runs after the wait, e.g. to click through the first-run screens:
//   '[{"click":[457,540]},{"wait":2000},{"key":"Enter"},{"type":"text"},{"shot":"b.png"}]'
// {"down":"ArrowUp"} / {"up":"ArrowUp"} hold and release a key.
// DPR=2 in the environment emulates a HiDPI display (clicks stay in CSS px).
// GAMEPAD=1 adds a fake standard gamepad (connected after 3 s);
// {"pad":[button, 0|1]} and {"axis":[index, value]} change its state.
//
// Needs Playwright (global npm module) and its Chromium. ANGLE/SwiftShader GL
// is required for the compositor: without it Chromium cannot create the
// shared image behind a WebGPU canvas and the device is lost on first present.
import { createRequire } from 'module';
import { execSync } from 'child_process';

const require = createRequire(execSync('npm root -g').toString().trim() + '/');
const { chromium } = require('playwright');

const url = process.argv[2] || 'http://localhost:8080/';
const waitMs = Number(process.argv[3] || 60000);
const shot = process.argv[4] || 'boot.png';
const steps = JSON.parse(process.argv[5] || '[]');

const browser = await chromium.launch({
  channel: 'chromium',
  headless: true,
  args: ['--enable-unsafe-webgpu', '--ignore-gpu-blocklist',
         '--enable-features=Vulkan', '--use-vulkan=swiftshader',
         '--use-webgpu-adapter=swiftshader',
         '--use-gl=angle', '--use-angle=swiftshader'],
});
const page = await browser.newPage({ viewport: { width: 1280, height: 720 },
  deviceScaleFactor: Number(process.env.DPR || 1) });
const start = Date.now();
const t = () => ((Date.now() - start) / 1000).toFixed(1).padStart(6);
// Strip the terminal colour codes of STK's log
const clean = (s) => s.replace(/\x1b\[[0-9;]*m|\[0;;m/g, '');
page.on('console', (m) => console.log(`${t()} [${m.type()}] ${clean(m.text())}`));
page.on('pageerror', (e) => console.log(`${t()} [pageerror] ${e.stack || e.message}`));
page.on('crash', () => console.log(`${t()} [crash]`));

if (process.env.GAMEPAD) {
  // navigator.getGamepads() is what SDL's Emscripten joystick driver polls
  await page.addInitScript(() => {
    const pad = {
      id: 'Fake Gamepad (STANDARD GAMEPAD)', index: 0, connected: true,
      mapping: 'standard', timestamp: performance.now(), axes: [0, 0, 0, 0],
      buttons: Array.from({ length: 17 },
        () => ({ pressed: false, touched: false, value: 0 })),
    };
    window.__fakePad = pad;
    navigator.getGamepads = () => [pad, null, null, null];
    setTimeout(() => {
      const event = new Event('gamepadconnected');
      event.gamepad = pad;
      window.dispatchEvent(event);
    }, 3000);
  });
}

await page.goto(url);
const deadline = Date.now() + waitMs;
let lastStatus = '';
while (Date.now() < deadline) {
  await page.waitForTimeout(1000);
  const status = await page.evaluate(() => {
    const o = document.getElementById('overlay');
    return o && !o.hidden ?
      `${document.getElementById('title').textContent} | ${document.getElementById('status').textContent}` :
      'running';
  }).catch((e) => `evaluate failed: ${e.message}`);
  if (status !== lastStatus) {
    console.log(`${t()} [status] ${status}`);
    lastStatus = status;
  }
}
await page.screenshot({ path: shot });
console.log(`${t()} [shot] ${shot}`);

for (const step of steps) {
  if (step.click) await page.mouse.click(step.click[0], step.click[1]);
  if (step.key) await page.keyboard.press(step.key);
  if (step.down) await page.keyboard.down(step.down);
  if (step.up) await page.keyboard.up(step.up);
  if (step.type) await page.keyboard.type(step.type, { delay: 50 });
  if (step.wait) await page.waitForTimeout(step.wait);
  if (step.pad) {
    await page.evaluate(([b, v]) => {
      const pad = window.__fakePad;
      pad.buttons[b] = { pressed: !!v, touched: !!v, value: v };
      pad.timestamp = performance.now();
    }, step.pad);
  }
  if (step.axis) {
    await page.evaluate(([a, v]) => {
      const pad = window.__fakePad;
      pad.axes[a] = v;
      pad.timestamp = performance.now();
    }, step.axis);
  }
  if (step.shot) {
    await page.screenshot({ path: step.shot });
    console.log(`${t()} [shot] ${step.shot}`);
  }
}
await browser.close();
