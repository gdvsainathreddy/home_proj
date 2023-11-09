#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const char* ssid = "Guggulla_IoT_Support";
const char* password = "9951414244";

ESP8266WebServer server(80);

const int dataPin = 12;  // DS (serial data input)
const int clockPin = 13; // SH_CP (clock)
const int latchPin = 14; // ST_CP (latch)

uint8_t DoorBell = D1;
uint8_t DoorSensor = D2;
uint8_t MotionSensor = D0;

uint8_t WiFiStatus = D8;

int DoorBellState = 0;
int DoorSensorState = 0;
int lastDoorSensorState = 0;
int MotionSensorState = 0;
int lastMotionSensorState = 0;

const int numOfRegisters = 4; // Assuming you're daisy-chaining 4 74HC595s

byte registers[numOfRegisters] = {0}; // Initialize registers to 0

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  pinMode(DoorBell, INPUT);
  pinMode(DoorSensor, INPUT);
  pinMode(MotionSensor, INPUT);

  pinMode(WiFiStatus, OUTPUT);

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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(WiFiStatus, 1);
  Serial.println("Loading States From EEPROM");
  loadStateFromEEPROM();
  Serial.println("Updating Powerup states");
  updateShiftRegister();
  checkDoorOnPowerUp();



  for (int i = 0; i < 32; i++) {
    String gpioOn = "/gpio" + String(i) + "/on/";
    String gpioOff = "/gpio" + String(i) + "/off/";
    String gpioStatus = "/gpio" + String(i) + "/status/";
    
    server.on(gpioOn.c_str(), HTTP_GET, [i](){
      setGPIOState(i, true);
      server.send(200, "text/plain", "OK");
      saveStateToEEPROM();
    });
    
    server.on(gpioOff.c_str(), HTTP_GET, [i](){
      setGPIOState(i, false);
      server.send(200, "text/plain", "OK");
      saveStateToEEPROM();
    });
    
    server.on(gpioStatus.c_str(), HTTP_GET, [i](){
      server.send(200, "text/plain", String(getGPIOState(i)));
    });
  }

  server.begin();
}

void loop() {
  server.handleClient();
  checkDoorBell();
  checkDoor();
  checkMotionSensor();
}

void checkMotionSensor() {
  MotionSensorState = digitalRead(MotionSensor);
  if (MotionSensorState != lastMotionSensorState) {
    if (MotionSensorState == HIGH) {
      Serial.println("MotionSensor Triggered");
      callURL("192.168.0.152", 18089, "/");
    }
  }
  lastMotionSensorState = MotionSensorState;
}

void checkDoorBell() {
  DoorBellState = digitalRead(DoorBell);
  if (DoorBellState == HIGH) {
    Serial.println("Door Bell Triggered");
    callURL("192.168.0.152", 9082, "/doorbell");
  }
}

void checkDoor() {
  DoorSensorState = digitalRead(DoorSensor);
  // compare the DoorSensorState to its previous state
  if (DoorSensorState != lastDoorSensorState) {
    // if the state has changed, increment the counter
    if (DoorSensorState == LOW) {
      // if the current state is HIGH then the button went from off to on:
      Serial.println("Door Sensor Opened");
      callURL("192.168.0.152", 9091, "/door/1");
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("Door Sensor Closed");
      callURL("192.168.0.152", 9091, "/door/0");
    }
  }
  // save the current state as the last state, for next time through the loop
  lastDoorSensorState = DoorSensorState;
}

void checkDoorOnPowerUp() {
  DoorSensorState = digitalRead(DoorSensor);
  if (DoorSensorState == LOW) {
    // if the current state is HIGH then the button went from off to on:
    Serial.println("Door Sensor Opened");
    callURL("192.168.0.152", 9091, "/door/1");
  } else {
    // if the current state is LOW then the button went from on to off:
    Serial.println("Door Sensor Closed");
    callURL("192.168.0.152", 9091, "/door/0");
  }
}

void setGPIOState(int gpio, bool state) {
  int registerIndex = gpio / 8;
  int bitIndex = gpio % 8;

  bitWrite(registers[registerIndex], bitIndex, state);
  if (gpio == 12 && state) {
    callURL("192.168.0.152", 9876, "/triggered?code=9876");
  }
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

void saveStateToEEPROM() {
  for (int i = 0; i < numOfRegisters; i++) {
    EEPROM.put(i * sizeof(byte), registers[i]);
    delay(10);
    EEPROM.commit();
  }
}

void loadStateFromEEPROM() {
  for (int i = 0; i < numOfRegisters; i++) {
    EEPROM.get(i * sizeof(byte), registers[i]);
  }
}

void callURL(const char* host, const int port, const char* endpoint) {
  WiFiClient client;
  const int httpPort = port;
  
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }
  
  client.print(String("GET ") + endpoint + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  
  while(client.connected() && !client.available()){
    delay(1); // Wait for data
  }
  
  while(client.connected() || client.available()){
    char c = client.read(); // Read data from server
    Serial.print(c);
  }
  
  client.stop();
}

