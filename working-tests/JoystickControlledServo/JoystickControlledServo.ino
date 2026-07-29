#include <ESP32Servo.h>
//#include <math.h>

// Create a Servo Object
Servo Servo1;

// Hardware Pins
const int servoPin = 10;
const int yPin = 15;
const int xPin = 16;
const int buttonPin = 12;

// Variables Used
int buttonState = 0;
int yValue = 0;
int xValue = 0;

// Joystick offCenter
double xOffCenter = 0;

// Servo Angle
double angle = 90;
double minAngle = 0; // Using double for future calibration function
double maxAngle = 180; // Using double for future calibration function

// Angles per Loop
double speed = 0;

// Changes the speed-to-angle ratio
double speedMultiplier = 1; // Change this when changing the loopDelay to keep the same ratio without fine tuning the whole Quad function again

// Debugging Variables
int debuggerLoopCounter = 0;
int loopDelay = 10;
int waitLoopCount = 300 / loopDelay;

// Joystick Speed Variables
int x_power_2 = 1;
int x_power_1 = 14;
double x_power_0 = -1.25;
double joystickScaleDivider = 2048;
int maxSpeed = 12;

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

  xOffCenter = calculateOffCenter(xValue);
  speed = offCenterToSpeed(xOffCenter, x_power_2, x_power_1, x_power_0);
  angle += speed * speedMultiplier;

  angle = fixServoAngle(angle, minAngle, maxAngle);

  Servo1.write(angle);

  debuggerLoopCounter++;
  debuggerLoop();

  delay(loopDelay);
}

void debuggerLoop() 
{
  if (debuggerLoopCounter == waitLoopCount)
  {
    debuggerLoopCounter = 0;

    // Log X and Y axis Values
    Serial.println("xValue: ");
    Serial.println(xValue);
    Serial.println("    yValue: ");
    Serial.println(yValue);

    // Log Servo Angle
    Serial.println("    Servo Angle: ");
    Serial.println(angle);

    // Log Speed
    Serial.println("speed: ");
    Serial.println(speed);

    // Log offCenter
    Serial.println("     xOffCenter: ");
    Serial.println(xOffCenter);

  }
}

// Caclulates offCenter by giving it a floating point number from -1 to 1
double calculateOffCenter(int xValueInput)
{
  double xValueToRange = map(xValueInput, 0, 4095, -2048, 2047);
  double offCenter = xValueToRange / joystickScaleDivider;

  return offCenter;
}

// Transfers offcenter to angle change with position - or +
double offCenterToSpeed(double axisOffCenter, int power2,int power1, double power0)
{
  int sign = 0;
  double speed = 0;
  if (axisOffCenter <= 0.05 && axisOffCenter >= -0.05)
  {
    return speed;
  }
  else if (axisOffCenter < 0)
  {
    sign = -1;
  }
  else 
  {
    sign = 1;
  }

  // Use a Quadratic Equation for Linear accereration + slight boost at the end
  speed = sign * (axisOffCenter * axisOffCenter) * power2 + axisOffCenter * power1 + sign * power0;
  
  // Check if power0 causes the speed to change sign 
  if (sign == -1) {
    if (speed > 0)
    {
      // Makes it so servo can't go right if you move the joystick to the left
      speed = 0;
    }
  }
  else {
    if (speed < 0) {
      // Makes it so servo can't go left if you move the joystick to the right
      speed = 0;
    }
  }

  // Check if Speed has gone over maxSpeed or under -maxSpeed
  if(speed > maxSpeed) 
  {
    speed = maxSpeed;
  }
  else if (speed < -maxSpeed)
  {
    speed = -maxSpeed;
  }

  // Return Speed
  return speed;
}

// Check if angle exceeds minAngle or maxAngle
double fixServoAngle(double angle, double minAngle, double maxAngle)
{
  if (angle <= maxAngle && angle >= minAngle)
  {
    // Do nothing
  }
  else if (angle > maxAngle)
  {
    angle = maxAngle;
  }
  else
  {
    angle = minAngle;
  }

  return angle;
}