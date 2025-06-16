const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Autoptiks</title>
  <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap" rel="stylesheet">
  <link rel="icon" href="data:,">
  <style>
    :root {
      --primary: #0f8b8d;
      --bg: #f0f2f5;
      --card: #fff;
      --text: #333;
      --accent: #143642;
      --radius: 8px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    html,body { height:100%; font-family:'Roboto',sans-serif; background:var(--bg); color:var(--text); }
    .topnav { background:var(--accent); padding:1rem; text-align:center; }
    .topnav h1 { color:#fff; font-size:2rem; text-transform:uppercase; letter-spacing:1px; }
    .content { display:flex; justify-content:center; align-items:flex-start; padding:2rem; }
    .card { background:var(--card); border-radius:var(--radius); box-shadow:0 4px 12px rgba(0,0,0,0.1); width:100%; max-width:400px; padding:2rem; }
    .card h2 { margin-bottom:1rem; color:var(--accent); font-size:1.5rem; text-transform:uppercase; letter-spacing:0.5px; }
    .state { margin-bottom:1.5rem; font-size:1.2rem; }
    .dropdown { position:relative; margin-bottom:1.5rem; }
    .dropbtn { width:100%; background:var(--primary); color:#fff; padding:.75rem; font-size:1.1rem; border:none; border-radius:var(--radius); cursor:pointer; transition:background 0.3s; }
    .dropbtn:hover { background:#0e7b7d; }
    .dropdown-content { display:none; position:absolute; top:110%; left:0; background:var(--card); box-shadow:0 4px 12px rgba(0,0,0,0.1); border-radius:var(--radius); width:100%; overflow:hidden; transition:opacity 0.3s; opacity:0; }
    .dropdown:hover .dropdown-content { display:block; opacity:1; }
    .dropdown-content a { display:block; padding:.75rem 1rem; text-decoration:none; color:var(--text); transition:background 0.2s; }
    .dropdown-content a:hover { background:var(--bg); }
    .color-picker { display:flex; align-items:center; gap:.5rem; margin-top:1rem; }
    .color-picker input[type="color"] { border:none; width:2.5rem; height:2.5rem; cursor:pointer; }
    .color-preview { width:2.5rem; height:2.5rem; border-radius:50%; border:1px solid #ccc; }
    .button { background:var(--primary); color:#fff; border:none; border-radius:var(--radius); padding:.75rem 1.5rem; cursor:pointer; transition:transform 0.1s; }
    .button:active { transform:translateY(1px); }
    .slider { width:100%; margin-top:1rem; }
  </style>
</head>
<body>
  <div class="topnav"><h1>Autoptiks</h1></div>
  <div class="content">
    <div class="card">
      <h2>Configuración</h2>
      <p class="state">Estado: <span id="state">%STATE%</span></p>
      <div class="dropdown">
        <button class="dropbtn">Modo</button>
        <div class="dropdown-content">
          <a onclick="sendString('tombo')">Modo Tombo</a>
          <a onclick="sendString('rainbow')">Rainbow RGB</a>
          <a onclick="sendString('glowy')">Glowy White</a>
          <a onclick="sendString('static')">Static Color</a>
          <a onclick="sendString('off')">Off</a>
        </div>
      </div>
      <div class="color-picker">
        <label for="colorPicker">Color:</label>
        <input type="color" id="colorPicker" value="#ffffff">
        <div id="colorPreview" class="color-preview"></div>
        <button class="button" id="setColorBtn">Set Color</button>
      </div>
      <div class="slider">
        <label for="brightnessSlider">Brillo:</label>
        <input type="range" id="brightnessSlider" min="0" max="255" value="255" oninput="updateBrightnessPreview(this.value)">
      </div>
      <button class="button" id="setBrightnessBtn">Set Brightness</button>
    </div>
  </div>
  <script>
    const gateway = `ws://${window.location.hostname}/ws`;
    let websocket;
    window.addEventListener('load', () => {
      websocket = new WebSocket(gateway);
      websocket.onopen = () => console.log('WS open');
      websocket.onclose = () => setTimeout(()=>location.reload(),2000);
      websocket.onmessage = evt => {
        document.getElementById('state').textContent = (evt.data=='1')?'ON':'OFF';
      };
      document.getElementById('setColorBtn').onclick = () => {
        const col = document.getElementById('colorPicker').value.substring(1);
        websocket.send(`color:${col}`);
      };
      document.getElementById('setBrightnessBtn').onclick = () => {
        const b = document.getElementById('brightnessSlider').value;
        websocket.send(`bright:${b}`);
      };
      document.getElementById('colorPicker').oninput = (e) => {
        document.getElementById('colorPreview').style.background = e.target.value;
      };
    });
    function sendString(str){
      if(websocket && websocket.readyState===1) websocket.send(str);
    }
    function updateBrightnessPreview(val){
      // optional: update UI preview
    }
  </script>
</body>
</html>
)rawliteral";
