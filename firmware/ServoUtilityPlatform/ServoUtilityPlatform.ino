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

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Debugger Option (Change to enable debugging)
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
bool debug = true;

// ##############
// Hardware Pins
// ##############
const int servoPin = 38;
const int yPin = 1;
const int xPin = 2;
const int buttonPin = 12;

// #################
// Servo Variables
// #################
double angle = 90; // Servo Angle
double minAngle = 0; // Using double for future servo calibration function
double maxAngle = 180; // Using double for future servo calibration function
double speed = 0; // Speed of angle change
int loopDelay = 10; // Delay every loop (handle cautiously)

// ###################
// Joystick Variables
// ###################

double xDeadzone = 0.15;
double yDeadzone = 0.15;

// Joystick offCenter Calibration Values
double xOffCenterCalibration = 0;
double yOffCenterCalibration = 0;

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Option to invert joystick directions
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
bool xAxisInverted = true;
bool yAxisInverted = true;
bool axisSwapped = false;

// Joystick Speed Multipliers
double offCenterSquaredMultiplier = 8;
double offCenterLinearMultiplier = 3;
double offCenterAddition = -1;

// Maximum speed (anything above that (or below -maxSpeed) will be flattened to this number :D)
double maxSpeed = 10;

// General speed Multiplier
double speedMultiplier = 1; // Change this when changing the loopDelay to keep the same ratio without fine tuning the whole speed function again

// Finer control when selecting distances (recommended to keep below 0.5)
double calibrationSpeedMultiplier = 0.1;

// Precice control for specific cases
double preciseSpeedMultiplier = 0.01;

// analogRead() to offCenter ratio
double joystickScaleDivider = 2048; // Use 2048 for esp32 and 1024 for Arduino


// #######################
// Universal Menu Variables
// #######################

int selected = 0;
int firstVisible = 0;
int maxOptions = 8; // Maximum options that can fit on the screen at the same time. Generic 128x64 displays can fit ~8 at setTextSize(1)
int maxCharsPerLine = 20; // Maximum ammount of chars that can fit on one line. For 128x64 use 20 chars at setTextSize(1)
int currentScrollPosition = 0; // Remembers the pos of the current animated element (which makes it work with ONLY ONE item at a time!)

int prettyPrintPageDelayMs = 7500;
int ScrollDelayMs = 200;
int ScrollSpacesUntilLoop = 4;

// ######################
// Menu Options Arrays
// ######################

// Main Menu Options
String mainMenuOptions[] = {"Manual Control", "Calibration", "Debug", "Settings", "About", "Exit"};
int mainMenuLength = sizeof(mainMenuOptions) / sizeof(mainMenuOptions[0]);

// Servo Type Options
String servoTypeOptions[] = {"90°", "180°", "360°", "Go Back"};
int servoTypesLength = sizeof(servoTypeOptions) / sizeof(servoTypeOptions[0]);

// Calibration Menu Options
String calibrationMenuOptions[] = {"Simple Calibration", "Pulse Width Calibration", "C.E.A. LASER Calibration", "Go Back"};
int calibrationMenuLength = sizeof(calibrationMenuOptions) / sizeof(calibrationMenuOptions[0]);

// Calibration List Picker Options
String pickCalibrationListMenuOptions[] = {"Basic", "Accurate", "Servophile", "Super Servophile", "MEGA Servophile", "OVERKILL Servophile", "Go Back"};
int pickCalibrationListMenuLength = sizeof(pickCalibrationListMenuOptions) / sizeof(pickCalibrationListMenuOptions[0]);

// Slot Picker Options
String pickCalibrationSlotMenuOptions[] = {"Slot 0", "Slot 1", "Slot 2", "Slot 3", "Slot 4", "Slot 5", "Go Back"};
int pickCalibrationSlotMenuLength = sizeof(pickCalibrationSlotMenuOptions) / sizeof(pickCalibrationSlotMenuOptions[0]);

String DebugMenuOptions[] = {"Debug Joystick", "Debug Error Messages", "Debug Calibration Angles", "Debug Custom Scroll",
                             "Debug Settings Variable Updates", "Simulate Errors", "Go Back"};
int DebugMenuLength = sizeof(DebugMenuOptions) / sizeof(DebugMenuOptions[0]);

// Error Message Used for Debugging
String DebugErrorMessage = "Non-Existing Menu Option: /nWriting at 0x000544e3 [==================>           ]  66.2% 147456/222828 bytes... /nWriting at 0x0005d3dc [=====================>        ]  73.5% 163840/222828 bytes... /nWrote 403680 bytes (222828 compressed) at 0x00010000 in 4.3 seconds (749.3 kbit/s). /nVerifying written data... /nHash of data verified. /nHard resetting via RTS pin...";

// Settings Menu Options
// Stores all varNames that the user should have access to changing
String settingsMenuOptions[] = {"xAxisInverted", "yAxisInverted", "axisSwapped", "maxSpeed", "speedMultiplier",
                                "calibrationSpeedMultiplier", "offCenterSquaredMultiplier", "offCenterLinearMultiplier",
                                "offCenterAddition", "debug", "maxOptions", "maxCharsPerLine", "Go Back"};
int settingsMenuLength = sizeof(settingsMenuOptions) / sizeof(settingsMenuOptions[0]);

// Error ID List
String ErrorTypes[] = {"Error 101: Dummy Error for Debugging", "Error 104: Non-Existing Menu", "Error 105: Non-Existing Menu Option", "Error 106: Unkown Error Specified", "Error 203: Wrong Axis Specified", "Error 204: Wrong Setting Type Specified", "Go Back"};
int ErrorTypesLength = sizeof(ErrorTypes) / sizeof(ErrorTypes[0]);

// settings[] can be found ~200 lines lower (the bottom of the variable setup)

// ########################
// Servo Profile Preferences:
// ########################

// Uses 0 for Basic, 1 for accurate, 2 for servophile (3 for super servophile)
int usedCalibrationList = 0;
int usedCalibrationListSlot = 0;

// [usedCalibrationProfile] can be found at the bottom of the variable setup (around line ~300)

// ###########################
// Basic Profile:
// [0, 45, 90, 135, 180]
// ###########################

double SavedAnglesBasic[6][5]= {{0,0,0,0,0}, 
                                {0,0,0,0,0},
                                {0,0,0,0,0},
                                {0,0,0,0,0},
                                {0,0,0,0,0},
                                {0,0,0,0,0}};
                                
// Length of the SavedAnglesBasic List (5)
const int SavedAnglesBasicLength = sizeof(SavedAnglesBasic[0]) / sizeof(SavedAnglesBasic[0][0]);

// Basic Profile Reference
double SavedAnglesBasicReference[] = {0, 45, 90, 135, 180};

// #########################
// Accurate Profile:
// #########################

double SavedAnglesAccurate[6][9]= {{0,0,0,0,0,0,0,0,0}, 
                                  {0,0,0,0,0,0,0,0,0},
                                  {0,0,0,0,0,0,0,0,0},
                                  {0,0,0,0,0,0,0,0,0},
                                  {0,0,0,0,0,0,0,0,0},
                                  {0,0,0,0,0,0,0,0,0}};

// Length of the SavedAnglesAccurate List (9)
const int SavedAnglesAccurateLength = sizeof(SavedAnglesAccurate[0]) / sizeof(SavedAnglesAccurate[0][0]);

// Accurate Profile Reference
double SavedAnglesAccurateReference[] =
{ 
 0,
 22.5,  45,   67.5,  90,
 112.5, 135,  157.5, 180
};

// #########################
// Servophile Profile:
// #########################

double SavedAnglesServophile[6][17] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                      {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

// Length of the SavedAnglesServophile List (17)
const int SavedAnglesServophileLength = sizeof(SavedAnglesServophile[0]) / sizeof(SavedAnglesServophile[0][0]);

// Servophile Profile Reference
double SavedAnglesServophileReference[] = 
{
 0, 
 11.25,  22.5,  33.75,  45,
 56.25,  67.5,  78.75,  90, 
 101.25, 112.5, 123.75, 135, 
 146.25, 157.5, 168.75, 180
};

// #########################
// Super Servophile Profile:
// #########################

double SavedAnglesSuperServophile[6][33] =  {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

// Length of the SavedAnglesSuperServophile List (33)
const int SavedAnglesSuperServophileLength = sizeof(SavedAnglesSuperServophile[0]) / sizeof(SavedAnglesSuperServophile[0][0]);

// Super Servophile Profile Reference
double SavedAnglesSuperServophileReference[] = 
{
 0, 
 5.625,   11.25,  16.875,  22.5,
 28.125,  33.75,  39.375,  45,
 50.625,  56.25,  61.875,  67.5,
 73.125,  78.75,  84.375,  90,
 95.625,  101.25, 106.875, 112.5,
 118.125, 123.75, 129.375, 135,
 140.625, 146.25, 151.875, 157.5,
 163.125, 168.75, 174.375, 180
};

// #########################
// MEGA Servophile Profile:
// #########################

double SavedAnglesMEGAServophile[6][65] = {{0.00, 2.80, 5.60, 8.50, 11.30, 14.10, 16.90, 19.70, 22.50, 25.30, 28.10, 30.90, 33.80, 36.60, 39.40, 42.20, 45.00, 47.80, 50.60, 53.40, 56.30, 59.10, 61.90, 64.70, 67.50, 70.30, 73.10, 75.90, 78.80, 81.60, 84.40, 87.20, 90.00, 92.80, 95.60, 98.40, 101.30, 104.10, 106.90, 109.70, 112.50, 115.30, 118.10, 120.90, 123.80, 126.60, 129.40, 132.20, 135.00, 137.80, 140.60, 143.40, 146.30, 149.10, 151.90, 154.70, 157.50, 160.30, 163.10, 165.90, 168.80, 171.60, 174.40, 177.20, 180.00},
                                          {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                          {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                          {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                          {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                          {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

// Length of the SavedAnglesMEGAServophile List (65)
const int SavedAnglesMEGAServophileLength = sizeof(SavedAnglesMEGAServophile[0]) / sizeof(SavedAnglesMEGAServophile[0][0]);

// Super Servophile Profile Reference
double SavedAnglesMEGAServophileReference[] = 
{
 0, 
 2.8125,   5.625,   8.4375,   11.25,
 14.0625,  16.875,  19.6875,  22.5,
 25.3125,  28.125,  30.9375,  33.75,
 36.5625,  39.375,  42.1875,  45,
 47.8125,  50.625,  53.4375,  56.25,
 59.0625,  61.875,  64.6875,  67.5,
 70.3125,  73.125,  75.9375,  78.75,
 81.5625,  84.375,  87.1875,  90,
 92.8125,  95.625,  98.4375,  101.25,
 104.0625, 106.875, 109.6875, 112.5,
 115.3125, 118.125, 120.9375, 123.75,
 126.5625, 129.375, 132.1875, 135,
 137.8125, 140.625, 143.4375, 146.25,
 149.0625, 151.875, 154.6875, 157.5,
 160.3125, 163.125, 165.9375, 168.75,
 171.5625, 174.375, 177.1875, 180
};

// ################################
// OVERKILL Servophile Profile:
// ################################

double savedAnglesOVERKILLServophile[6][129] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

// Length of the SavedAnglesMEGAServophile List (129)
const int savedAnglesOVERKILLServophileLength = sizeof(savedAnglesOVERKILLServophile[0]) / sizeof(savedAnglesOVERKILLServophile[0][0]);

double savedAnglesOVERKILLServophileReference[] = 
{0,
 1.40625, 2.8125, 4.21875, 5.625, 7.03125, 8.4375, 9.84375, 11.25,
 12.65625, 14.0625, 15.46875, 16.875, 18.28125, 19.6875, 21.09375, 22.5,
 23.90625, 25.3125, 26.71875, 28.125, 29.53125, 30.9375, 32.34375, 33.75,
 35.15625, 36.5625, 37.96875, 39.375, 40.78125, 42.1875, 43.59375, 45,
 46.40625, 47.8125, 49.21875, 50.625, 52.03125, 53.4375, 54.84375, 56.25,
 57.65625, 59.0625, 60.46875, 61.875, 63.28125, 64.6875, 66.09375, 67.5,
 68.90625, 70.3125, 71.71875, 73.125, 74.53125, 75.9375, 77.34375, 78.75,
 80.15625, 81.5625, 82.96875, 84.375, 85.78125, 87.1875, 88.59375, 90,
 91.40625, 92.8125, 94.21875, 95.625, 97.03125, 98.4375, 99.84375, 101.25,
 102.65625, 104.0625, 105.46875, 106.875, 108.28125, 109.6875, 111.09375, 112.5,
 113.90625, 115.3125, 116.71875, 118.125, 119.53125, 120.9375, 122.34375, 123.75,
 125.15625, 126.5625, 127.96875, 129.375, 130.78125, 132.1875, 133.59375, 135,
 136.40625, 137.8125, 139.21875, 140.625, 142.03125, 143.4375, 144.84375, 146.25,
 147.65625, 149.0625, 150.46875, 151.875, 153.28125, 154.6875, 156.09375, 157.5,
 158.90625, 160.3125, 161.71875, 163.125, 164.53125, 165.9375, 167.34375, 168.75,
 170.15625, 171.5625, 172.96875, 174.375, 175.78125, 177.1875, 178.59375, 180
};

// Use SavedAnglesMEGAServophile[0] as default cuz it is the only one with a full slot
double* usedCalibrationProfile = SavedAnglesMEGAServophile[0];


// ###################
// SETTINGS
// ###################

void* settings[] = {&xAxisInverted, &yAxisInverted, &axisSwapped, &maxSpeed, 
                    &speedMultiplier, &calibrationSpeedMultiplier, &preciseSpeedMultiplier, &offCenterSquaredMultiplier,
                    &offCenterLinearMultiplier, &offCenterAddition, &debug, &maxOptions, &maxCharsPerLine,
                    &currentScrollPosition, &prettyPrintPageDelayMs, &ScrollDelayMs, &ScrollSpacesUntilLoop,
                    &usedCalibrationList, &usedCalibrationListSlot};

// #######################
// End of Variable Setup
// #######################

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
  buildMenu(mainMenuOptions, mainMenuLength);

  // Use the Joystick's Y axis to navigate
  double yOffCenter = YaxisJoystickInfo();
  String yDirection = offCenterToDirection(yOffCenter, 1);

  // Read if button has been pressed (first checks for button on purpose)
  int buttonState = digitalRead(buttonPin);

  if (!buttonState)
  {
    callMenuOption(0);
  }

  // Update Y based on yDirection
  dirUpdateYselected(yDirection, mainMenuLength);

  delay(200);
}


void buildMenu(String menuOptions[], int menuLength)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  // Chooses the smaller of the 2 numbers
  int visibleOptions = min(maxOptions, menuLength);

  for (int i = 0; i < visibleOptions; i++)
  {
    int currentOption = i + firstVisible;
    String menuOption = menuOptions[currentOption];
    display.setTextColor(WHITE);

    if (currentOption == selected)
    {
      menuOption = customScroll(17, menuOption);
      menuOption = String("> ") + menuOption;
      display.setTextColor(BLACK, WHITE);
    }

    // Trim menuOption to fit
    menuOption = menuOption.substring(0, maxCharsPerLine);

    display.println(menuOption);
  }

  drawScrollbar(menuLength);

  display.display();
}

void drawScrollbar(int menuLength)
{
  for(int i = 0; i < SCREEN_HEIGHT / 4 ; i++)
  {
    display.drawLine(SCREEN_WIDTH - 4, i * 4 + 2, SCREEN_WIDTH - 4, i * 4 + 4, WHITE);
  }

  for(int i = 0; i < menuLength; i++)
  {
    if (i == selected)
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

// All Menu Calls are organised and managed here
void callMenuOption(int contextMenu)
{
  // Find the menu the user is currently in
  switch (contextMenu)
  {
    // #############
    // Main Menu
    // #############
    case 0:
    {
      switch (selected)
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
          callSettings();
          break;

        case 4:
          //callAbout();
          break;

        default: 
          Serial.println("Wrong Option Called: /n");
          Serial.print(selected);
          showErrorMessage(105, "Non-Existing Menu Option: " + String(selected));
          break;
      }
      
      // Reset selected
      selected = 0;
      break;
    }
      
    // ###################
    // Calibration Menu
    // ###################
    case 1:
    {
      switch (selected)
      {
        case 0:
          break;
        
        case 1:
          break;

        case 2:
          callLaserCalibration();
          break;

        case 3:
          break;

        case 4:
          break;

        // Go Back
        case 5:
          // Do nothing
          break;

        default:
          Serial.println("Wrong Option Called: /n");
          Serial.print(selected);
          showErrorMessage(105, "Non-Existing Menu Option: " + String(selected));
          break;
      }

      // Reset selected
      selected = 0;
      break;
    }

    // #############
    // Debug Menu
    // #############
    case 2:
    {
      switch (selected)
      {
        case 0:
          callJoystickDebug();
          break;
        case 1:
          showErrorMessage(101, DebugErrorMessage);
          break;
        case 2:
        case 3:
        case 4:
        case 5:
          callErrorMenu();
          break;

        default:
          break;
      }
      // Reset selected
      selected = 0;
      break;      
    }

    // ################
    // Settings Menu
    // ################
    case 3:
    {
      switch (selected)
      {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:        
        default:
          break;
      }      
    }

    // #############
    // About Menu
    // #############
    case 4:
    {
      // prettyPrint();
      // TODO: add a super long string at the top that talks about the creator (me) and the app/project, idk 
      break;      
    }

    default:
    {
      showErrorMessage(104, "Non-Existing Menu: " + String(contextMenu));
      break;      
    }
  }
}

// ####################
// Menu Function Calls
// ####################

void callManualControl()
{
  // Prevent accidental "Go back" reading
  delay(300);

  // Open actual menu
  manualControl();

  // Prevent accidental return to the same option
  delay(300);
}

void callCalibrationMenu()
{
  delay(300);

  calibrationMenu();

  delay(300);
}

bool callPickCalibrationListMenu()
{
  delay(300);

  bool goback = pickCalibrationListMenu();

  delay(300);

  if (debug == true)
  {
    pickCalibrationListMenuDebug();
  }

  return goback;
}

void callPickCalibrationListSlot()
{
  delay(300);

  pickCalibrationListSlot();

  if (debug == true)
  {
    pickCalibrationListSlotMenuDebug();
  }

  delay(300);
}

void callLaserCalibration()
{
  delay(300);

  laserCalibration();

  delay(300);
}

void callDebug()
{
  // Prevent accidental "Go back" reading
  delay(300);

  debugMenu();

  // Prevent accidental return to same option
  delay(300);  
}

void callJoystickDebug()
{
  // Prevent accidental "Go back" reading
  delay(300);

  joystickDebug();

  // Prevent accidental return to same option
  delay(300);  
}

void callErrorMenu()
{
  // Prevent accidental "Go back" reading
  delay(300);

  errorMenu();

  // Prevent accidental return to same option
  delay(300);  
}

void callSettings()
{
  delay(300);

  settingsMenu();

  delay(300);
}

void callAbout()
{
  delay(300);

  About();

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
// call with **0** for X and **1** for Y
String offCenterToDirection(double axisOffCenter, int Axis)
{
  String yDirection[] = {"UP", "DOWN", "NONE"};
  String xDirection[] = {"RIGHT", "LEFT", "NONE"};

  // Set to NONE by default
  int dirSelect = 2;

  // Set up default Deadzone (in case wrong axis is specified)
  double Deadzone = 0.15;

  // Select Deadzone Based on Axis
  // X - 0
  // Y - 1
  if(Axis == 0)
  {
    Deadzone = xDeadzone;
  }
  else if(Axis == 1)
  {
    Deadzone = yDeadzone;
  }
  else
  {
    showErrorMessage(203, "Wrong Axis specified: " + String(Axis));
    return "NONE";
  }

  if (axisOffCenter > Deadzone)
  {
    // "UP" / "RIGHT"
    dirSelect = 0;
  }
  else if (axisOffCenter < -Deadzone)
  {
    // "DOWN" / "LEFT"
    dirSelect = 1;
  } 
  else
  {
    // "NONE"
    dirSelect = 2;
  }

  // Return Based on Axis
  // X - 0
  // Y - 1
  if (Axis == 0)
  {
    return xDirection[dirSelect];
  }
  else if (Axis == 1)
  {
    return yDirection[dirSelect];
  }
  else
  {
    showErrorMessage(203, "Wrong Axis specified: " + String(Axis));
    return "NONE";
  }
}


void updateXvalue(String settingType)
{
  if (settingType == String("BOOL"))
  {
    double xOffCenter = XaxisJoystickInfo();
    String xDirection = offCenterToDirection(xOffCenter, 0);

    if (xDirection == "LEFT")
    {
      *(bool*)settings[selected] = false;
    }
    else if (xDirection == "RIGHT")
    {
      *(bool*)settings[selected] = true;
    }
    else
    {
      // Do nothing
    }
  }
  else if (settingType == String("DOUBLE"))
  {
    double xOffCenter = XaxisJoystickInfo();
    double speed = offCenterToSpeed(xOffCenter); 
    *(double*)settings[selected] += (speed * speedMultiplier * preciseSpeedMultiplier);
  }
  else if (settingType == String("INT"))
  {
    double xOffCenter = XaxisJoystickInfo();
    double speed = offCenterToSpeed(xOffCenter); 
    *(double*)settings[selected] += (speed * speedMultiplier * preciseSpeedMultiplier);
  }
  else
  {
    showErrorMessage(204, "Wrong Setting Type specified: " + String(settingType));
  }
}


void dirUpdateYselected(String yDirection, int menuLength)
{
  // Actually DOWN makes "selected" go UP, because the list starts at 0 and INCREASES as it goes DOWN
  if (yDirection == "DOWN")
  {
    selected++;
  }
  // UP makes "selected" go DOWN, because the list ends at menu[menuLength - 1] and DECREASES as it goes UP
  else if (yDirection == "UP")
  {
    selected--;
  }
  else if (yDirection == "NONE")
  {
    // Do nothing
  }

  // Flatten selected to normal range
  fixSelected(menuLength);

  // Move the whole menu UP/DOWN one option
  if (selected - firstVisible > (maxOptions - 1))
  {
    firstVisible++;
  }
  else if (selected - firstVisible < 0)
  {
    firstVisible--;
  }
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
void joystickDebug()
{
  // Immitate a new loop() function
  while (true)
  {
    double xOffCenter = XaxisJoystickInfo();
    double yOffCenter = YaxisJoystickInfo();

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
double offCenterToSpeed(double xOffCenter)
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

// Converts default (digital) angle calibrated one using a saved calibration profile List
double digitalAngleToCalibrated(double angle, int ListSize, double* calibratedAngleList, double* calibratedAngleListReference)
{
  // Checks each angle interval
  for (int i = 0; i < ListSize - 1; i++)
  {
    // check if angle is within that interval 
    if (angle >= calibratedAngleListReference[i] && angle <= calibratedAngleListReference[i + 1])
    {
      // THIS DOESNT WORK CUZ IT REMOVES DECIMALS (SAD)
      //double calibratedAngle = map(angle, calibratedAngleListReference[i], calibratedAngleListReference[i + 1], calibratedAngleList[usedCalibrationListSlot][i], calibratedAngleList[usedCalibrationListSlot][i + 1]);

      // *Digital* interval length
      double originIntervalDiff = calibratedAngleListReference[i + 1] - calibratedAngleListReference[i];
      
      // Get distance to turn it into percentage
      double distanceToIntervalStart = angle - calibratedAngleListReference[i];

      // Gets the percentage from the original interval to apply it in the calibrated one
      double percentageOrigin = distanceToIntervalStart / originIntervalDiff * 100;

      // Get the calibrated interval length
      double calibratedIntervalDiff = calibratedAngleList[i + 1] - calibratedAngleList[i];

      // Apply the extracted percentage to the calibrated interval
      double calibratedAngleAddition = calibratedIntervalDiff / 100 * percentageOrigin;
      double calibratedAngle = calibratedAngleList[i] + calibratedAngleAddition;

      if (debug == true)
      {
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        display.setTextColor(WHITE);

        display.print("Angle: ");
        display.setTextColor(BLACK,WHITE);
        display.println(angle);

        display.setTextColor(WHITE);
        display.print("InterDiff: ");
        display.setTextColor(BLACK,WHITE);
        display.println(originIntervalDiff);

        display.setTextColor(WHITE);
        display.print("distToStart: ");
        display.setTextColor(BLACK,WHITE);
        display.println(distanceToIntervalStart);

        display.setTextColor(WHITE);
        display.print("percentage: "); 
        display.setTextColor(BLACK,WHITE);
        display.println(percentageOrigin);

        display.setTextColor(WHITE);
        display.print("calDiff: ");
        display.setTextColor(BLACK,WHITE);
        display.println(calibratedIntervalDiff);

        display.setTextColor(WHITE);
        display.print("AngleAdd: ");
        display.setTextColor(BLACK,WHITE);
        display.println(calibratedAngleAddition);

        display.setTextColor(WHITE);
        display.print("calAngle: ");
        display.setTextColor(BLACK,WHITE);
        display.println(calibratedAngle);

        display.display();

        delay(1500);
      }

      return calibratedAngle;
    }
  }
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

// update Min and Max Angles based on profile preferences
// Uses calibration profile's first and last index for the new Bound angle
void updateMinMaxAngles()
{
  minAngle = usedCalibrationProfile[0];

  int length = sizeof(*usedCalibrationProfile) / sizeof(usedCalibrationProfile[0]);

  maxAngle = usedCalibrationProfile[length - 1];

  if (debug == true)
  {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setTextColor(WHITE);
    display.print("minAngle: ");
    display.setTextColor(BLACK,WHITE);
    display.println(minAngle); 

    display.setTextColor(WHITE);
    display.print("maxAngle: ");
    display.setTextColor(BLACK,WHITE);
    display.println(maxAngle);

    display.display();

    delay(2000);
  }
}

void manualControl() 
{
  // update Min and Max Angles based on profile preferences
  updateMinMaxAngles();

  // Immitate a new loop() function
  while (true)
  {
    double xOffCenter = XaxisJoystickInfo();
    double angleChange = offCenterToSpeed(xOffCenter);
    angle += angleChange * speedMultiplier;

    angle = fixServoAngle(angle, minAngle, maxAngle);

    // calibrate angle before sending it to Servo.write()
    double calibratedAngle = digitalAngleToCalibrated(angle, SavedAnglesMEGAServophileLength, SavedAnglesMEGAServophile[usedCalibrationListSlot], SavedAnglesMEGAServophileReference);
    
    TestServo.write(calibratedAngle);

    display.clearDisplay();

    display.setCursor(0,0);
    display.setTextSize(1);

    String showCurrentAngleDigital = String("Angle (Digital): ") + angle;
    String showCurrentAngleReal = String("Angle (Real): ") + calibratedAngle;
    String showCurrentSpeed = String("Current Speed: ") + angleChange;

    display.println(showCurrentAngleDigital);
    display.println(showCurrentAngleReal);
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

void calibrationMenu()
{
  selected = 0;
  firstVisible = 0;

  while(true)
  {
    buildMenu(calibrationMenuOptions, calibrationMenuLength);

    // Use the Joystick's Y axis to navigate
    double yOffCenter = YaxisJoystickInfo();

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

      // Call menu option if it's not the last one
      callMenuOption(1);
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, calibrationMenuLength);

    delay(200);
  }
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


void pickCalibrationListMenuDebug()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);

  String Message = String("List Picked: ") + usedCalibrationList;

  display.println(Message);
  display.display();

  delay(1000);
}

void pickCalibrationListSlotMenuDebug()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);

  String Message = String("Slot Picked: ") + usedCalibrationListSlot;

  display.println(Message);
  display.display();

  delay(1000);
}

bool pickCalibrationListMenu()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;
  
  while (true)
  {
    buildMenu(pickCalibrationListMenuOptions, pickCalibrationListMenuLength);

    // Use the Joystick's Y axis to navigate
    double yOffCenter = YaxisJoystickInfo();
    String yDirection = offCenterToDirection(yOffCenter, 1);

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Keep "Go Back" last so it always works
      if (selected == pickCalibrationListMenuLength - 1)
      {
        // make goback = true
        return true;
      }
      else
      {
        // selected corresponds to the number of each list 0 - basic, 1 - accurate, 2 - servophile, 3 - super servophile, 4 - MEGA servophile, 5 - Overkill Servophile
        usedCalibrationList = selected;
        return false;
      }
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, pickCalibrationListMenuLength);

    delay(200);
  }
}


void pickCalibrationListSlot()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;

  while (true)
  {
    buildMenu(pickCalibrationSlotMenuOptions, pickCalibrationSlotMenuLength);

    // Use the Joystick's Y axis to navigate
    double yOffCenter = YaxisJoystickInfo();
    String yDirection = offCenterToDirection(yOffCenter, 1);

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Check if "Go Back" was selected
      if (selected == pickCalibrationSlotMenuLength - 1)
      {
        // "return" by going back to picking the list type
        pickCalibrationListMenu();
      }
      else
      {
        // selected corresponds to the number of each slot (from 0 to 6)
        usedCalibrationListSlot = selected;
        return;  
      }
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, pickCalibrationSlotMenuLength);

    delay(200);
  }
}

// Uses a laser strapped to the horn of the servo to calculate servo angle using tg  
void laserCalibration()
{
  // Pick Servo Angle Accuracy (more samples is bettter)
  // no vars needed cuz it uses global scale ones
  bool goback = callPickCalibrationListMenu();
  
  if (goback)
  {
    // Go back
    return;
  }

  int ListID = usedCalibrationList;
  
  // [SLOT] Picker
  callPickCalibrationListSlot();
  int slot = usedCalibrationListSlot;

  // Reference, Picked and Distance Lists all have the same size
  int ListSize = 0;

  // Initialize empty pointers so we don't declare and lose them in the switch (case)  
  double* pickedAngleList = nullptr;
  double* referenceAngleList = nullptr;

  switch (ListID)
  {
    case 0:
    {
      ListSize = SavedAnglesBasicLength;
      pickedAngleList = &SavedAnglesBasic[slot][0]; // Call [0] to get the array address
      referenceAngleList = &SavedAnglesBasicReference[0];
      break;      
    }

    case 1:
    {
      ListSize = SavedAnglesAccurateLength;
      pickedAngleList = &SavedAnglesAccurate[slot][0];
      referenceAngleList = &SavedAnglesAccurateReference[0];
      break;
    }

    case 2:
    {
      ListSize = SavedAnglesServophileLength;
      pickedAngleList = &SavedAnglesServophile[slot][0];
      referenceAngleList = &SavedAnglesServophileReference[0];
      break;      
    }

    default:
      break;
  }
  
  // Saved Servo Variables
  double startingDistance = 0;
  double Distances[ListSize];

  // Start up the Script for getting the Laser to wall distance
  startingDistance = getLaserToWallDistance();

  // Wait a bit to prevent accidental clicks 
  delay(300);

  // distance from laser dot to marking on the wall (in cm)
  double distance = 0;

  for (int i = 0; i < ListSize; i++)
  {
    // RESET distance
    distance = 0;

    // Picks current angle based on used list
    double servoAngle = referenceAngleList[i];

    // Set the servo to the corresponding angle 
    TestServo.write(servoAngle);

    // wait till user clicks
    while (true)
    {
      double yOffCenter = YaxisJoystickInfo();
      double speed = offCenterToSpeed(yOffCenter); 
      distance += (speed * speedMultiplier * calibrationSpeedMultiplier);

      String Message = String("Input ") + servoAngle + ("deg \n distance");
      
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

  display.clearDisplay();

  // Make a String for displaying Distances
  String distancesMessage = String("startingDistance: ") + String(startingDistance);
  distancesMessage += "Distances: ";

  // Make a String for displaying Angles
  String anglesMessage = String("Angles Saved: ");

  // Use the Distances[] array to produce an Angles[] array
  for (int i = 0; i < ListSize; i++)
  {
    // Check if it's the first index
    if (i == 0)
    {
      // There's nothing you can add to the first one (if you try to, you will get an error)
      pickedAngleList[i] = calcDistanceToAngle(startingDistance, Distances[i]);
    }
    else
    {
      // Each angle is the sum of all the angles before it + the current distance converted to degrees
      pickedAngleList[i] = calcDistanceToAngle(startingDistance, Distances[i]) + pickedAngleList[i - 1];
    }

    // Add to Distances Message
    // referenceAngle: distance
    distancesMessage += String(referenceAngleList[i]);
    distancesMessage += String(": ");
    distancesMessage += String(Distances[i]);
    distancesMessage += String("; ");

    // Add to Angles Message
    // referenceAngle: calibratedAngle
    anglesMessage += String(referenceAngleList[i]); 
    anglesMessage += String(": ");
    anglesMessage += String(pickedAngleList[i]); 
    anglesMessage += String("; ");
  }

  // Print Message in pages
  prettyPrint(distancesMessage, 5, 0, 1, 20, 7);

  display.clearDisplay();

  // Print Message in pages
  prettyPrint(anglesMessage, 5, 0, 1, 20, 7);
}

void displayDistanceAndSpeedResult(double inputDistance, double inputSpeed, String TopMessage)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

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
  display.setTextColor(WHITE);

  // Initialize distance
  double distance = 0;

  while (true)
  {
    // Read joystick for speed
    double yOffCenter = YaxisJoystickInfo();
    double speed = offCenterToSpeed(yOffCenter); 
    distance += (speed * speedMultiplier * calibrationSpeedMultiplier);

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


void About()
{

}

void showErrorMessage(int ErrorID, String errorMessage)
{
  delay(300);

  // Static part
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(10,10);

  // Show Error ID
  display.print("ERROR:" + String(ErrorID));

  /// Static part
  display.setTextSize(1);
  display.setCursor(6, 30);
  
  // Call custom print function for equally long lines
  prettyPrint(errorMessage, 6, 30, 1, 19, 3);
}

void prettyPrint(String Message, int x, int y, int textSize, int maxChars, int maxLines)
{
  // removed display.clearDisplay() so it can work with ERROR: errorID at the top without hardcoding it into this function 
  display.setTextColor(WHITE);
  display.setTextSize(textSize);
  display.setCursor(x,y);

  String messageFragments[100] = {};

  int charCounter = 0;
  int lineCounter = 0;
  int pageCounter = 0;

  int Lines = maxLines;
  bool specialSymbol = false;

  // Go through each char in the message to separate it into fragments
  for (char currentChar : Message)
  {
    // Adds special commands and symbols using "/"
    if (specialSymbol)
    {
      // Reset State
      specialSymbol = false;
      
      // Check for new line command
      if (currentChar == 'n')
      {
        // start a new line
        charCounter = 0;
        lineCounter++;

        continue;
      }

      // else: just continue
    }
    else
    {
      if (currentChar == '/')
      {
        specialSymbol = true;
        continue;
      }      
    }


    // add char to current fragment
    messageFragments[pageCounter * Lines + lineCounter] += currentChar;

    // Count char to this line
    charCounter++;

    // check if message got to the needed size
    if (charCounter == maxChars)
    {
      // start a new line
      charCounter = 0;
      lineCounter++;
    }

    // after first page linecounter must fill the whole page to get to the next page
    if (lineCounter >= Lines)
    {
      // Check if the current page is the first one
      if (pageCounter == 0)
      {
        // Make the switch from partial to full page
        Lines = maxOptions;
      }

      // go to next page
      lineCounter = 0;
      pageCounter++;
    }
  }

  // **i** is the display page counter
  for (int i = 0; i < pageCounter + 1; i++)
  { 
    // The first time it runs there are less lines available to the screen. that's why we have this check
    if (i == 0)
    {
      // **j** is the current line counter
      // use maxLines for the first iteration of the cycle
      for (int j = 0; j < maxLines; j++)
      {
        display.setCursor(x, y + j * 8);

        // Print current line
        // No i * maxLines cuz we know i = 0
        display.println(messageFragments[j]);
      }

      // Display page and clear buffer
      display.display();
      display.setCursor(x, 0);

      // wait 7.5 seconds and display next page
      // allow user to skip to next page on click
      for (int delayCounter = 0; delayCounter < prettyPrintPageDelayMs / 50; delayCounter++)
      {
        // Go to next page if clicked
        int buttonState = digitalRead(buttonPin);
        if (!buttonState)
        {
          // Go to next page
          delay(300);
          break;
        }
        
        // divide the big delay into 50ms chunks
        delay(50);
      }
    }
    else
    {
      display.clearDisplay();

      // **j** is the current line counter
      for (int j = 0; j < maxOptions; j++)
      {
        display.setCursor(x, j * 8);
        // Print current line
        display.println(messageFragments[i * maxOptions + j]);
      }

      // Display page and clear buffer
      display.display();
      display.clearDisplay();
      display.setCursor(x, 0);

      // wait 7.5 seconds and display next page
      // allow user to skip to next page on click
      for (int delayCounter = 0; delayCounter < prettyPrintPageDelayMs / 50; delayCounter++)
      {
        // Go to next page if clicked
        int buttonState = digitalRead(buttonPin);
        if (!buttonState)
        {
          delay(300);
          break;
        }
        
        // divide the big delay into 50ms chunks
        delay(50);
      }     
    }
  }
}

String customScroll(int lineLength, String Text)
{
  int charCounter = 0;

  // Separate Text into chars
  char rawText[100] = {};

  // Find out how long the line is
  for (char currentChar : Text)
  {
    // Add current char to text
    rawText[charCounter] = currentChar;

    charCounter++;
  }

  // Stop if there is no need for scroll
  if (charCounter > lineLength)
  {
    String returnText = "";

    // Continues from lastScrollPosition
    for (int i = 0; i < lineLength; i++)
    {
      int currentIndex = i + currentScrollPosition;

      // Check if the message has finished and loop it again
      while (currentIndex > charCounter + ScrollSpacesUntilLoop)
      {
        currentIndex -= (charCounter + 5);
      }

      returnText += rawText[currentIndex];
    }

    // This helps for next function call
    currentScrollPosition++;

    // add delay
    // allow user to leave delay on movement
    for (int delayCounter = 0; delayCounter < (ScrollDelayMs / 10); delayCounter++)
    {
      // Break delay if there is movement
      double yOffCenter = YaxisJoystickInfo();
      String yDirection = offCenterToDirection(yOffCenter, 1);
      
      // Check for Y-axis movement
      if (yDirection != "NONE")
      {
        // Reset Scroll Position
        currentScrollPosition = 0;

        break;
      }
      
      // divide the big delay into 10ms chunks
      delay(10);
    } 

    return returnText;
  }
  else
    return Text;
}

double XaxisJoystickInfo()
{
  int xReading = analogRead(xPin);    

  // Try to swap axis
  if (axisSwapped)
  {
    // Swap fundamental joystick reading
    xReading = analogRead(yPin);
  }

  // Calculate reading to offCenter
  double xOffCenter = calculateOffCenter(xReading); 
  xOffCenter = tryInvertAxis(0, xOffCenter);

  return xOffCenter;
}

double YaxisJoystickInfo()
{
  int yReading = analogRead(yPin);

  // Try to swap axis
  if (axisSwapped)
  {
    // Swap fundamental joystick reading
    yReading = analogRead(xPin);
  }

  // Calculate reading to offCenter
  double yOffCenter = calculateOffCenter(yReading); 
  yOffCenter = tryInvertAxis(1, yOffCenter);

  return yOffCenter;
}

// Made to use only one axis at a time so you dont need both when you only need one
double tryInvertAxis(int Axis, double axisOffcenter)
{
  bool axisInverted = false;

  // X axis
  if (Axis == 0)
  {
    axisInverted = xAxisInverted;
  }
  // Y axis
  else if (Axis == 1)
  {
    axisInverted = yAxisInverted;
  }
  else
  {
    showErrorMessage(203, "Wrong Axis specified: " + String(Axis));
  }

  if (axisInverted)
  {
    axisOffcenter = -axisOffcenter;
  }

  return axisOffcenter;
}

void debugMenu()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;
  
  while (true)
  {
    buildMenu(DebugMenuOptions, DebugMenuLength);

    // Use the Joystick's Y axis to navigate
    double yOffCenter = YaxisJoystickInfo();
    String yDirection = offCenterToDirection(yOffCenter, 1);

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Keep "Go Back" last so this always works
      if (selected == (DebugMenuLength - 1))
      {
        // Go Back
        return;
      }
      callMenuOption(2);
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, DebugMenuLength);

    delay(200);
  }
}

void settingsMenu()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;
  
  while (true)
  {
    buildMenu(settingsMenuOptions, settingsMenuLength);

    // Use the Joystick's Y axis to navigate
    double yOffCenter = YaxisJoystickInfo();
    String yDirection = offCenterToDirection(yOffCenter, 1);

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Keep "Go Back" last so this always works
      if (selected == (settingsMenuLength - 1))
      {
        // Go Back
        return;
      }
      updateSetting();

      // TODO: Temporarily change the selected menuOption to its value form;
      // >>    [value]    <<
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, settingsMenuLength);

    delay(200);
  }
}

void updateSetting()
{
  String settingType = "NONE";

  // Find setting Type
  switch (selected)
  {
    case 0:
    case 1:
    case 2:
      settingType = "BOOL";
      break;

    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      settingType = "DOUBLE";
      break;

    case 10:
      settingType = "BOOL";
      break;

    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
      settingType = "INT";
      break;

    default:
      break;
  }

  updateXvalue(settingType);
}

void errorMenu()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;
  
  while (true)
  {
    buildMenu(ErrorTypes, ErrorTypesLength);

    // Use the Joystick's Y axis to navigate
    double yOffCenter = YaxisJoystickInfo();
    String yDirection = offCenterToDirection(yOffCenter, 1);

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Keep "Go Back" last so this always works
      if (selected == (ErrorTypesLength - 1))
      {
        // Go Back
        return;
      }
      simulateError();
    }

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, ErrorTypesLength);

    delay(200);
  }
}

// Simulates all possible errors across the code
// uses "selected" so no input needed
void simulateError()
{
  switch (selected)
  {
    case 0:
      // Show Error 101: Dummy Error for Debugging
      showErrorMessage(101, DebugErrorMessage);
      break;

    case 1:
      // Force Error 104: Non-Existing Menu
      callMenuOption(100);
      break;

    case 2:
      // Force Error 105: Non-Existing Menu Option
      selected = 100;
      callMenuOption(0);
      break;

    case 3:
      // Force Error 106: Unkown Error Specified
      selected = 100;
      simulateError();
      break;

    case 4:
      // Force Error 203: Wrong Axis Specified
      offCenterToDirection(0, 3);
      break;

    default:
      showErrorMessage(106, "Unkown Error Specified: " + String(selected));
      break;
  }
}