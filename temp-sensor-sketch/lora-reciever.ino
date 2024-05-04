#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 866E6 // Europe frequency band

const char* ssid = "DIGI-EhP3";
const char* password = "N3M4Pe38";
const char* mqtt_server = "192.168.100.34";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
String LoRaData;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialization OK");
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, state: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    LoRaData = ""; // clear string
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }
    Serial.print("Received packet: ");
    Serial.println(LoRaData); // Print received data to the Serial Monitor

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, LoRaData);
    if (!error) {
      float t = doc["temp"];
      float h = doc["hum"];
      float l = doc["light"];
      char message[100];
      snprintf(message, sizeof(message), "{\"temp\": %.2f, \"hum\": %.2f, \"light\": %.2f}", t, h, l);
      client.publish("sensor/data", message);
    } else {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
    }
  } else {
    // No packet received
    Serial.println("No LoRa packet received.");
  }
  delay(2000);
}
