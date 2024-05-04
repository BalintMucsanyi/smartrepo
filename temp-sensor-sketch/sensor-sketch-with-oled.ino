//OLED libraried for esp
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

//Network connection, pub/sub client and DHT
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

//OLED constants
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);


//DHT constants
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


const int lightPin = 35;
const char* ssid = "DIGI-EhP3";
const char* password = "N3M4Pe38";
const char* mqtt_server = "192.168.100.34";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
  Serial.begin(115200);
  delay(1000);

  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to WiFi");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
}

void displaySensorData(float t, float h, float l) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Sensor Readings:");
    display.print("Temp: ");
    display.print(t);
    display.println(" C");

    display.print("Humidity: ");
    display.print(h);
    display.println(" %");

    display.print("Light: ");
    display.print(l);
    display.display(); // Actually display all of the above
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

void publishSensorData() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int rawLight = analogRead(lightPin);
    float l = map(rawLight, 0, 4095, 100, 0);

    if (isnan(h) || isnan(t)) {
        Serial.println("Failed to read sensors!");
        return;
    }

    char message[100];
    snprintf(message, sizeof(message), "{\"temp\": %.2f, \"hum\": %.2f, \"light\": %.2f}", t, h, l);
    if (!client.publish("sensor/data", message)) {
        Serial.println("Publish failed");
    }
    Serial.printf("Temp: %.2f, Hum: %.2f, Light: %.2f (raw: %d)\n", t, h, l, rawLight);
    displaySensorData(t, h, l); // Update the OLED display with new data
}


void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    publishSensorData();
    delay(2000);
}
