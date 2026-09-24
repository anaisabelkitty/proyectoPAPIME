// Panel de pruebas de Sensei: hace en el navegador lo mismo que la app Android.
// Flujo (docs/protocolo.md §3 y docs/plan_implementacion.md §5):
//   1. Bluetooth: buscar SENSEI-XXXX y leer WIFI_STATE.
//   2. WiFi: enviar "SSID\nPASSWORD" y esperar CONNECTING -> CONNECTED + IP.
//   3. Datos: abrir ws://<ip>/ws y mostrar el JSON de los 4 puertos.
// Web Bluetooth solo funciona en Chrome y Edge, desde localhost o https.

const UUID = {
  servicio: '4fafc201-1fb5-459e-8fcc-c5c9c331914b',
  scan:     'beb5483e-36e1-4688-b7f5-ea07361b26a8',
  cred:     'beb5483e-36e1-4688-b7f5-ea07361b26a9',
  stat:     'beb5483e-36e1-4688-b7f5-ea07361b26aa',
  ip:       'beb5483e-36e1-4688-b7f5-ea07361b26ab',
  ssid:     '243c4f09-ebd7-4026-940a-10afc3c8993e',
  state:    '4499e77a-31ec-4153-bcde-00e90bb808c9',
};

// Nombre que se muestra para cada "id" del JSON.
const NOMBRES = {
  ph: 'pH', humedad: 'Humedad', temperatura: 'Temperatura', distancia: 'Distancia',
  voltaje: 'Voltaje', corriente: 'Corriente', co2: 'CO2', pulso: 'Pulso',
  luz: 'Luz', hall: 'Efecto Hall',
};

const $ = (id) => document.getElementById(id);
const decodificar = (dv) => new TextDecoder().decode(dv);
const esperar = (ms) => new Promise((r) => setTimeout(r, ms));

const kit = { dispositivo: null, car: {}, ip: '', ssid: '' };
let socket = null;
let simulador = null;
let marcasMensajes = [];   // para calcular cada cuánto llegan los datos

// ─── Interfaz ─────────────────────────────────────────────────────
function log(texto) {
  const hora = new Date().toLocaleTimeString();
  $('registro').textContent += `[${hora}] ${texto}\n`;
  $('registro').scrollTop = $('registro').scrollHeight;
}

// Muestra una sola sección y marca el paso del indicador.
function mostrar(seccion, paso) {
  for (const id of ['paso-1', 'paso-2', 'paso-aviso', 'paso-3']) {
    $(id).classList.toggle('oculto', id !== seccion);
  }
  for (let i = 1; i <= 3; i++) {
    $(`marca-${i}`).className = i < paso ? 'hecho' : i === paso ? 'activo' : '';
  }
}

function estadoWifi(texto) {
  $('estado-kit').textContent = texto;
  const mensajes = {
    CONNECTING: 'Conectando a la red...',
    CONNECTED: 'Conectado.',
    FAILED: 'No se pudo conectar. Revisa la red y la contraseña.',
    UNCONFIGURED: 'Sin configurar.',
  };
  $('msg-wifi').textContent = mensajes[texto] || texto;
}

// ─── Paso 1: Bluetooth ────────────────────────────────────────────
async function suscribir(caracteristica, manejador) {
  await caracteristica.startNotifications();
  caracteristica.addEventListener('characteristicvaluechanged', (e) => {
    manejador(decodificar(e.target.value));
  });
}

async function buscarKit() {
  if (!navigator.bluetooth) {
    alert('Este navegador no soporta Web Bluetooth. Usa Chrome o Edge.');
    return;
  }
  try {
    log('Buscando dispositivos SENSEI-...');
    const dispositivo = await navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: 'SENSEI-' }],
      optionalServices: [UUID.servicio],
    });
    kit.dispositivo = dispositivo;
    dispositivo.addEventListener('gattserverdisconnected', () => log('Bluetooth desconectado.'));

    log(`Conectando a ${dispositivo.name}...`);
    const servidor = await dispositivo.gatt.connect();
    const servicio = await servidor.getPrimaryService(UUID.servicio);
    for (const nombre of ['scan', 'cred', 'stat', 'ip', 'ssid', 'state']) {
      kit.car[nombre] = await servicio.getCharacteristic(UUID[nombre]);
    }

    await suscribir(kit.car.stat, alRecibirStat);
    await suscribir(kit.car.ip, (t) => { kit.ip = t; log(`IP recibida: ${t}`); });
    await suscribir(kit.car.ssid, (t) => { kit.ssid = t; });
    await suscribir(kit.car.state, (t) => log(`WIFI_STATE: ${t}`));

    $('nombre-kit').textContent = dispositivo.name;
    const estado = decodificar(await kit.car.state.readValue());
    log(`Conectado por Bluetooth. WIFI_STATE = ${estado}`);

    if (estado === 'CONNECTED') {
      await leerIpYSsid();
      mostrarAviso();
    } else {
      estadoWifi(estado);
      mostrar('paso-2', 2);
    }
  } catch (e) {
    log(`Bluetooth: ${e.message}`);
  }
}

async function leerIpYSsid() {
  kit.ip = decodificar(await kit.car.ip.readValue());
  kit.ssid = decodificar(await kit.car.ssid.readValue());
}

// ─── Paso 2: WiFi ─────────────────────────────────────────────────
async function buscarRedes() {
  try {
    $('btn-redes').disabled = true;
    $('msg-wifi').textContent = 'Buscando redes...';
    await kit.car.scan.writeValue(new TextEncoder().encode('SCAN'));

    // El kit responde "SCANNING" hasta que termina el escaneo.
    let texto = 'SCANNING';
    for (let i = 0; i < 10 && (texto === 'SCANNING' || texto === 'SCAN'); i++) {
      await esperar(1000);
      texto = decodificar(await kit.car.scan.readValue());
    }
    const redes = texto.split('\n').filter((r) => r && r !== 'SCANNING');
    $('redes').innerHTML = redes.map((r) => `<option value="${r.replace(/"/g, '&quot;')}">`).join('');
    $('msg-wifi').textContent = redes.length ? `${redes.length} redes encontradas.` : 'No se encontraron redes.';
    log(`Redes: ${redes.join(', ') || '(ninguna)'}`);
  } catch (e) {
    $('msg-wifi').textContent = `Error al buscar redes: ${e.message}`;
  } finally {
    $('btn-redes').disabled = false;
  }
}

async function enviarCredenciales() {
  const ssid = $('ssid').value.trim();
  if (!ssid) { $('msg-wifi').textContent = 'Escribe el nombre de la red.'; return; }
  try {
    $('btn-enviar').disabled = true;
    estadoWifi('CONNECTING');
    log(`Enviando credenciales de "${ssid}"...`);
    await kit.car.cred.writeValue(new TextEncoder().encode(`${ssid}\n${$('clave').value}`));
  } catch (e) {
    $('msg-wifi').textContent = `Error al enviar: ${e.message}`;
    $('btn-enviar').disabled = false;
  }
}

async function alRecibirStat(texto) {
  log(`WIFI_STAT: ${texto}`);
  estadoWifi(texto);
  if (texto === 'FAILED') $('btn-enviar').disabled = false;
  if (texto === 'CONNECTED') {
    $('btn-enviar').disabled = false;
    await esperar(300);   // deja llegar IP_ADDR y WIFI_SSID
    try { await leerIpYSsid(); } catch (e) { /* ya llegaron por notificación */ }
    mostrarAviso();
  }
}

// ─── Aviso y paso 3: WebSocket ────────────────────────────────────
function mostrarAviso(conError = false) {
  $('aviso-ssid').textContent = kit.ssid || '(desconocida)';
  $('aviso-ip').textContent = kit.ip || '-';
  $('aviso-error').classList.toggle('oculto', !conError);
  mostrar('paso-aviso', 2);
}

function conectarWebSocket(ip) {
  cerrarConexiones();
  $('estado-ws').textContent = 'conectando...';
  $('estado-ws').classList.remove('ok');
  log(`Abriendo ws://${ip}/ws ...`);

  let abierto = false;
  socket = new WebSocket(`ws://${ip}/ws`);
  socket.onopen = () => {
    abierto = true;
    marcasMensajes = [];
    $('estado-ws').textContent = 'conectado';
    $('estado-ws').classList.add('ok');
    mostrar('paso-3', 3);
    log('WebSocket abierto.');
  };
  socket.onmessage = (e) => recibirJson(e.data);
  socket.onclose = () => {
    $('estado-ws').textContent = 'sin conexión';
    $('estado-ws').classList.remove('ok');
    log('WebSocket cerrado.');
    if (!abierto) mostrarAviso(true);   // docs/protocolo.md §3: si falla, vuelve la advertencia
  };
}

function cerrarConexiones() {
  if (socket) { socket.onclose = null; socket.close(); socket = null; }
  if (simulador) { clearInterval(simulador); simulador = null; }
}

// ─── Datos ────────────────────────────────────────────────────────
function recibirJson(texto) {
  $('json-crudo').textContent = texto;

  // Cada cuánto llegan los datos (deben ser ~500 ms)
  const ahora = performance.now();
  marcasMensajes.push(ahora);
  if (marcasMensajes.length > 10) marcasMensajes.shift();
  if (marcasMensajes.length > 1) {
    const cada = (ahora - marcasMensajes[0]) / (marcasMensajes.length - 1);
    $('info-ws').textContent = `un dato cada ${Math.round(cada)} ms`;
  }

  let datos;
  try { datos = JSON.parse(texto); } catch (e) { log(`JSON inválido: ${e.message}`); return; }
  if (!Array.isArray(datos.s)) return;

  $('rejilla').innerHTML = datos.s.map(tarjetaSensor).join('');
}

function tarjetaSensor(s) {
  if (!s.c) {
    return `<div class="sensor vacio"><div class="puerto">Puerto ${s.p}</div>
      <div class="valor">Sin sensor</div></div>`;
  }
  const nombre = NOMBRES[s.id] || s.id;
  return `<div class="sensor"><div class="puerto">Puerto ${s.p}</div>
    <div class="nombre">${nombre}</div>
    <div><span class="valor">${s.v}</span><span class="unidad">${s.u}</span></div></div>`;
}

// Genera datos falsos con el mismo formato, para probar la página sin el kit.
function simularDatos() {
  cerrarConexiones();
  marcasMensajes = [];
  mostrar('paso-3', 3);
  $('estado-ws').textContent = 'simulación';
  $('estado-ws').classList.add('ok');
  simulador = setInterval(() => {
    const json = { s: [
      { p: 1, c: true, id: 'ph', v: +(6.5 + Math.random() * 2).toFixed(2), u: 'pH' },
      { p: 2, c: true, id: 'humedad', v: +(30 + Math.random() * 40).toFixed(1), u: '%' },
      { p: 3, c: false },
      { p: 4, c: true, id: 'temperatura', v: +(22 + Math.random() * 5).toFixed(2), u: '°C' },
    ] };
    recibirJson(JSON.stringify(json));
  }, 500);
  log('Simulación de datos iniciada.');
}

function salir() {
  cerrarConexiones();
  if (kit.dispositivo && kit.dispositivo.gatt.connected) kit.dispositivo.gatt.disconnect();
  kit.dispositivo = null;
  kit.car = {};
  $('rejilla').innerHTML = '';
  $('btn-enviar').disabled = false;
  mostrar('paso-1', 1);
}

// ─── Eventos ──────────────────────────────────────────────────────
$('btn-buscar').addEventListener('click', buscarKit);
$('btn-redes').addEventListener('click', buscarRedes);
$('btn-enviar').addEventListener('click', enviarCredenciales);
$('btn-continuar').addEventListener('click', () => conectarWebSocket(kit.ip));
$('btn-ip-manual').addEventListener('click', () => {
  const ip = $('ip-manual').value.trim();
  if (ip) { kit.ip = ip; conectarWebSocket(ip); }
});
$('btn-simular').addEventListener('click', simularDatos);
$('btn-salir').addEventListener('click', salir);

mostrar('paso-1', 1);
log(navigator.bluetooth ? 'Listo.' : 'Este navegador no soporta Web Bluetooth (usa Chrome o Edge).');
