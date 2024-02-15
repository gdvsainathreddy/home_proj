#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>

#define LOCKED_PIN 0   // GPIO0
#define UNLOCKED_PIN 2 // GPIO2
// #define RESET_PIN 3    // GPIO3
// #define EEPROM_SIZE 512

const char *serverAddress = "192.168.0.152";
const int serverPort = 80;
const int trigerserverPort = 9876;
const char *triggerURL = "/triggered?code=9876";

IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

volatile boolean locked = true;
boolean previousLockedState = true;

ESP8266WebServer server(80);


// void handlesetup() {
//   String page = "<html><body>";
//   page += "<h1>ESP-01S Configuration</h1>";
//   page += "</body></html>";
//   server.send(200, "text/html", page);
// }


// void handleRoot() {
//   String page = "<html><body>";
//   page += "<h1>ESP-01S Configuration</h1>";
//   page += "<form action='/config' method='post'>";
//   page += "SSID: <input type='text' name='ssid'><br>";
//   page += "Password: <input type='text' name='password'><br>";
//   page += "Static IP: <input type='text' name='staticIP'><br>";
//   page += "<input type='submit' value='Connect'></form>";
//   page += "</body></html>";

//   server.send(200, "text/html", page);
// }

// void handleConfig() {
//   String ssid = server.arg("ssid");
//   String password = server.arg("password");
//   String staticIP = server.arg("staticIP");

//   // // Store SSID, password, and static IP in EEPROM
//   // int address = 0;
//   // for (int i = 0; i < ssid.length(); ++i) {
//   //   EEPROM.write(address++, ssid[i]);
//   // }
//   // EEPROM.write(address++, '\0'); // Null terminator for the string
//   // for (int i = 0; i < password.length(); ++i) {
//   //   EEPROM.write(address++, password[i]);
//   // }
//   // EEPROM.write(address++, '\0'); // Null terminator for the string
//   // for (int i = 0; i < staticIP.length(); ++i) {
//   //   EEPROM.write(address++, staticIP[i]);
//   // }
//   // EEPROM.write(address++, '\0'); // Null terminator for the string
//   // EEPROM.commit();

//   String response = "<html><body>";
//   response += "<h1>Configuration saved. Rebooting ESP-01S...</h1>";
//   response += "</body></html>";
//   server.send(200, "text/html", response);

//   //ESP.restart();
// }

void handleStatus() {
  String response;
  if (locked) {
    response = "{\"currentState\": 1}";
  } else {
    response = "{\"currentState\": 0}";
  }
  server.send(200, "application/json", response);
}

// void handleLock() {
//   if (digitalRead(LOCKED_PIN) == HIGH && digitalRead(UNLOCKED_PIN) == LOW) {
//     locked = true;
//     sendStateToServer(1);
//   }
// }

// void handleUnlock() {
//   if (digitalRead(LOCKED_PIN) == LOW && digitalRead(UNLOCKED_PIN) == HIGH) {
//     locked = false;
//     sendStateToServer(0);
//     triggerAlarm();
//   }
// }

// void handleReset() {
//   delay(5000); // Wait for 5 seconds
//   if (digitalRead(RESET_PIN) == LOW) {
//     // Erase EEPROM
//     for (int i = 0; i < EEPROM_SIZE; ++i) {
//       EEPROM.write(i, 0);
//     }
//     EEPROM.commit();
//     ESP.restart();
//   }
// }

void sendStateToServer(int state) {
  String url = "/lock1/setState?value=";
  url += state;
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverAddress, serverPort, url);
  int httpCode = http.GET();
  http.end();
  // Serial.print("Current State: ");
  // Serial.println(state);
  if (httpCode == 200) {
    Serial.println("State sent successfully");
  } else {
    Serial.println("Failed to send state");
  }
}

void triggerAlarm() {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverAddress, trigerserverPort, triggerURL);
  int httpCode = http.GET();
  http.end();
  if (httpCode == 200) {
    Serial.println("Alarm triggered successfully");
  } else {
    Serial.println("Failed to trigger alarm");
  }
}

// void lockedInterrupt() {
//   handleLock();
// }

// void unlockedInterrupt() {
//   handleUnlock();
// }

void setup() {
  Serial.begin(115200);

  pinMode(LOCKED_PIN, INPUT_PULLUP);
  pinMode(UNLOCKED_PIN, INPUT_PULLUP);
  // pinMode(RESET_PIN, INPUT_PULLUP);

  //attachInterrupt(digitalPinToInterrupt(LOCKED_PIN), lockedInterrupt, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(UNLOCKED_PIN), unlockedInterrupt, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(RESET_PIN), handleReset, FALLING);

  // EEPROM.begin(EEPROM_SIZE);

  // Read stored SSID, password, and static IP
  String storedSSID = "Guggulla_IoT_Support";
  String storedPassword = "9951414244";
  String storedStaticIP = "192.168.0.205";
  // int address = 0;
  // char currentChar = EEPROM.read(address++);
  // while (currentChar != '\0') {
  //   storedSSID += currentChar;
  //   currentChar = EEPROM.read(address++);
  // }
  // Serial.print("Stored SSID: ");
  // Serial.println(storedSSID);

  // currentChar = EEPROM.read(address++);
  // while (currentChar != '\0') {
  //   storedPassword += currentChar;
  //   currentChar = EEPROM.read(address++);
  // }
  // Serial.print("Stored Password: ");
  // Serial.println(storedPassword);

  // currentChar = EEPROM.read(address++);
  // while (currentChar != '\0') {
  //   storedStaticIP += currentChar;
  //   currentChar = EEPROM.read(address++);
  // }
  // Serial.print("Stored Static IP: ");
  // Serial.println(storedStaticIP);

  // Convert stored IP address to IPAddress object
  IPAddress staticIP;
  // if (!storedStaticIP.isEmpty()) {
    staticIP.fromString(storedStaticIP);
  // }

  WiFi.mode(WIFI_STA);

  // if (!staticIP.isSet()) {
  //   // If no stored static IP, set up ESP-01S as an access point
  //   WiFi.mode(WIFI_AP);
  //   WiFi.softAPConfig(gateway, gateway, subnet); // Set IP to 192.168.0.1
  //   WiFi.softAP("LazyDoorLock_AP", "12345678");
  // } else {
    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
    WiFi.config(staticIP, gateway, subnet);
  // }

  server.on("/", HTTP_GET, handleStatus);
  // server.on("/config", HTTP_POST, handleConfig);
  server.on("/lock1/status", HTTP_GET, handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();

    // Check LOCKED_PIN for changes
  boolean currentLockedState = digitalRead(LOCKED_PIN); //1
  if (currentLockedState != previousLockedState) {      //1 != 0
    if (currentLockedState == LOW) {   // 1 and 0
      locked = true;
      sendStateToServer(1);
      // Serial.println("LOCKED");
    }
    previousLockedState = currentLockedState;           //1
      // Check UNLOCKED_PIN for changes
    if (digitalRead(LOCKED_PIN) == HIGH) {   //1 and 0
      locked = false;
      triggerAlarm();
      sendStateToServer(0);
      // Serial.println("UNLOCKED");
    }
  }



  //   // Check LOCKED_PIN for changes
  // boolean currentLockedState = digitalRead(LOCKED_PIN); //1
  // if (currentLockedState != previousLockedState) {      //1 != 0
  //   if (currentLockedState == HIGH && digitalRead(UNLOCKED_PIN) == LOW) {   // 1 and 0
  //     locked = true;
  //     sendStateToServer(1);
  //   }
  //   previousLockedState = currentLockedState;           //1
  // }

  // // Check UNLOCKED_PIN for changes
  // if (digitalRead(UNLOCKED_PIN) == HIGH && previousLockedState == LOW) {   //1 and 0
  //   locked = false;
  //   sendStateToServer(0);
  //   triggerAlarm();
  // }
}
