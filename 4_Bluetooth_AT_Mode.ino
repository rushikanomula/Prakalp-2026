#include <SoftwareSerial.h>

SoftwareSerial BT(11, 12); // Arduino RX, TX

void setup() {
  Serial.begin(9600);
  BT.begin(38400); // Default AT mode baud rate
  Serial.println("READY. TYPE AT");
}

void loop() {
  if (BT.available()) {
    Serial.write(BT.read());
  }
  if (Serial.available()) {
    BT.write(Serial.read());
  }
}
