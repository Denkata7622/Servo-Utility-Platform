#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Math.h>

// Define Screen Parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Hardware Pins
const int servoPin = 38;
const int yPin = 1;
const int xPin = 2;
const int buttonPin = 12;

// Joystick offCenter Calibration Values
double xOffCenterCalibration = 0;
double yOffCenterCalibration = 0;

// Option to invert joystick directions
bool axisInverted = true;

// Servo Variables
double angle = 90; // Servo Angle
double minAngle = 0; // Using double for future servo calibration function
double maxAngle = 180; // Using double for future servo calibration function
double speed = 0; // Angles moved every Loop()
int loopDelay = 10;

// Joystick Speed Variables
double offCenterSquaredMultiplier = 8;
int offCenterLinearMultiplier = 3;
double offCenterAddition = -1;
double speedMultiplier = 1; // Change this when changing the loopDelay to keep the same ratio without fine tuning the whole Quad function again
int maxSpeed = 10;

// analogRead() to offCenter ratio
double joystickScaleDivider = 2048; // Use 2048 for esp32 and 1024 for Arduino

// Main Menu Options
String mainMenuOptions[] = {"Manual Control", "Calibration", "Debug", "Settings"};
int mainMenuLength = sizeof(mainMenuOptions) / sizeof(mainMenuOptions[0]);

// Calibration Menu Options
String calibrationMenuOptions[] = {"Simple Calibr.", "Pulse Width Calibr.", "C.E.A. LASER Calibr.", "360deg Servo", "180deg Servo", "Go Back"}; // Some options are just for example!
int calibrationMenuLength = sizeof(calibrationMenuOptions) / sizeof(calibrationMenuOptions[0]);

// Universal Menu Variables
int selected = 0;
int firstVisible = 0;
int maxOptions = 6; // Maximum options that can fit on the screen at the same time. Generic 128x64 displays can fit ~7 at setTextSize(1)
int maxCharsPerLine = 20;

// ########################
// Servo Profile Preferences:
// ########################

// Basic Profile:
// [0, 45, 90, 135, 180]
double SavedAnglesBasic[3][6]= {{0,0,0,0,0,0}, // made with 6 instead of 5 because SavedAngles[i] = SavedAngles[i - 1] + the angle on top of that
                                {0,0,0,0,0,0},
                                {0,0,0,0,0,0}};
// Accurate Profile:
// [0,
//  22.5,  45,   67.5,  90,
//  112.5, 135,  157.5, 180
// ]
double SavedAnglesAccurate[3][10]= {{0,0,0,0,0,0,0,0,0,0}, // made with 10 instead of 9 because SavedAngles[i] = SavedAngles[i - 1] + the angle on top of that
                                    {0,0,0,0,0,0,0,0,0,0},
                                    {0,0,0,0,0,0,0,0,0,0}};
// Servophile Profile:
// [0, 
//  11.25,  22.5,  33.75,  45,
//  56.25,  67.5,  78.75,  90, 
//  101.25, 112.5, 123.75, 135, 
//  146.25, 157.5, 168.75, 180
// ]
double SavedAnglesServophile[3][14]= {{0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // made with 14 instead of 13 because SavedAngles[i] = SavedAngles[i - 1] + the angle on top of that
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

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
  display.setCursor(5, 32);  
  display.println("Developed by Kacenta");

  display.display();
  delay(3000);

  // ALWAYS KEEP THIS AT THE BOTTOM
  TestServo.attach(servoPin);
}


void loop()
{
  buildMenu(firstVisible, selected, mainMenuOptions, mainMenuLength);

  // Use the Joystick's Y axis to navigate
  int yReading = analogRead(yPin);
  double yOffCenter = calculateOffCenter(yReading);

  if (axisInverted)
  {
    yOffCenter = -yOffCenter;  
  }

  String yDirection = offCenterToDirection(yOffCenter, 1);

  // Read if button has been pressed (first checks for button on purpose)
  int buttonState = digitalRead(buttonPin);

  if (!buttonState)
  {
    callMenuOption(0, selected);
  }

  // Update Y based on yDirection
  dirUpdateYselected(yDirection, mainMenuLength);

  delay(50);
}


void buildMenu(int firstVisibleOption, int focusedOption, String menuOptions[], int menuLength)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  // Chooses the smaller of the 2 numbers
  int visibleOptions = min(maxOptions, menuLength);

  for (int i = 0; i < visibleOptions; i++)
  {
    int currentOption = i + firstVisibleOption;
    String menuOption = menuOptions[currentOption];
    display.setTextColor(WHITE);

    if (currentOption == focusedOption)
    {
      menuOption = String("> ") + menuOption;
      display.setTextColor(BLACK, WHITE);
    }

    // Make sure that each option fits in its own line (configure maxCharsPerLine to your screen)
    // TODO: make a scroll animation
    // For now - only trim
    menuOption = menuOption.substring(0, maxCharsPerLine);


    display.println(menuOption);
  }

  drawScrollbar(focusedOption, menuLength);

  display.display();
}

void drawScrollbar(int focusedOption, int menuLength)
{
  for(int i = 0; i < SCREEN_HEIGHT / 4 ; i++)
  {
    display.drawLine(SCREEN_WIDTH - 4, i * 4 + 2, SCREEN_WIDTH - 4, i * 4 + 4, WHITE);
  }

  for(int i = 0; i < menuLength; i++)
  {
    if (i == focusedOption)
    {
      if (i == menuLength - 1)
      {
        display.fillRect(SCREEN_WIDTH - 6, SCREEN_HEIGHT - 8, 5, 8, WHITE);
        continue;
      }
      else if (i == firstVisible)
      {
        display.fillRect(SCREEN_WIDTH - 6, 0, 5, 8, WHITE);
        continue;
      }
      else 
      {
        display.fillRect(SCREEN_WIDTH - 6, i * (SCREEN_HEIGHT - 8) / (menuLength - 1), 5, 8, WHITE);
      }
    }
  }
}

void callMenuOption(int contextMenu, int selectedOption)
{
  switch (contextMenu)
  {
    case 0:
      switch (selectedOption)
      {
        case 0:
          callManualControl();
          break;

        case 1:
          callCalibrationMenu();
          break;

        case 2:
          callDebug();
          break;

        case 3:
          //callSettings();
          break;

        case 4:
          //callAbout();
          break;

        default: 
          Serial.println("Wrong Option Called");
          Serial.print(selectedOption);
          break;
      }
      break;

    case 1:
      switch (selected)
      {
        case 0:
          break;
        
        case 1:
          break;

        // Laser Calibration
        case 2:
          callLaserCalibration();
          break;

        case 3:
          break;

        case 4:
          break;

        // Go Back
        case 5:
          break;

        default:
          break;
      }
      break;

    case 2:
      switch (selected)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        default:
          break;
      }
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
  String xDirection[] = {"RIGHT", "LEFT", "NONE"};

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


void dirUpdateYselected(String yDirection, int menuLength)
{
  // Actually DOWN makes "selected" go UP, because the list starts at 0 and INCREASES as it goes DOWN
  if (yDirection == "DOWN")
  {
    selected++;
    delay(180);
  }
  // UP makes "selected" go DOWN, because the list ends at menu[menuLength - 1] and DECREASES as it goes UP
  else if (yDirection == "UP")
  {
    selected--;
    delay(180);
  }

  fixSelected(menuLength);
}

// Check that keeps "selected" in range 0 to (menuLength - 1)
void fixSelected(int menuLength)
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
    double yOffCenter = calculateOffCenter(yReading);

    if (axisInverted)
    {
      xOffCenter = -xOffCenter;
      yOffCenter = -yOffCenter;
    }

    String xDirection = offCenterToDirection(xOffCenter, 0);
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

  // Use a Quadratic Equation for Linear accereration + Quadratic boost near the ends
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

void callCalibrationMenu()
{
  delay(300);

  calibrationMenu();

  delay(300);
}

void calibrationMenu()
{
  selected = 0;
  firstVisible = 0;

  while(true)
  {
    buildMenu(firstVisible, selected, calibrationMenuOptions, calibrationMenuLength);

    // Use the Joystick's Y axis to navigate
    int yReading = analogRead(yPin);
    double yOffCenter = calculateOffCenter(yReading);

    // TODO Make a function invertAxis(int yesX,int yesY)
    if (axisInverted)
    {
      yOffCenter = -yOffCenter;  
    }

    // Convert offCenter to "UP" or "DOWN"
    String yDirection = offCenterToDirection(yOffCenter, 1);

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Keep "Go Back" last so it always works
      if (selected == calibrationMenuLength - 1)
      {
        // Go Back
        return;
      }

      callMenuOption(1, selected);
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, calibrationMenuLength);

    delay(50);
  }
}



void callLaserCalibration()
{
  delay(300);

  laserCalibration();

  delay(300);
}


//
// Use tg to calc the angle
// example:
// 200cm Distance from Laser to wall
// 10cm Distance from 0 deg to ? deg (in the code it's set to 15 deg)
// tg of needed angle = 10 / 200 = 0.05
// use atan(tg) to get angle in radians
// * 180 / Pi to convert to angles
// 
// Made to be Reusable (Unlike toilet paper) <3
//
double calcDistanceToAngle(double startingDistance, double distance)
{
  double angleTan = distance / startingDistance;
  double angleAtan = atan(angleTan);
  double angle = convertRadiansToDegrees(angleAtan);

  return angle;
}

// Multiplies by (180 / PI) to convert radians to angles
double convertRadiansToDegrees(double radians)
{
  double angle = radians * 180 / PI;
  return angle;
}


// 
void laserCalibration()
{
  //
  // TODO: Call a menu before starting and initialise arrays based on List chosen
  // chooseModeMenu()
  //

  // Saved Servo Variables
  double startingDistance = 0;
  double Distances[5] = {0, 0, 0, 0, 0};
  // saved angles is at the top cuz its used later
  int distancesLength = sizeof(Distances) / sizeof(Distances[0]);

  // Wait until user clicks and save distance
  startingDistance = getLaserToWallDistance();

  // Wait a bit to prevent accidental clicks 
  delay(300);

  // distance from servo to wall in cm
  double distance = 0;

  for (int i = 0; i < distancesLength; i++)
  {
    // RESET distance
    distance = 0;

    // Basic list of angles [0, 45, 90, 135, 180]
    int servoAngle = i * 45;

    // Set the servo to the corresponding angle 
    TestServo.write(servoAngle);

    // wait till user clicks
    while (true)
    {
      int yReading = analogRead(yPin);
      double yOffCenter = calculateOffCenter(yReading);
      double speed = offCenterToAngleChange(yOffCenter); 
      distance += (speed * speedMultiplier * 0.1); // TODO Change 0.2 with Y axis speed scale

      String Message = String("Input ") + (i * 45) + ("deg distance");
      
      // Update Screen to Show Results
      displayDistanceAndSpeedResult(distance, speed, Message);


      int buttonState = digitalRead(buttonPin);
      if (!buttonState)
      {
        // Save distance
        Distances[i] = distance;

        // Continue
        break;
      }
    }
    delay(300);
  }

  //
  // Show saved distances at the end
  // TODO: Separate with a function.
  display.clearDisplay();
  display.setCursor(0,0);

  display.println("startingDistance: ");
  display.println(startingDistance);

  display.display();
  
  delay(1500);

  display.clearDisplay();
  display.setCursor(0,0);

  display.println("Distances: ");
  display.println("0, 45, 90, 135, 180");
  display.println(Distances[0]);
  display.println(Distances[1]);
  display.println(Distances[2]);
  display.println(Distances[3]);
  display.println(Distances[4]);
  display.display();
  //
  //
  //

  delay(2000);

  display.clearDisplay();
  display.setCursor(0,0);

  display.println("Angles Saved:");

  double SavedAngles[100] = {0};

  // Use the Distances[] array to produce an Angles[] array
  for (int i = 0; i < distancesLength; i++)
  {
    SavedAngles[i + 1] = calcDistanceToAngle(startingDistance, Distances[i])  + SavedAngles[i];
    display.print(i * 45);
    display.print(": ");
    display.println(SavedAngles[i + 1]);
    SavedAnglesBasic[0][i + 1] = SavedAngles[i + 1]; // for now we only use basic
  }

  // TODO: save to correct SavedAngles array based on angle array choice

  display.display();

  delay(3000);
}

void displayDistanceAndSpeedResult(double inputDistance, double inputSpeed, String TopMessage)
{
  display.clearDisplay();
  display.setCursor(0, 0);

  // Set up Messages 
  String distanceMessage = String("Distance (cm): ") + inputDistance;

  // Print everything to screen
  display.println(TopMessage);
  display.println(distanceMessage);
  display.println(String("Speed: ") + inputSpeed);
  display.setCursor(25,45);
  display.println("CLICK TO SAVE");
  display.display();
}

double getLaserToWallDistance()
{
  // Set textSize only once
  display.setTextSize(1);

  // Initialize distance
  double distance = 0;

  while (true)
  {
    // Read joystick for speed
    int yReading = analogRead(yPin);
    double yOffCenter = calculateOffCenter(yReading);
    double speed = offCenterToAngleChange(yOffCenter); 
    distance += (speed * speedMultiplier * 0.2); // TODO: Change 0.2 with Y axis speed scale

    // Fix distance
    if (distance < 0)
    {
      distance = 0;
    }

    // Output Message
    displayDistanceAndSpeedResult(distance, speed, String("Input wallDistance"));
    
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Save Distance
      return distance;
    }
  }
}
