#include <ESP32Servo.h>

// Create a Servo Object
Servo Servo1;

// Hardware Pins
const int servoPin = 12;
const int yPin = 34;
const int xPin = 35;
const int buttonPin = 13;

// Variables Used
int buttonState = 0;
int yValue = 0;
int xValue = 0;

void setup() {
  // Attach Pin to Servo
  Servo1.attach(servoPin);

  // Start Serial
  Serial.begin(115200);

  // Give buttonPin an internal pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Read buttonState
  buttonState = digitalRead(buttonPin);

  // Read Joystick X and Y axis
  yValue = analogRead(yPin);
  xValue = analogRead(xPin);

  // Map X axis to static servo angle 
  int servoAngle = map(xValue, 0, 4095, 180, 0);
  Servo1.write(servoAngle); 

  // Check for button press and log it
  if (buttonState == LOW) {
    Serial.println("Button Pressed.");
  }

  Serial.print("\n");

  // Log X and Y axis Values
  Serial.println("xValue: ");
  Serial.println(xValue);
  Serial.println("    yValue: ");
  Serial.println(yValue);

  // Log Servo Angle
  Serial.println("    Servo Angle: ");
  Serial.println(servoAngle);

  delay(1000);
}