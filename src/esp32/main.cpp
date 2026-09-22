// esp32/main.cpp — Firmware del ESP32-C6 del kit Sensei
//
// Responsabilidad del ESP32 (ver docs/proyecto.md y docs/protocolo.md):
//   1. Anunciarse por BLE como SENSEI-XXXX y recibir las credenciales WiFi.
//   2. Conectarse a la red WiFi del aula — sin guardar las credenciales.
//   3. Notificar por BLE el progreso de la conexión, la IP y el SSID.
//   4. Levantar un servidor HTTP + WebSocket en el puerto 80.
//   5. Recibir el JSON de sensores del Arduino Mega por UART (cada 500 ms)
//      y reenviarlo TAL CUAL por WebSocket a todas las apps conectadas.
//
// El ESP32 nunca lee sensores ni modifica el JSON: solo hace de puente.
//
// ── Cableado UART con el Mega (ver docs/conexiones.md) ──────────────
//   Mega TX1 (pin 18) ──[divisor 5V→3.3V]──> ESP32-C6 GPIO18 (RX)
//   Mega RX1 (pin 19) ───────────────────────> ESP32-C6 GPIO19 (TX)
//   115200 baudios, 8N1. Se usa Serial1 para dejar libre la consola USB.
//
// ── Librerías (platformio.ini, entorno esp32-c6-devkitc-1) ──────────
//   h2zero/NimBLE-Arduino
//   ESP32Async/AsyncTCP
//   ESP32Async/ESPAsyncWebServer

#include <Arduino.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <ESPAsyncWebServer.h>

// ─── UART con el Mega ──────────────────────────────────────────────
static const int  UART_RX_PIN  = 18;   // ESP32 RX  ← Mega TX1 (pin 18)
static const int  UART_TX_PIN  = 19;   // ESP32 TX  → Mega RX1 (pin 19)
static const long UART_BAUDIOS = 115200;
#define UART_MEGA Serial1

// ─── UUIDs del perfil GATT (acordados con Android — NO modificar) ──
// Fuentes: docs/protocolo.md §2 y docs/respuestasAndroid.md §2
static const char* SERVICE_UUID    = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
static const char* UUID_WIFI_SCAN  = "beb5483e-36e1-4688-b7f5-ea07361b26a8"; // App  → ESP32
static const char* UUID_WIFI_CRED  = "beb5483e-36e1-4688-b7f5-ea07361b26a9"; // App  → ESP32
static const char* UUID_WIFI_STAT  = "beb5483e-36e1-4688-b7f5-ea07361b26aa"; // ESP32 → App
static const char* UUID_IP_ADDR    = "beb5483e-36e1-4688-b7f5-ea07361b26ab"; // ESP32 → App
static const char* UUID_WIFI_SSID  = "243c4f09-ebd7-4026-940a-10afc3c8993e"; // ESP32 → App
static const char* UUID_WIFI_STATE = "4499e77a-31ec-4153-bcde-00e90bb808c9"; // ESP32 → App

// Progreso de conexión que la app espera en WIFI_STAT (respetar mayúsculas)
static const char* STAT_CONNECTING = "CONNECTING";
static const char* STAT_CONNECTED  = "CONNECTED";
static const char* STAT_FAILED     = "FAILED";

// Estado que la app lee en WIFI_STATE nada más conectarse por BLE
static const char* STATE_UNCONFIGURED = "UNCONFIGURED";
static const char* STATE_CONNECTED    = "CONNECTED";
static const char* STATE_FAILED       = "FAILED";

// ─── Objetos BLE ──────────────────────────────────────────────────
static NimBLECharacteristic* chWifiScan  = nullptr;
static NimBLECharacteristic* chWifiStat  = nullptr;
static NimBLECharacteristic* chIpAddr    = nullptr;
static NimBLECharacteristic* chWifiSsid  = nullptr;
static NimBLECharacteristic* chWifiState = nullptr;

// ─── Servidor HTTP + WebSocket ────────────────────────────────────
static AsyncWebServer server(80);
static AsyncWebSocket  ws("/ws");
static bool servidorIniciado = false;

// ─── Último JSON recibido del Mega (buffer compartido y protegido) ─
// Se escribe desde loop() y se lee desde las tareas async del servidor.
static portMUX_TYPE muxJson = portMUX_INITIALIZER_UNLOCKED;
static char ultimoJson[512] =
    "{\"s\":[{\"p\":1,\"c\":false},{\"p\":2,\"c\":false},"
    "{\"p\":3,\"c\":false},{\"p\":4,\"c\":false}]}";

static void guardarJson(const char* linea) {
    taskENTER_CRITICAL(&muxJson);
    strlcpy(ultimoJson, linea, sizeof(ultimoJson));
    taskEXIT_CRITICAL(&muxJson);
}

static void copiarJson(char* destino, size_t n) {
    taskENTER_CRITICAL(&muxJson);
    strlcpy(destino, ultimoJson, n);
    taskEXIT_CRITICAL(&muxJson);
}

// ─── Máquina de estados WiFi (no bloqueante) ──────────────────────
enum EstadoWifi { WIFI_ESPERA, WIFI_CONECTANDO, WIFI_LISTO, WIFI_ERROR };
static EstadoWifi estadoWifi = WIFI_ESPERA;

static volatile bool hayCredenciales = false;
static volatile bool haySolicitudScan = false;
static String ssidPendiente, passPendiente;
static unsigned long tInicioConexion = 0;
static const unsigned long TIMEOUT_WIFI_MS = 15000;

static String nombreKitGlobal;   // calculado una vez en setup()

// ─── Reporte de estado al Mega (para el LCD) ──────────────────────
// Líneas con prefijo '#' por el mismo UART, junto al JSON de sensores
// que viaja en sentido contrario. Ver nucleo/enlace_esp32.h del Mega.
static const unsigned long REPORTE_MEGA_MS = 3000;
static unsigned long tUltimoReporte = 0;
static uint8_t clientesBleConectados = 0;   // cuántos celulares tienen el BLE conectado

static const char* wifiEstadoTexto() {
    switch (estadoWifi) {
        case WIFI_CONECTANDO: return "CONNECTING";
        case WIFI_LISTO:      return "CONNECTED";
        case WIFI_ERROR:      return "FAILED";
        default:              return "UNCONFIGURED";
    }
}

static void reportarAlMega() {
    UART_MEGA.print("#NAME "); UART_MEGA.println(nombreKitGlobal);
    UART_MEGA.print("#WIFI "); UART_MEGA.println(wifiEstadoTexto());
    UART_MEGA.print("#BLE ");  UART_MEGA.println(clientesBleConectados);
    if (estadoWifi == WIFI_LISTO) {
        UART_MEGA.print("#IP "); UART_MEGA.println(WiFi.localIP().toString());
    }
    // Espejo por USB: para comprobar que SÍ se está mandando algo por el
    // UART hacia el Mega, aunque el Mega no lo esté recibiendo (cableado).
    Serial.printf("[UART->Mega] NAME=%s WIFI=%s BLE=%u\n",
                  nombreKitGlobal.c_str(), wifiEstadoTexto(), clientesBleConectados);
    tUltimoReporte = millis();
}

// ─── Utilidades ──────────────────────────────────────────────────
static void notificar(NimBLECharacteristic* c, const String& v) {
    if (!c) return;
    c->setValue(v.c_str());
    c->notify();
}

static String nombreKit() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char n[16];
    snprintf(n, sizeof(n), "SENSEI-%02X%02X", mac[4], mac[5]);
    return String(n);
}

// ─── Callbacks BLE ───────────────────────────────────────────────
// La app escribe "SSID\nPASSWORD" en WIFI_CRED (un solo write).
class CredencialesCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override {
        String v = String(c->getValue().c_str());
        int corte = v.indexOf('\n');
        if (corte < 0) corte = v.indexOf('|');   // separador alterno para nRF Connect
        if (corte < 0) return;
        ssidPendiente   = v.substring(0, corte);
        passPendiente   = v.substring(corte + 1);
        hayCredenciales = true;
    }
};

// La app escribe "SCAN" en WIFI_SCAN para pedir la lista de redes.
class EscaneoCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override {
        if (String(c->getValue().c_str()) == "SCAN") haySolicitudScan = true;
    }
};

// Mantener el anuncio BLE activo siempre: cualquier celular adicional
// debe poder conectarse para leer la IP sin reconfigurar nada.
class ServidorCallback : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*, NimBLEConnInfo&) override {
        clientesBleConectados++;
        NimBLEDevice::startAdvertising();
        reportarAlMega();
    }
    void onDisconnect(NimBLEServer*, NimBLEConnInfo&, int) override {
        if (clientesBleConectados > 0) clientesBleConectados--;
        NimBLEDevice::startAdvertising();
        reportarAlMega();
    }
};

// ─── Servidor HTTP + WebSocket ───────────────────────────────────
static void onEventoWs(AsyncWebSocket*, AsyncWebSocketClient* client,
                       AwsEventType tipo, void*, uint8_t*, size_t) {
    if (tipo == WS_EVT_CONNECT) {
        char buf[512];
        copiarJson(buf, sizeof(buf));
        client->text(buf);                 // último estado conocido al conectar
    }
}

static void iniciarServidor() {
    if (servidorIniciado) return;

    ws.onEvent(onEventoWs);
    server.addHandler(&ws);

    // GET /data → último JSON recibido del Mega (sin autenticación, red de aula)
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* req) {
        char buf[512];
        copiarJson(buf, sizeof(buf));
        req->send(200, "application/json", buf);
    });

    // GET / → página mínima de diagnóstico
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "text/plain",
                  "Sensei ESP32 en linea.\n"
                  "Datos en tiempo real: ws://" + WiFi.localIP().toString() + "/ws\n"
                  "Ultima lectura:       GET /data\n");
    });

    server.begin();
    servidorIniciado = true;
    Serial.println("[HTTP] Servidor + WebSocket escuchando en el puerto 80");
}

// ─── Arranque BLE ────────────────────────────────────────────────
static void iniciarBLE(const String& nombre) {
    NimBLEDevice::init(nombre.c_str());

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServidorCallback());

    NimBLEService* svc = pServer->createService(SERVICE_UUID);

    chWifiScan = svc->createCharacteristic(
        UUID_WIFI_SCAN, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    chWifiScan->setCallbacks(new EscaneoCallback());

    NimBLECharacteristic* chCred = svc->createCharacteristic(
        UUID_WIFI_CRED, NIMBLE_PROPERTY::WRITE);
    chCred->setCallbacks(new CredencialesCallback());

    chWifiStat = svc->createCharacteristic(
        UUID_WIFI_STAT, NIMBLE_PROPERTY::NOTIFY);

    chIpAddr = svc->createCharacteristic(
        UUID_IP_ADDR, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    chWifiSsid = svc->createCharacteristic(
        UUID_WIFI_SSID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);

    chWifiState = svc->createCharacteristic(
        UUID_WIFI_STATE, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    chWifiState->setValue(STATE_UNCONFIGURED);

    svc->start();

    // El nombre completo va en el paquete PRINCIPAL de publicidad (no en el
    // scan response): algunos stacks BLE de Windows no piden/parsean el
    // scan response al escanear por nombre, y Web Bluetooth (namePrefix)
    // no necesitaba el UUID del servicio anunciado para encontrarlo — el
    // servicio se descubre igual ya conectados, vía optionalServices.
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->setName(nombre.c_str());
    NimBLEDevice::startAdvertising();

    Serial.printf("[BLE] Anunciando como %s\n", nombre.c_str());
}

// ─── Escaneo de redes WiFi (fuera de los callbacks: bloquea ~2 s) ─
static void atenderSolicitudScan() {
    if (!haySolicitudScan) return;
    haySolicitudScan = false;

    // Limpiar el "SCAN" que quedó del write: si la app lee antes de que
    // termine el escaneo, debe ver "SCANNING" (y seguir esperando), nunca
    // el "SCAN" que ella misma mandó.
    if (chWifiScan) chWifiScan->setValue("SCANNING");

    Serial.println("[WiFi] Escaneo solicitado por la app:");
    int n = WiFi.scanNetworks();
    String lista;
    for (int i = 0; i < n; i++) {
        if (i) lista += '\n';
        lista += WiFi.SSID(i);
        Serial.printf("  \"%s\" (%d dBm)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
    if (chWifiScan) chWifiScan->setValue(lista.c_str());  // "" si no encontró nada
    WiFi.scanDelete();
}

// ─── Lectura de líneas JSON del Mega y reenvío por WebSocket ──────
static void procesarUART() {
    static char   linea[512];
    static size_t idx = 0;

    while (UART_MEGA.available()) {
        char c = (char)UART_MEGA.read();
        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;               // línea vacía
            linea[idx] = '\0';
            idx = 0;
            if (linea[0] == '{') {                // se ve como JSON válido
                guardarJson(linea);
                ws.textAll(linea);               // reenvío TAL CUAL, sin tocar
            }
        } else if (idx < sizeof(linea) - 1) {
            linea[idx++] = c;
        } else {
            idx = 0;                              // línea demasiado larga: descartar
        }
    }
}

// ─── setup / loop ────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    UART_MEGA.begin(UART_BAUDIOS, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);                         // menos latencia en el WebSocket

    nombreKitGlobal = nombreKit();
    iniciarBLE(nombreKitGlobal);
    reportarAlMega();   // el LCD ya puede mostrar el nombre desde el arranque
    Serial.println("[SYS] Listo. Esperando credenciales WiFi por BLE...");
}

void loop() {
    // 1. Escaneo de redes pedido por la app
    atenderSolicitudScan();

    // 2. ¿Llegaron credenciales nuevas por BLE?
    if (hayCredenciales) {
        hayCredenciales = false;
        Serial.printf("[WiFi] Conectando a \"%s\"...\n", ssidPendiente.c_str());
        notificar(chWifiStat, STAT_CONNECTING);
        WiFi.disconnect();
        WiFi.begin(ssidPendiente.c_str(), passPendiente.c_str());
        estadoWifi      = WIFI_CONECTANDO;
        tInicioConexion = millis();
        reportarAlMega();
    }

    // 3. Avance de la conexión WiFi (sin bloquear el loop ni el BLE)
    if (estadoWifi == WIFI_CONECTANDO) {
        if (WiFi.status() == WL_CONNECTED) {
            estadoWifi = WIFI_LISTO;
            String ip = WiFi.localIP().toString();
            Serial.printf("[WiFi] Conectado. IP %s\n", ip.c_str());
            notificar(chWifiStat,  STAT_CONNECTED);
            notificar(chIpAddr,    ip);
            notificar(chWifiSsid,  WiFi.SSID());
            notificar(chWifiState, STATE_CONNECTED);
            iniciarServidor();
            reportarAlMega();
        } else if (millis() - tInicioConexion > TIMEOUT_WIFI_MS) {
            estadoWifi = WIFI_ERROR;
            Serial.printf("[WiFi] Falló la conexión (status %d)\n", WiFi.status());
            notificar(chWifiStat,  STAT_FAILED);
            notificar(chWifiState, STATE_FAILED);
            reportarAlMega();
        }
    }

    // 4. Puente UART → WebSocket (solo con WiFi arriba)
    if (estadoWifi == WIFI_LISTO) {
        procesarUART();
        ws.cleanupClients();
    } else {
        while (UART_MEGA.available()) UART_MEGA.read();   // no acumular basura
    }

    // 5. Recordatorio periódico al Mega (por si se reinició después del ESP32)
    if (millis() - tUltimoReporte >= REPORTE_MEGA_MS) {
        reportarAlMega();
    }
}
