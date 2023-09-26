long int x1 = 0;  // the 0b prefix indicates a binary constant
long int x2 = 0;  // the 0b prefix indicates a binary constant
long int yy1 = 0;
long int yy2 = 0;

int max_bit = 32;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // wait for serial port to connect. Needed for native USB port only
}

void loop() {
  Serial.print(x2, BIN); // 10000000
  Serial.println(x1, BIN); // 10000000
  if (yy1 < max_bit){
    bitWrite(x1, yy1, 1);  // write 1 to the least significant bit of x
  }
  else {
    bitWrite(x2, yy2, 1);  // write 1 to the least significant bit of x
  }

  Serial.print(x2, BIN); // 10000000
  Serial.println(x1, BIN); // 10000000
  delay (2000);
  if (yy1 == max_bit){
    yy2 = yy2 + 1;
  }
  else{
    yy1 = yy1 + 1;
  }
}