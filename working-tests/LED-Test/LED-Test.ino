#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define Screen Parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Create a Servo Object

// Hardware Pins
const int servoPin = 10;
const int yPin = 15;
const int xPin = 16;
const int buttonPin = 12;

// Joystick offCenter Calibration Values
double xOffCenterCalibration = 0;
double yOffCenterCalibration = 0;

// Servo Angle
double angle = 90;
double minAngle = 0; // Using double for future servo calibration function
double maxAngle = 180; // Using double for future servo calibration function
double speed = 0; // Angles moved every Loop()
int maxSpeed = 4;
int loopDelay = 10;
double speedMultiplier = 1; // Change this when changing the loopDelay to keep the same ratio without fine tuning the whole Quad function again

// Joystick Speed Variables
int offCenterSquaredMultiplier = 1;
int offCenterLinearMultiplier = 14;
double offCenterAddition = -1.25;

// analogRead() to offCenter ratio
double joystickScaleDivider = 2048; // Use 2048 for esp32 and 1024 for Arduino

// Array for menu options
String menuOptions[] = {"Manual Control", "Calibration", "Debug", "Settings"};
int menuLength = sizeof(menuOptions) / sizeof(menuOptions[0]);
int selected = 0;
int firstVisible = 0;
int maxOptions = 6; // Maximum options that can fit on the screen at the same time. Generic 128x64 displays can fit ~7 at setTextSize(1)

// Make a display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo TestServo;

void setup() 
{
  Serial.begin(115200);

  // Give buttonPin an internal pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);

  // Set up display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  // Prototype version of a startup display 
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30,15);
  display.println("S.U.P.");
  display.setTextSize(1);
  display.println("Developed by Kacenta");

  display.display();
  delay(3000);

  // ALWAYS KEEP THIS AT THE BOTTOM
  TestServo.attach(servoPin);
}


void loop()
{
  buildMenu(firstVisible, selected);

  // Use the Joystick's Y axis to navigate
  int yReading = analogRead(yPin);
  double yOffCenter = calculateOffCenter(yReading);
  String yDirection = offCenterToDirection(yOffCenter, 1);

  // Read if button has been pressed (first checks for button on purpose)
  int buttonState = digitalRead(buttonPin);

  if (!buttonState)
  {
    callMenuOption(selected);
  }

  // Update Y based on yDirection
  dirUpdateYselected(yDirection);

  delay(50);
}


void buildMenu(int firstOption, int focusedOption)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  // Chooses the smaller of the 2 numbers
  int visibleOptions = min(maxOptions, menuLength);

  for (int i = 0; i < visibleOptions; i++)
  {
    display.setTextColor(WHITE);

    if (i == focusedOption)
    {
      display.setTextColor(BLACK, WHITE);
    }

    display.println(menuOptions[i]);
  }
  display.display();
}

void callMenuOption(int selectedOption)
{
  switch (selectedOption)
  {
    case 0:
      callManualControl();
      break;

    case 1:
      //callCalibrate();
      break;

    case 2:
      callDebug();
      break;

    case 3:
      //callSettings();
      break;

    default: 
      Serial.println("Wrong Option Called");
      Serial.print(selectedOption);
      break;
  }
}

void callDebug()
{
  // Prevent accidental "Go back" reading
  delay(300);

  // Immitate a new loop() function
  debuggerFunction();

  // Prevent accidental return to same option
  delay(300);  
}

// Caclulates offCenter by giving it a floating point number from -1 to 1
double calculateOffCenter(int ValueInput)
{
  double ValueToRange = map(ValueInput, 0, 4095, -2047, 2048);
  double offCenter = ValueToRange / joystickScaleDivider;

  return offCenter;
}


// Takes the Joystick value from 1 to -1 and converts it into an UP/DOWN/LEFT/RIGHT/NONE direction
// call with 0 for X and 1 for Y
String offCenterToDirection(double axisOffCenter, int Axis)
{
  String yDirection[] = {"UP", "DOWN", "NONE"};
  String xDirection[] = {"LEFT", "RIGHT", "NONE"};

  // Set to NONE by default
  int dirSelect = 2;

  if (axisOffCenter < -0.07)
  {
    dirSelect = 1;
  } 
  else if (axisOffCenter > 0.07)
  {
    dirSelect = 0;
  }
  else
  {
    dirSelect = 2;
  }

  // Return Based on Axis
  if(Axis == 1)
  {
    return yDirection[dirSelect];
  }
  else if(Axis == 0)
  {
    return xDirection[dirSelect];
  }
  else
  {
    Serial.println("Error: Wrong Axis specified!");
    return "NONE";
  }
}


void dirUpdateXselected(String xDirection)
{
  // TODO - add some additional function for X in the main menu
}


void dirUpdateYselected(String yDirection)
{
  // Actually DOWN makes "selected" go UP, because the list starts at 0 and INCREASES as it goes DOWN
  if (yDirection == "DOWN")
  {
    selected++;
    delay(180);
  }
  // UP makes "selected" go DOWN, because the list ends at menu[menuLenght - 1] and DECREASES as it goes UP
  else if (yDirection == "UP")
  {
    selected--;
    delay(180);
  }

  fixSelected();
}

// Check that keeps "selected" in range 0 to (menuLength - 1)
void fixSelected()
{
  if (selected < 0)
  {
    selected = 0;
  }
  else if (selected > menuLength - 1)
  {
    selected = menuLength - 1;
  }
}

// Function that tracks offCenter of both X and Y axis of the joystick and their Direction (UP/DOWN/LEFT/RIGHT/NONE)
// To use directly just clear loop() and call debuggerFunction()
void debuggerFunction()
{
  // Immitate a new loop() function
  while (true)
  {
    int yReading = analogRead(yPin);
    int xReading = analogRead(xPin);
    double xOffCenter = calculateOffCenter(xReading);
    String xDirection = offCenterToDirection(xOffCenter, 0);
    double yOffCenter = calculateOffCenter(yReading);
    String yDirection = offCenterToDirection(yOffCenter, 1);

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println(xOffCenter);
    display.println(xDirection);
    display.println(yOffCenter);
    display.println(yDirection);
    display.display();

    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Go back
      return;
    }

    delay(100);
  }
}

// Transfers offcenter to angle change with position - or +
double offCenterToAngleChange(double xOffCenter)
{
  int sign = 0;
  double speed = 0;
  if (xOffCenter <= 0.05 && xOffCenter >= -0.05)
  {
    return speed;
  }
  else if (xOffCenter < 0)
  {
    sign = -1;
  }
  else 
  {
    sign = 1;
  }

  // Use a Quadratic Equation for Linear accereration + slight boost at the end
  speed = (sign * (xOffCenter * xOffCenter) * offCenterSquaredMultiplier) + (xOffCenter * offCenterLinearMultiplier) + (sign * offCenterAddition);
  
  // Check if offCenterAddition causes the speed to change sign 
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
  if (angle > maxAngle)
  {
    angle = maxAngle;
  }
  else if (angle < minAngle)
  {
    angle = minAngle;
  }

  return angle;
}


void callManualControl()
{
  delay(300);

  manualControl();

  delay(300);
}

void manualControl() 
{
  // Immitate a new loop() function
  while (true)
  {
    int xReading = analogRead(xPin);
    double xOffCenter = calculateOffCenter(xReading);
    double angleChange = offCenterToAngleChange(xOffCenter);
    angle += angleChange * speedMultiplier;

    angle = fixServoAngle(angle, minAngle, maxAngle);
    TestServo.write(angle);

    display.clearDisplay();

    display.setCursor(0,0);
    display.setTextSize(1);
    String showCurrentAngle = String("Current Angle: ") + angle;
    display.println(showCurrentAngle);
    String showCurrentSpeed = String("Current Speed: ") + angleChange;
    display.println(showCurrentSpeed);

    display.display();


    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Go back
      return;
    }
    delay(loopDelay);
  }
}
