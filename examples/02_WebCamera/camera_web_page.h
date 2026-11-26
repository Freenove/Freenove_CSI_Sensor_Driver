/*
 * Embedded index page for 02_WebCamera (edit as normal HTML inside the raw string).
 */
#pragma once

static const char CAMERA_WEB_PAGE_HTML[] = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>CSI Web Camera</title>
<style>
*{box-sizing:border-box}
body{margin:0;font:14px/1.4 system-ui,sans-serif;background:#12141a;color:#e8eaed;display:flex;min-height:100vh}
#side{width:300px;flex:0 0 300px;padding:16px;background:#1c1f28;border-right:1px solid #2a2f3a;overflow:auto}
#main{flex:1;padding:16px;display:flex;flex-direction:column;gap:12px;min-width:0}
h1{font-size:16px;margin:0 0 12px;font-weight:600}
h2{font-size:12px;margin:16px 0 8px;color:#9aa0a6;text-transform:uppercase;letter-spacing:.04em}
label{display:block;margin:8px 0 4px;color:#bdc1c6}
button{width:100%;padding:8px 10px;border-radius:6px;border:1px solid #3c4048;background:#292c36;color:#e8eaed;cursor:pointer;margin-top:8px}
button:hover{background:#353945}
button.pri{background:#1a73e8;border-color:#1a73e8}
button.pri:hover{background:#1967d2}
.row{display:flex;gap:8px;align-items:center;margin:6px 0}
.row label{margin:0;flex:1}
input[type=checkbox]{width:18px;height:18px}
#meta{font-size:12px;color:#9aa0a6;margin:0}
#log{font:12px/1.35 ui-monospace,monospace;white-space:pre-wrap;background:#0e1015;border:1px solid #2a2f3a;border-radius:6px;padding:8px;min-height:72px;max-height:160px;overflow:auto}
#view{background:#000;border-radius:8px;display:block;width:auto;height:auto;max-width:none;image-rendering:auto}
a{color:#8ab4f8}
</style>
</head>
<body>
<aside id="side">
  <h1>API Control</h1>
  <div id="meta">loading…</div>
  <h2>setFlip(hmirror, vflip)</h2>
  <div class="row"><label for="hm">hmirror</label><input type="checkbox" id="hm"></div>
  <div class="row"><label for="vf">vflip</label><input type="checkbox" id="vf"></div>
  <button class="pri" id="btnFlip">Apply setFlip</button>
  <h2>Read-only</h2>
  <div id="ro" style="font-size:12px;color:#9aa0a6"></div>
  <h2>Log</h2>
  <div id="log"></div>
</aside>
<main id="main">
  <h1>ESP32-P4 CSI Stream</h1>
  <img id="view" src="/stream" alt="stream" width="320" height="240">
  <p><a href="/jpg" target="_blank">/jpg</a> · <a href="/status" target="_blank">/status</a></p>
</main>
<script>
const logEl = document.getElementById('log');
function log(m) {
  logEl.textContent = (new Date().toLocaleTimeString()) + '  ' + m + '\n' + logEl.textContent;
}
async function jget(u) {
  const r = await fetch(u, {cache: 'no-store'});
  return r.json();
}
function applyViewSize(w, h) {
  const img = document.getElementById('view');
  if (w > 0 && h > 0) {
    img.width = w;
    img.height = h;
    img.style.width = w + 'px';
    img.style.height = h + 'px';
  }
}
async function loadStatus() {
  const s = await jget('/status');
  document.getElementById('meta').textContent =
    s.sensor + '  ' + s.width + 'x' + s.height + '  ' + s.framesizeName;
  document.getElementById('hm').checked = !!s.hmirror;
  document.getElementById('vf').checked = !!s.vflip;
  applyViewSize(s.width, s.height);
  document.getElementById('ro').innerHTML =
    'streamQuality=' + s.streamQuality + '<br>photoQuality=' + s.photoQuality +
    '<br>sensorId=' + s.sensorId;
  return s;
}
document.getElementById('btnFlip').onclick = async () => {
  const h = document.getElementById('hm').checked ? 1 : 0;
  const v = document.getElementById('vf').checked ? 1 : 0;
  log('setFlip(' + h + ',' + v + ')…');
  const r = await jget('/api/flip?h=' + h + '&v=' + v);
  log('setFlip(' + h + ',' + v + ') → ' + JSON.stringify(r));
  await loadStatus();
};
(async () => {
  try { await loadStatus(); } catch (e) { log(String(e)); }
})();
</script>
</body>
</html>
)=====";
