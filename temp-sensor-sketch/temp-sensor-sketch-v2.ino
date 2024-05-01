#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// DHT dolgok
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi konstansok
const char* ssid = "Mucsphone";
const char* password = "balinttt";

//mosquitto constants
const char* mqtt_server = "192.168.141.88"; // Broker's IP inside Docker
const int mqtt_port = 1883; // Broker's port

WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
    }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  client.setServer(mqtt_server, mqtt_port);
  dht.begin();

}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Connecting to MQTT broker...");

        if (client.connect("ESP32Client")) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed to connect, state: ");
            Serial.println(client.state());
            delay(5000); // Retry after 5 seconds
        }
    }
}

void publishSensorData() {
    float h = dht.readHumidity(); // Read humidity
    float t = dht.readTemperature(); // Read temperature as Celsius

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read from DHT11 sensor!");
        return; // Exit if readings are invalid
    }

    // Create a JSON string in the message buffer
    char message[100];
    snprintf(message, sizeof(message), "{\"temperature\": %.2f, \"humidity\": %.2f}", t, h);

    // Publish the message to the specified topic
    if (!client.publish("sensor/data", message)) {
        Serial.println("Failed to publish sensor data");
    }
}


void loop() {
    if (!client.connected()) {
        reconnect(); // Reconnect to the MQTT broker if necessary
    }

    client.loop(); // Maintain MQTT connection

    publishSensorData(); // Read and publish sensor data

    delay(2000); // Pause for 2 seconds before next iteration
}
