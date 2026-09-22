// Panel web de Sensei — implementa el mismo flujo que la app Android
// (docs/protocolo.md): BLE para configurar WiFi -> IP por BLE -> WebSocket
// para los datos de sensores. Sin frameworks, solo Web Bluetooth + WebSocket.
//
// Requiere Chrome o Edge (computadora o Android) y correr desde un
// "contexto seguro": localhost sirve (ver web/servidor_local.py).

// ─── UUIDs del perfil GATT (docs/protocolo.md §2 — no modificar) ──────
const SERVICE_UUID    = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const UUID_WIFI_SCAN  = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const UUID_WIFI_CRED  = 'beb5483e-36e1-4688-b7f5-ea07361b26a9';
const UUID_WIFI_STAT  = 'beb5483e-36e1-4688-b7f5-ea07361b26aa';
const UUID_IP_ADDR    = 'beb5483e-36e1-4688-b7f5-ea07361b26ab';
const UUID_WIFI_SSID  = '243c4f09-ebd7-4026-940a-10afc3c8993e';
const UUID_WIFI_STATE = '4499e77a-31ec-4153-bcde-00e90bb808c9';

// Nombres bonitos para el campo "id" del JSON (docs/protocolo.md §5).
// "distancia" (HC-SR04) todavía no está confirmado con Android.
const NOMBRES_SENSOR = {
  ph: 'pH', voltaje: 'Voltaje', corriente: 'Corriente', temperatura: 'Temperatura',
  humedad: 'Humedad', co2: 'CO2', pulso: 'Pulso', luz: 'Luz', hall: 'Hall',
  distancia: 'Distancia',
};

const codificador   = new TextEncoder();
const decodificador = new TextDecoder();

// ─── Estado ────────────────────────────────────────────────────────
let dispositivo, chScan, chCred, chStat, chIp, chSsid, chState;
let socket = null;
let ssidRedActual = '';
let ipActual = '';

// ─── Utilidades de UI ───────────────────────────────────────────────
function mostrarPantalla(id) {
  document.querySelectorAll('.pantalla').forEach(s => { s.hidden = (s.id !== id); });
}
function actualizarBadge(texto) {
  document.getElementById('estadoConexion').textContent = texto;
}
function mostrarError(id, msg) {
  const el = document.getElementById(id);
  el.textContent = msg;
  el.hidden = false;
}
function ocultarError(id) {
  document.getElementById(id).hidden = true;
}
function mostrarWifiMensaje(msg) {
  document.getElementById('wifiMensaje').textContent = msg;
}
function mostrarWifiEstado(msg, esError) {
  const p = document.getElementById('wifiEstado');
  p.textContent = msg;
  p.hidden = false;
  p.classList.toggle('error', !!esError);
}
function escaparHtml(s) {
  return String(s).replace(/[&<>"']/g, c => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;',
  }[c]));
}
function esperar(ms) { return new Promise(r => setTimeout(r, ms)); }

// ─── Compatibilidad del navegador ───────────────────────────────────
if (!navigator.bluetooth) {
  document.getElementById('btnConectar').disabled = true;
  mostrarError('errorConectar',
    'Este navegador no soporta Web Bluetooth. Abre esta página en ' +
    'Chrome o Edge (computadora o Android).');
}

// ─── 1. Conectar por BLE ─────────────────────────────────────────────
document.getElementById('btnConectar').addEventListener('click', conectar);

async function conectar() {
  ocultarError('errorConectar');
  try {
    dispositivo = await navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: 'SENSEI-' }],
      optionalServices: [SERVICE_UUID],
    });
    dispositivo.addEventListener('gattserverdisconnected', alDesconectarBLE);

    const gatt = await dispositivo.gatt.connect();
    const servicio = await gatt.getPrimaryService(SERVICE_UUID);

    chScan  = await servicio.getCharacteristic(UUID_WIFI_SCAN);
    chCred  = await servicio.getCharacteristic(UUID_WIFI_CRED);
    chStat  = await servicio.getCharacteristic(UUID_WIFI_STAT);
    chIp    = await servicio.getCharacteristic(UUID_IP_ADDR);
    chSsid  = await servicio.getCharacteristic(UUID_WIFI_SSID);
    chState = await servicio.getCharacteristic(UUID_WIFI_STATE);

    await chStat.startNotifications();
    chStat.addEventListener('characteristicvaluechanged', onWifiStat);
    await chIp.startNotifications();
    chIp.addEventListener('characteristicvaluechanged', onIpAddr);
    await chSsid.startNotifications();
    chSsid.addEventListener('characteristicvaluechanged', onWifiSsid);

    actualizarBadge(`Conectado a ${dispositivo.name}`);

    // docs/protocolo.md §3: WIFI_STATE decide la pantalla.
    const estado = decodificador.decode(await chState.readValue());
    if (estado === 'CONNECTED') {
      try { ssidRedActual = decodificador.decode(await chSsid.readValue()); } catch (e) { /* sin notificación previa */ }
      try { ipActual = decodificador.decode(await chIp.readValue()); } catch (e) { /* sin notificación previa */ }
      mostrarAdvertencia();
    } else {
      if (estado === 'FAILED') {
        mostrarWifiMensaje('El kit no logró conectarse la última vez. Intenta de nuevo.');
      } else {
        mostrarWifiMensaje('El kit todavía no tiene WiFi configurado.');
      }
      mostrarPantalla('pantallaWifi');
    }
  } catch (err) {
    console.error(err);
    mostrarError('errorConectar', 'No se pudo conectar: ' + err.message);
  }
}

// ─── 2. Configurar WiFi ──────────────────────────────────────────────
document.getElementById('btnScan').addEventListener('click', escanearRedes);

async function escanearRedes() {
  const boton = document.getElementById('btnScan');
  boton.disabled = true;
  boton.textContent = 'Escaneando...';
  try {
    await chScan.writeValue(codificador.encode('SCAN'));

    // El escaneo de WiFi en el ESP32 no tarda siempre lo mismo (depende de
    // cuántas redes hay alrededor), así que sondeamos en vez de esperar un
    // tiempo fijo: mientras la característica siga diciendo "SCAN" (lo que
    // acabamos de escribir) o "SCANNING", el resultado todavía no está listo.
    let texto = '';
    for (let intento = 0; intento < 20; intento++) { // hasta ~10 s
      await esperar(500);
      texto = decodificador.decode(await chScan.readValue());
      if (texto !== 'SCAN' && texto !== 'SCANNING') break;
    }

    const redes = texto.split('\n').map(s => s.trim()).filter(Boolean);
    const select = document.getElementById('selectRedes');
    select.innerHTML = '<option value="">— elige una red —</option>' +
      redes.map(r => `<option>${escaparHtml(r)}</option>`).join('');
    select.hidden = redes.length === 0;
    if (redes.length === 0) mostrarWifiMensaje('No se encontraron redes. Escribe el SSID a mano.');
  } catch (err) {
    console.error(err);
    mostrarWifiMensaje('No se pudo escanear: ' + err.message);
  } finally {
    boton.disabled = false;
    boton.textContent = '🔄 Escanear redes';
  }
}

document.getElementById('selectRedes').addEventListener('change', (e) => {
  if (e.target.value) document.getElementById('inputSsid').value = e.target.value;
});

document.getElementById('btnEnviarCred').addEventListener('click', enviarCredenciales);

async function enviarCredenciales() {
  const ssid = document.getElementById('inputSsid').value.trim();
  const pass = document.getElementById('inputPass').value;
  if (!ssid) { mostrarWifiMensaje('Escribe el nombre de la red.'); return; }
  try {
    mostrarWifiEstado('Enviando credenciales al kit...');
    // docs/protocolo.md §2: "SSID\nPASSWORD" en un solo write.
    await chCred.writeValue(codificador.encode(ssid + '\n' + pass));
  } catch (err) {
    mostrarWifiEstado('No se pudo enviar: ' + err.message, true);
  }
}

function onWifiStat(evento) {
  const valor = decodificador.decode(evento.target.value);
  if (valor === 'CONNECTING')      mostrarWifiEstado('Conectando al WiFi...');
  else if (valor === 'CONNECTED')  mostrarWifiEstado('¡WiFi conectado!');
  else if (valor === 'FAILED')     mostrarWifiEstado('Falló la conexión. Revisa la contraseña e intenta de nuevo.', true);
}

function onIpAddr(evento) {
  const ip = decodificador.decode(evento.target.value);
  if (ip && ip !== '0.0.0.0') {
    ipActual = ip;
    mostrarAdvertencia();
  }
}

function onWifiSsid(evento) {
  ssidRedActual = decodificador.decode(evento.target.value);
  const el = document.getElementById('ssidActual');
  if (el) el.textContent = ssidRedActual || '—';
}

// ─── 3. Advertencia de red + 4. WebSocket con los datos ─────────────
function mostrarAdvertencia() {
  document.getElementById('ssidActual').textContent = ssidRedActual || '(nombre no disponible)';
  mostrarPantalla('pantallaAdvertencia');
}

document.getElementById('btnContinuar').addEventListener('click', conectarWebSocket);

function conectarWebSocket() {
  ocultarError('errorWs');
  if (!ipActual) {
    mostrarError('errorWs', 'Todavía no tengo la IP del kit. Espera un momento y vuelve a intentar.');
    return;
  }

  document.getElementById('linkData').href = `http://${ipActual}/data`;

  try {
    socket = new WebSocket(`ws://${ipActual}/ws`);
  } catch (err) {
    mostrarError('errorWs', 'No se pudo abrir el WebSocket: ' + err.message);
    return;
  }

  socket.addEventListener('open', () => mostrarPantalla('pantallaDatos'));
  socket.addEventListener('message', (ev) => renderizarJson(ev.data));
  socket.addEventListener('error', () => {
    mostrarPantalla('pantallaAdvertencia');
    mostrarError('errorWs', `No se pudo conectar. Verifica que estés en la red WiFi: ${ssidRedActual || ipActual}`);
  });
  socket.addEventListener('close', () => {
    if (document.getElementById('pantallaDatos').hidden) return; // ya se manejó por error
    mostrarPantalla('pantallaAdvertencia');
    mostrarError('errorWs', 'Se perdió la conexión con el kit.');
  });
}

function renderizarJson(texto) {
  document.getElementById('jsonCrudo').textContent = texto;
  let datos;
  try { datos = JSON.parse(texto); } catch (err) { return; }
  const sensores = Array.isArray(datos.s) ? datos.s : [];
  document.getElementById('grillaSensores').innerHTML =
    sensores.map(renderizarTarjeta).join('');
}

function renderizarTarjeta(sensor) {
  if (!sensor.c) {
    return `<div class="tarjeta vacia">
      <span class="puerto">Puerto ${sensor.p}</span>
      <span class="valor">— vacío —</span>
    </div>`;
  }
  const nombre = NOMBRES_SENSOR[sensor.id] || sensor.id;
  return `<div class="tarjeta">
    <span class="puerto">Puerto ${sensor.p}</span>
    <span class="nombre">${escaparHtml(nombre)}</span>
    <span class="valor">${sensor.v} <small>${escaparHtml(sensor.u || '')}</small></span>
  </div>`;
}

// ─── Desconexión ─────────────────────────────────────────────────────
document.getElementById('btnDesconectar').addEventListener('click', desconectarTodo);

function desconectarTodo() {
  if (socket) { socket.close(); socket = null; }
  if (dispositivo && dispositivo.gatt.connected) dispositivo.gatt.disconnect();
  reiniciarUI();
}

function alDesconectarBLE() {
  if (socket) { socket.close(); socket = null; }
  reiniciarUI();
}

function reiniciarUI() {
  actualizarBadge('Desconectado');
  document.getElementById('wifiEstado').hidden = true;
  document.getElementById('selectRedes').hidden = true;
  mostrarPantalla('pantallaConectar');
}
