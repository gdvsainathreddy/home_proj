#include <Arduino.h>
#include <Matter.h>
#include <WiFi.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

MatterDevice device;
int ledPin = 2; // Assuming your LED is connected to GPIO pin 2

void onOffCallback(bool on) {
  digitalWrite(ledPin, on ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize Matter
  device.begin();
  device.onOnOff(onOffCallback);

  // Set up LED pin
  pinMode(ledPin, OUTPUT);
}

void loop() {
  device.update();
}
