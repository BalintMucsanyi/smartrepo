#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// OLED display settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// DHT sensor settings
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Light sensor pin
const int lightPin = 35;

// LoRa transceiver settings
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 868E6 // Frequency band for Europe

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize and clear the OLED display
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) ;
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("LoRa Sender");
  display.display();

  // Initialize DHT sensor
  dht.begin();

  // Set up SPI and LoRa transceiver
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa initialized successfully");
}

void displaySensorData(float t, float h, float l) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Sending Data:");
  display.printf("Temp: %.2f C\nHumidity: %.2f %%\nLight: %.2f %%", t, h, l);
  display.display();
}

void sendLoRaData(float t, float h, float l) {
  char message[100];
  snprintf(message, sizeof(message), "{\"temp\": %.2f, \"hum\": %.2f, \"light\": %.2f}", t, h, l);
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  Serial.print("Sent packet: ");
  Serial.println(message);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int rawLight = analogRead(lightPin);
  float l = map(rawLight, 0, 4095, 100, 0);

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  displaySensorData(t, h, l); // Update the OLED display with new data
  sendLoRaData(t, h, l); // Send data via LoRa

  delay(2000); // Send data every 10 seconds
}
