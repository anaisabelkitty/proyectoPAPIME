// Prueba de servidor completo para el desarrollador Android.
// Cumple el contrato de docs/respuestasAndroid.md:
//  - BLE anunciado como SENSEI-XXXX (últimos 4 de la MAC)
//  - Características WIFI_SCAN / WIFI_CRED / WIFI_STAT / IP_ADDR
//  - Servidor HTTP GET /data + WebSocket ws://<ip>/ws en puerto 80
//  - Datos de sensores simulados (fake) cada 2 s, mismo formato JSON
//
// lib_deps necesarios en platformio.ini (entorno esp32-c6-devkitc-1):
//   ESP32Async/AsyncTCP @ ^3.2.0
//   ESP32Async/ESPAsyncWebServer @ ^3.0.0

#include <Arduino.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESPAsyncWebServer.h>

// ---- UUIDs acordados con Android (no modificar) ----
#define SERVICE_UUID      "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define UUID_WIFI_SCAN    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define UUID_WIFI_CRED    "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define UUID_WIFI_STAT    "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define UUID_IP_ADDR      "beb5483e-36e1-4688-b7f5-ea07361b26ab"

BLECharacteristic *pWifiStat;
BLECharacteristic *pIpAddr;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

volatile bool credencialesRecibidas = false;
String ssidPendiente, passPendiente;
bool wifiConectado = false;

// ---- JSON de sensores simulado (fake), mismo formato que el firmware real ----
String construirJsonFake() {
  float voltaje = 3.0 + (random(0, 40) / 100.0);
  float temperatura = 22.0 + (random(0, 500) / 100.0);
  float corriente = 0.5 + (random(0, 60) / 100.0);

  String json = "{\"s\":[";
  json += "{\"p\":1,\"c\":true,\"id\":\"voltaje\",\"v\":" + String(voltaje, 2) + ",\"u\":\"V\"},";
  json += "{\"p\":2,\"c\":true,\"id\":\"temperatura\",\"v\":" + String(temperatura, 1) + ",\"u\":\"°C\"},";
  json += "{\"p\":3,\"c\":false},";
  json += "{\"p\":4,\"c\":true,\"id\":\"corriente\",\"v\":" + String(corriente, 2) + ",\"u\":\"A\"}";
  json += "],\"int\":2000}";
  return json;
}

// ---- Callback: la app escribe "SSID\nPASSWORD" en WIFI_CRED ----
class CredencialesCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String valor = String(c->getValue().c_str());
    int corte = valor.indexOf('\n');
    if (corte < 0) corte = valor.indexOf('|'); // separador alterno para pruebas manuales con nRF Connect
    if (corte < 0) return;
    ssidPendiente = valor.substring(0, corte);
    passPendiente = valor.substring(corte + 1);
    credencialesRecibidas = true;
  }
};

// ---- Callback: la app escribe "SCAN" en WIFI_SCAN ----
class EscaneoCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String valor = String(c->getValue().c_str());
    if (valor != "SCAN") return;
    Serial.println("Escaneo solicitado por la app:");
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      Serial.printf("  \"%s\" (%d dBm)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
  }
};

void notificar(BLECharacteristic *c, const String &valor) {
  c->setValue(valor.c_str());
  c->notify();
}

void iniciarServidorHTTP() {
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", construirJsonFake());
  });

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("Cliente WS conectado: #%u\n", client->id());
      client->text(construirJsonFake());
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("Cliente WS desconectado: #%u\n", client->id());
    }
  });
  server.addHandler(&ws);

  server.begin();
  Serial.println("Servidor HTTP + WebSocket iniciado en puerto 80.");
}

void iniciarBLE() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char nombre[20];
  snprintf(nombre, sizeof(nombre), "SENSEI-%02X%02X", mac[4], mac[5]);
  Serial.printf("Nombre BLE: %s\n", nombre);

  BLEDevice::init(nombre);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pWifiScan = pService->createCharacteristic(
      UUID_WIFI_SCAN, BLECharacteristic::PROPERTY_WRITE);
  pWifiScan->setCallbacks(new EscaneoCallback());

  BLECharacteristic *pWifiCred = pService->createCharacteristic(
      UUID_WIFI_CRED, BLECharacteristic::PROPERTY_WRITE);
  pWifiCred->setCallbacks(new CredencialesCallback());

  pWifiStat = pService->createCharacteristic(
      UUID_WIFI_STAT, BLECharacteristic::PROPERTY_NOTIFY);
  pWifiStat->addDescriptor(new BLE2902());

  pIpAddr = pService->createCharacteristic(
      UUID_IP_ADDR, BLECharacteristic::PROPERTY_NOTIFY);
  pIpAddr->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE anunciándose, esperando credenciales WiFi...");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  randomSeed(analogRead(0));

  WiFi.mode(WIFI_MODE_APSTA); // STA para conectar, BLE ya usa su propio radio
  iniciarBLE();
}

void loop() {
  if (credencialesRecibidas) {
    credencialesRecibidas = false;
    Serial.printf("Conectando a \"%s\"...\n", ssidPendiente.c_str());
    notificar(pWifiStat, "CONNECTING");

    WiFi.begin(ssidPendiente.c_str(), passPendiente.c_str());
    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
      delay(300);
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConectado = true;
      Serial.print("Conectado. IP: ");
      Serial.println(WiFi.localIP());
      notificar(pWifiStat, "CONNECTED");
      notificar(pIpAddr, WiFi.localIP().toString());
      iniciarServidorHTTP();
    } else {
      Serial.printf("Falló la conexión WiFi. Código de estado: %d\n", WiFi.status());
      notificar(pWifiStat, "FAILED");
    }
  }

  static unsigned long ultimoEnvio = 0;
  if (wifiConectado && millis() - ultimoEnvio > 2000) {
    ultimoEnvio = millis();
    ws.textAll(construirJsonFake());
    ws.cleanupClients();
  }
}
