#include <ESP32Servo.h>

Servo Servo1;
int servoPin = 12;
int potPin = 34;

void setup() {
  Servo1.attach(servoPin);
}

void loop() {
  int reading = analogRead(potPin);
  int angle = map(reading, 0, 4095, 0, 180);
  Servo1.write(angle);
}
