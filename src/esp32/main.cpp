// Firmware de la ESP32-C6 del kit Sensei.
// Diseño completo: docs/plan_implementacion.md
//
//   1. Anuncia BLE como SENSEI-XXXX y recibe las credenciales WiFi de la app.
//   2. Se conecta al WiFi sin guardar las credenciales (al apagar se pierden).
//   3. Notifica por BLE el progreso, la IP y el nombre de la red.
//   4. Levanta un servidor HTTP + WebSocket en el puerto 80.
//   5. Recibe por UART el JSON de sensores del Mega (cada 500 ms) y lo reenvía
//      tal cual por WebSocket. No lee sensores ni modifica el JSON.
//   6. Le reporta al Mega su estado (#NAME, #WIFI, #BLE, #IP) para el LCD.
//
// Cableado UART con el Mega (docs/plan_implementacion.md §3):
//   Mega TX1 (18) --[divisor 1k/2k]--> GPIO17 (RX)
//   Mega RX1 (19) <------------------- GPIO16 (TX)
//   La consola USB sale por el USB nativo (ARDUINO_USB_CDC_ON_BOOT), porque
//   GPIO16/17 son también el UART0 del chip USB-serial de la placa.

#include <Arduino.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <ESPAsyncWebServer.h>

// ─── UART con el Mega ─────────────────────────────────────────────
static constexpr int      PIN_RX_MEGA = 17;
static constexpr int      PIN_TX_MEGA = 16;
static constexpr uint32_t BAUDIOS     = 115200;
#define UART_MEGA Serial1

// ─── UUID del perfil GATT (acordados con Android, no modificar) ───
static const char* UUID_SERVICIO   = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
static const char* UUID_WIFI_SCAN  = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
static const char* UUID_WIFI_CRED  = "beb5483e-36e1-4688-b7f5-ea07361b26a9";
static const char* UUID_WIFI_STAT  = "beb5483e-36e1-4688-b7f5-ea07361b26aa";
static const char* UUID_IP_ADDR    = "beb5483e-36e1-4688-b7f5-ea07361b26ab";
static const char* UUID_WIFI_SSID  = "243c4f09-ebd7-4026-940a-10afc3c8993e";
static const char* UUID_WIFI_STATE = "4499e77a-31ec-4153-bcde-00e90bb808c9";

// ─── Estado del WiFi ──────────────────────────────────────────────
enum class EstadoWifi { SinConfigurar, Conectando, Conectado, Fallo };
static EstadoWifi estadoWifi = EstadoWifi::SinConfigurar;

static const unsigned long TIMEOUT_WIFI_MS    = 15000;  // límite para conectar
static const unsigned long CAIDA_WIFI_MS      = 5000;   // sin WiFi tanto tiempo = se cayó
static const unsigned long REPORTE_MEGA_MS    = 3000;   // el Mega espera un reporte cada 3 s

// Texto de WIFI_STATE y de #WIFI (mismo vocabulario, docs/protocolo.md §2)
static const char* textoEstado(EstadoWifi e) {
    switch (e) {
        case EstadoWifi::Conectando: return "CONNECTING";
        case EstadoWifi::Conectado:  return "CONNECTED";
        case EstadoWifi::Fallo:      return "FAILED";
        default:                     return "UNCONFIGURED";
    }
}

// ─── Objetos BLE ──────────────────────────────────────────────────
static NimBLECharacteristic* chScan  = nullptr;
static NimBLECharacteristic* chStat  = nullptr;
static NimBLECharacteristic* chIp    = nullptr;
static NimBLECharacteristic* chSsid  = nullptr;
static NimBLECharacteristic* chState = nullptr;

static char nombreKit[16];
static volatile uint8_t clientesBle = 0;

// Datos que llegan desde los callbacks BLE (otra tarea). Se llenan primero y la
// bandera se sube al final; loop() la baja después de leerlos.
static char ssidPendiente[33];
static char passPendiente[65];
static volatile bool hayCredenciales = false;
static volatile bool hayPedidoScan   = false;
static volatile bool reportarYa      = false;   // pide un reporte inmediato al Mega

static unsigned long tInicioConexion = 0;
static unsigned long tSinWifi        = 0;
static unsigned long tUltimoReporte  = 0;

// ─── Servidor HTTP + WebSocket ────────────────────────────────────
static AsyncWebServer servidor(80);
static AsyncWebSocket ws("/ws");
static bool servidorIniciado = false;

// Último JSON recibido del Mega. Lo escribe loop() y lo leen las tareas del
// servidor, por eso va protegido. Arranca con los 4 puertos vacíos.
static portMUX_TYPE mutexJson = portMUX_INITIALIZER_UNLOCKED;
static char ultimoJson[512] =
    "{\"s\":[{\"p\":1,\"c\":false},{\"p\":2,\"c\":false},"
    "{\"p\":3,\"c\":false},{\"p\":4,\"c\":false}]}";

static void guardarJson(const char* json) {
    portENTER_CRITICAL(&mutexJson);
    strlcpy(ultimoJson, json, sizeof(ultimoJson));
    portEXIT_CRITICAL(&mutexJson);
}

static void copiarJson(char* destino, size_t n) {
    portENTER_CRITICAL(&mutexJson);
    strlcpy(destino, ultimoJson, n);
    portEXIT_CRITICAL(&mutexJson);
}

// ─── Notificaciones BLE ───────────────────────────────────────────
static void notificar(NimBLECharacteristic* c, const char* valor) {
    if (!c) return;
    c->setValue(valor);
    c->notify();
}

// ─── Callbacks BLE ────────────────────────────────────────────────
// La app escribe "SSID\nPASSWORD" en WIFI_CRED (con '|' también sirve, para
// probar a mano desde nRF Connect).
class CallbackCredenciales : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override {
        std::string v = c->getValue();
        size_t corte = v.find('\n');
        if (corte == std::string::npos) corte = v.find('|');
        if (corte == std::string::npos) return;
        strlcpy(ssidPendiente, v.substr(0, corte).c_str(), sizeof(ssidPendiente));
        strlcpy(passPendiente, v.substr(corte + 1).c_str(), sizeof(passPendiente));
        hayCredenciales = true;
    }
};

// La app escribe "SCAN" en WIFI_SCAN para pedir la lista de redes.
class CallbackScan : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c, NimBLEConnInfo&) override {
        if (c->getValue() == "SCAN") hayPedidoScan = true;
    }
};

// El anuncio se mantiene siempre activo: otro celular debe poder conectarse
// para leer la IP sin reconfigurar nada.
class CallbackServidor : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*, NimBLEConnInfo&) override {
        clientesBle = clientesBle + 1;
        NimBLEDevice::startAdvertising();
        reportarYa = true;
    }
    void onDisconnect(NimBLEServer*, NimBLEConnInfo&, int) override {
        if (clientesBle > 0) clientesBle = clientesBle - 1;
        NimBLEDevice::startAdvertising();
        reportarYa = true;
    }
};

static void iniciarBle() {
    NimBLEDevice::init(nombreKit);

    NimBLEServer* servidorBle = NimBLEDevice::createServer();
    servidorBle->setCallbacks(new CallbackServidor());
    NimBLEService* servicio = servidorBle->createService(UUID_SERVICIO);

    chScan = servicio->createCharacteristic(UUID_WIFI_SCAN, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    chScan->setCallbacks(new CallbackScan());

    NimBLECharacteristic* chCred = servicio->createCharacteristic(UUID_WIFI_CRED, NIMBLE_PROPERTY::WRITE);
    chCred->setCallbacks(new CallbackCredenciales());

    chStat  = servicio->createCharacteristic(UUID_WIFI_STAT,  NIMBLE_PROPERTY::NOTIFY);
    chIp    = servicio->createCharacteristic(UUID_IP_ADDR,    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    chSsid  = servicio->createCharacteristic(UUID_WIFI_SSID,  NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    chState = servicio->createCharacteristic(UUID_WIFI_STATE, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    chState->setValue(textoEstado(estadoWifi));

    // NimBLE 2.x arranca los servicios solo al empezar el anuncio.

    // El nombre va en el paquete principal de publicidad: algunos stacks BLE de
    // Windows no piden el scan response al buscar por nombre.
    NimBLEAdvertising* anuncio = NimBLEDevice::getAdvertising();
    anuncio->setName(nombreKit);
    NimBLEDevice::startAdvertising();

    Serial.printf("[BLE] Anunciando como %s\n", nombreKit);
}

// ─── Servidor ─────────────────────────────────────────────────────
static void iniciarServidor() {
    if (servidorIniciado) return;

    // Al conectarse un cliente recibe de inmediato el último JSON conocido.
    ws.onEvent([](AsyncWebSocket*, AsyncWebSocketClient* cliente, AwsEventType tipo,
                  void*, uint8_t*, size_t) {
        if (tipo == WS_EVT_CONNECT) {
            char json[512];
            copiarJson(json, sizeof(json));
            cliente->text(json);
        }
    });
    servidor.addHandler(&ws);

    servidor.on("/data", HTTP_GET, [](AsyncWebServerRequest* req) {
        char json[512];
        copiarJson(json, sizeof(json));
        req->send(200, "application/json", json);
    });

    servidor.begin();
    servidorIniciado = true;
    Serial.println("[HTTP] Servidor y WebSocket en el puerto 80");
}

// ─── Reporte al Mega (para el LCD) ────────────────────────────────
static void reportarAlMega() {
    UART_MEGA.print("#NAME "); UART_MEGA.println(nombreKit);
    UART_MEGA.print("#WIFI "); UART_MEGA.println(textoEstado(estadoWifi));
    UART_MEGA.print("#BLE ");  UART_MEGA.println(clientesBle);
    if (estadoWifi == EstadoWifi::Conectado) {
        UART_MEGA.print("#IP "); UART_MEGA.println(WiFi.localIP().toString());
    }
    tUltimoReporte = millis();
}

// ─── WiFi ─────────────────────────────────────────────────────────
static void cambiarEstado(EstadoWifi nuevo) {
    estadoWifi = nuevo;
    if (chState) notificar(chState, textoEstado(nuevo));
    reportarYa = true;
}

static void iniciarConexion() {
    Serial.printf("[WiFi] Conectando a \"%s\"...\n", ssidPendiente);
    notificar(chStat, "CONNECTING");
    WiFi.disconnect();
    WiFi.begin(ssidPendiente, passPendiente);
    tInicioConexion = millis();
    cambiarEstado(EstadoWifi::Conectando);
}

static void atenderWifi() {
    if (hayCredenciales) {
        hayCredenciales = false;
        iniciarConexion();
    }

    if (estadoWifi == EstadoWifi::Conectando) {
        if (WiFi.status() == WL_CONNECTED) {
            String ip = WiFi.localIP().toString();
            Serial.printf("[WiFi] Conectado. IP %s\n", ip.c_str());
            notificar(chIp, ip.c_str());
            notificar(chSsid, WiFi.SSID().c_str());
            notificar(chStat, "CONNECTED");
            iniciarServidor();
            cambiarEstado(EstadoWifi::Conectado);
            tSinWifi = 0;
        } else if (millis() - tInicioConexion > TIMEOUT_WIFI_MS) {
            Serial.printf("[WiFi] Fallo la conexion (status %d)\n", WiFi.status());
            notificar(chStat, "FAILED");
            cambiarEstado(EstadoWifi::Fallo);
        }
    } else if (estadoWifi == EstadoWifi::Conectado) {
        // Si el router se cae, se vuelve a intentar y el LCD regresa al paso 2.
        if (WiFi.status() == WL_CONNECTED) {
            tSinWifi = 0;
        } else if (tSinWifi == 0) {
            tSinWifi = millis();
        } else if (millis() - tSinWifi > CAIDA_WIFI_MS) {
            Serial.println("[WiFi] Se perdio la conexion, reintentando...");
            WiFi.reconnect();
            tInicioConexion = millis();
            tSinWifi = 0;
            cambiarEstado(EstadoWifi::Conectando);
        }
    }
}

// La app pidió la lista de redes. Se hace fuera del callback porque tarda ~2 s.
static void atenderScan() {
    if (!hayPedidoScan) return;
    hayPedidoScan = false;

    // Si la app lee antes de que termine, debe ver "SCANNING" y no su propio "SCAN".
    chScan->setValue("SCANNING");
    Serial.println("[WiFi] Escaneo solicitado por la app");

    int n = WiFi.scanNetworks();
    String lista;
    for (int i = 0; i < n; i++) {
        if (i) lista += '\n';
        lista += WiFi.SSID(i);
    }
    chScan->setValue(lista.c_str());   // vacío si no encontró nada
    WiFi.scanDelete();
}

// ─── Puente UART → WebSocket ──────────────────────────────────────
static void atenderUart() {
    static char    linea[512];
    static size_t  idx = 0;

    while (UART_MEGA.available()) {
        char c = (char)UART_MEGA.read();
        if (c == '\n' || c == '\r') {
            if (idx == 0) continue;
            linea[idx] = '\0';
            idx = 0;
            if (linea[0] != '{') continue;   // solo se reenvía lo que parece JSON
            guardarJson(linea);
            if (estadoWifi == EstadoWifi::Conectado) ws.textAll(linea);
        } else if (idx < sizeof(linea) - 1) {
            linea[idx++] = c;
        } else {
            idx = 0;   // línea demasiado larga: descartar
        }
    }
}

void setup() {
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 2000) delay(10);   // USB nativo: esperar al monitor

    UART_MEGA.begin(BAUDIOS, SERIAL_8N1, PIN_RX_MEGA, PIN_TX_MEGA);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);   // menos latencia en el WebSocket

    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(nombreKit, sizeof(nombreKit), "SENSEI-%02X%02X", mac[4], mac[5]);

    iniciarBle();
    reportarAlMega();   // el LCD ya puede mostrar el nombre desde el arranque
    Serial.println("[SYS] Listo. Esperando credenciales WiFi por BLE...");
}

void loop() {
    atenderScan();
    atenderWifi();
    atenderUart();

    if (estadoWifi == EstadoWifi::Conectado) ws.cleanupClients();

    // Reporte al Mega: al cambiar algo y, como respaldo, cada 3 s.
    if (reportarYa || millis() - tUltimoReporte >= REPORTE_MEGA_MS) {
        reportarYa = false;
        reportarAlMega();
    }
}
