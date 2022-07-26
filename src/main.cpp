#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <AsyncElegantOTA.h>
#include <AsyncMqttClient.h>
#include <SPI.h>
#include <LoRa.h>
#include "definitions.h"

/**
 * Program pre LoRa komunikáciu pomocou ESP32 - prijímač
 *
 * Posledná zmena(last change): 26.07.2022
 * @author Ing. Peter VOJTECH ml. <petak23@gmail.com>
 * @copyright  Copyright (c) 2022 - 2022 Ing. Peter VOJTECH ml.
 * @license
 * @link       http://petak23.echo-msz.eu
 * @version 1.0.1
 */

// Inicializácia espClient.
WiFiClient espClient;
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

String LoRaData = ""; // Dáta prijaté cez LoRa prenos
int rssi = -120;      // Sila signálu

unsigned long lastMqttReconectTime = 0; // Uloží posledný čas pokusu o pripojenie k MQTT
int mqtt_state = 0;                     // Stav MQTT pripojenia podľa https://pubsubclient.knolleary.net/api#state

AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

/* Konverzia výstupov do JSON-u */
String getOutputStates()
{
  JSONVar myArray;
  if (LoRaData.length() > 0)
  {
    myArray["lora"] = LoRaData;
  }
  myArray["rssi"] = rssi;

  myArray["mqtt"] = String(mqtt_state);

  String jsonString = JSON.stringify(myArray);
  return jsonString;
}

void notifyClients(String state)
{
  ws.textAll(state);
}

void onLoRaData()
{
  String out = getOutputStates(); // LoRaData + ";RS:";

  // out.concat(rssi);
  int l_m = out.length() + 1;
  char tmp_m[l_m];
  out.toCharArray(tmp_m, l_m);
  mqttClient.publish(topic_meteo_status, 0, true, tmp_m);
  notifyClients(out);
}

/* Inicializácia prístupu k súborovému systému SPIFFS */
void initSPIFFS()
{
  SPIFFS.begin(true);
}

/* Funkcia pre pripojenie na wifi */
void connectToWifi()
{
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("w.");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/* Pripojenie k MQTT brokeru */
void connectToMqtt()
{
  mqttClient.connect();
}

/* Spustená pri prerušení spojenia s MQTT brokerom */
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  if (WiFi.isConnected())
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
  mqtt_state = 0; // Nastav príznak chýbajúceho MQTT spojenia
  notifyClients(getOutputStates());
}

/* Spustená pri pripojení k MQTT brokeru */
void onMqttConnect(bool sessionPresent)
{
  // Prihlásenie sa na odber:
  mqttClient.subscribe(topic_meteo_last, 1);

  mqtt_state = 1;                   // Nastav príznak MQTT spojenia
  notifyClients(getOutputStates()); // Aktualizuj stavy webu
}

/* Správa WIFI eventov */
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    connectToMqtt();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    xTimerStop(mqttReconnectTimer, 0);
    xTimerStart(wifiReconnectTimer, 0);
    break;
  default:
    break;
  }
}

/* Spracovanie správy z websocket */
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "states") == 0)
    { // Pre nastavenie počiatočného stavu
      notifyClients(getOutputStates());
    }
    else
    { // Pre konkrétnu zmenu
      if (mqtt_state == 1)
      {
        String t_m = LoRaData;
        int l_m = t_m.length() + 1;
        char tmp_m[l_m];
        t_m.toCharArray(tmp_m, l_m);
        mqttClient.publish(topic_meteo_status, 0, true, tmp_m);
      }
      else
      {
        notifyClients(getOutputStates());
      }
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    // Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void onMqttPublish(uint16_t packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

/* This functions is executed when some device publishes a message to a topic that your ESP32 is subscribed to
 * Change the function below to add logic to your program, so when a device publishes a message to a topic that
 * your ESP32 is subscribed you can actually do something */
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  // Zmeň message na string
  String messageTemp;
  for (int i = 0; i < len; i++)
  {
    messageTemp += (char)payload[i];
  }
  String topicTmp;
  for (int i = 0; i < strlen(topic); i++)
  {
    topicTmp += (char)topic[i];
  }

  // Ak príde správa s topic začínajúcim na main_topic_switch, tak zistím obsah správy a spracujem
  if (topicTmp.startsWith(topic_meteo_last))
  {
    // int topic_length = topicTmp.length() - 1;
    // String lamp_id = topicTmp.substring(topic_length);
    // int id_lamp = lamp_id.toInt();
    // byte state = messageTemp == "on" ? 1 : 0;
    // changeLight(id_lamp, state);
    notifyClients(getOutputStates());
  }
}

void setup()
{
  // initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  // setup LoRa transceiver module
  LoRa.setPins(LORA_CS_PIN, LORA_RESET_PIN, LORA_IRQ_PIN);
  while (!LoRa.begin(866E6))
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  LoRa.setSyncWord(LORA_SYNC_WORD);
  Serial.println("LoRa Initializing OK!");

  initSPIFFS();
  initWebSocket();

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.setKeepAlive(5); // Set the keep alive. Defaults to 15 seconds.
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER, MQTT_PASSWORD);

  connectToWifi();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html", false); });

  server.serveStatic("/", SPIFFS, "/");

  AsyncElegantOTA.begin(&server, OTA_USER, OTA_PASSWORD); // Štart ElegantOTA s autentifikáciou https://github.com/ayushsharma82/AsyncElegantOTA
  server.begin();                                         // Start server
}

void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    LoRaData = "";
    rssi = -120;
#if SERIAL_PORT_ENABLED
    // received a packet
    Serial.print("Prijaté dáta: '");
#endif
    // read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
#if SERIAL_PORT_ENABLED
      Serial.print(LoRaData);
#endif
    }
    rssi = LoRa.packetRssi();
    onLoRaData();
#if SERIAL_PORT_ENABLED
    // print RSSI of packet
    Serial.print("' with RSSI: ");
    Serial.print(rssi);
    Serial.print("dBm ");
    Serial.print((10 / 9) * (rssi + 120));
    Serial.println("%");
#endif
  }
}