#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Guggulla_IoT_Support";
const char* password = "9951414244";

ESP8266WebServer server(80);

const int dataPin = 12;  // DS (serial data input)
const int clockPin = 13; // SH_CP (clock)
const int latchPin = 14; // ST_CP (latch)

const int numOfRegisters = 4; // Assuming you're daisy-chaining 4 74HC595s

byte registers[numOfRegisters] = {0}; // Initialize registers to 0

void setup() {
  Serial.begin(115200);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  IPAddress ip(192, 168, 0, 20); // Define the desired IP address
  IPAddress gateway(192, 168, 0, 1); // Define your gateway IP address
  IPAddress subnet(255, 255, 255, 0); // Define your subnet mask

  WiFi.config(ip, gateway, subnet); // Set the static IP, gateway, and subnet

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  for (int i = 0; i < 32; i++) {
    String gpioOn = "/gpio" + String(i) + "/on/";
    String gpioOff = "/gpio" + String(i) + "/off/";
    String gpioStatus = "/gpio" + String(i) + "/status/";
    
    server.on(gpioOn.c_str(), HTTP_GET, [i](){
      setGPIOState(i, true);
      server.send(200, "text/plain", "OK");
    });
    
    server.on(gpioOff.c_str(), HTTP_GET, [i](){
      setGPIOState(i, false);
      server.send(200, "text/plain", "OK");
    });
    
    server.on(gpioStatus.c_str(), HTTP_GET, [i](){
      server.send(200, "text/plain", String(getGPIOState(i)));
    });
  }

  server.begin();
}

void loop() {
  server.handleClient();
}

void setGPIOState(int gpio, bool state) {
  int registerIndex = gpio / 8;
  int bitIndex = gpio % 8;

  bitWrite(registers[registerIndex], bitIndex, state);
  updateShiftRegister();
}

bool getGPIOState(int gpio) {
  int registerIndex = gpio / 8;
  int bitIndex = gpio % 8;

  return bitRead(registers[registerIndex], bitIndex);
}

void updateShiftRegister() {
  digitalWrite(latchPin, LOW);
  for (int i = numOfRegisters - 1; i >= 0; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, registers[i]);
  }
  digitalWrite(latchPin, HIGH);
}
