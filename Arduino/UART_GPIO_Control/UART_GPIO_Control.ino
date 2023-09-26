const int numPins = 54; // Total number of pins on Arduino Mega
bool pinStates[numPins]; // Array to store the states of each pin

void setup() {
  Serial.begin(115200);
  
  for(int i=0; i<numPins; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
    pinStates[i] = LOW; // Initialize pin states
  }
}

void loop() {
  if(Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read the command from serial
    
    if(command.startsWith("SET")) {
      int pin = command.substring(3).toInt(); // Extract the pin number
      if(pin >= 0 && pin < numPins) {
        int state = command.substring(6).toInt(); // Read the desired state
        pinStates[pin] = state; // Update pin state
        digitalWrite(pin, state);
        Serial.println("OK");
      } else {
        Serial.println("ERR"); // Invalid pin number
      }
    } else if(command.startsWith("GET")) {
      int pin = command.substring(3).toInt(); // Extract the pin number
      if(pin >= 0 && pin < numPins) {
        Serial.println(pinStates[pin]); // Send back the pin state
      } else {
        Serial.println("ERR"); // Invalid pin number
      }
    } else {
      Serial.println("ERR"); // Invalid command
    }
  }
}
