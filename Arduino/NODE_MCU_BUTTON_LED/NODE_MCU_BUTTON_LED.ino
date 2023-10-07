uint8_t LED_Pin = D4;       // declare LED pin on NodeMCU Dev Kit
uint8_t push1 = D2;

int buttonState1 = 0;
void setup() {
	pinMode(LED_Pin, OUTPUT);   // Initialize the LED pin as an output
  pinMode(push1, INPUT);
}

void loop() {
  buttonState1 = digitalRead(push1);
  
  if (buttonState1 == LOW) {
    // turn LED on:
    digitalWrite(LED_Pin, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LED_Pin, LOW);
  }

}