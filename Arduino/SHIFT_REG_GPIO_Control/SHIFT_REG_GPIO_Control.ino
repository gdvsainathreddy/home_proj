const int dataPin = 13;   // Pin connected to DS (serial data input) of 74HC595
const int latchPin = 15;  // Pin connected to ST_CP (latch pin) of 74HC595
const int clockPin = 14;  // Pin connected to SH_CP (clock pin) of 74HC595

byte dataArray[64];  // Array to store the state of all 512 GPIOs (512/8 = 64 bytes)

void setup() {
  Serial.begin(115200);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.startsWith("SET")) {
      int gpio, state;
      sscanf(input.c_str(), "SET %d %d", &gpio, &state);
      if (gpio >= 0 && gpio < 512 && (state == 0 || state == 1)) {
        int byteIndex = gpio / 8;
        int bitIndex = gpio % 8;
        bitWrite(dataArray[byteIndex], bitIndex, state);
        updateShiftRegisters();
        Serial.println("OK");
      } else {
        Serial.println("Invalid GPIO or state");
      }
    } else if (input.startsWith("GET")) {
      int gpio;
      sscanf(input.c_str(), "GET %d", &gpio);
      if (gpio >= 0 && gpio < 512) {
        int byteIndex = gpio / 8;
        int bitIndex = gpio % 8;
        int state = bitRead(dataArray[byteIndex], bitIndex);
        Serial.println(state);
      } else {
        Serial.println("Invalid GPIO");
      }
    }
  }
}

void updateShiftRegisters() {
  digitalWrite(latchPin, LOW);
  for (int i = 63; i >= 0; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, dataArray[i]);
  }
  digitalWrite(latchPin, HIGH);
}

