#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <Math.h>

const String version = "1.0.0";

// Define Screen Parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Timers
// currentTime = millis();
unsigned long previousTime = 0;
unsigned long previousDisplayTime = 0;
unsigned long previousJoystickTime = 0;
unsigned long previousClickTime = 0;
unsigned long previousDirectionTime = 0;

// The most important delay of them all
// Makes the app feel more polished and helps evade fake "Go Back", "Select" or "Next" clicks
// keep this low - between 200 and 400 ms
int actionDelay = 300;

int displayDelayMs = 50;
int joystickDelayMs = 100;
int clickDelayMs = 20;
int selectedDelayMs = 200;
int loopDelayMs = 10; // Delay every loop (handle cautiously)


// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Debugger Option (Change to enable debugging)
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
bool debug = false;

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Calibration Option (Change to enable joystick)
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
bool joystickCalibrate = true;

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Reset Settings/Profiles (Change to turn NVS on / off)
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
bool resetSavedSettings = false;
bool resetSavedProfiles = false;

// ##############
// Hardware Pins
// ##############
int servoPin = 38;
int yPin = 1;
int xPin = 2;
int buttonPin = 12;

// #################
// Servo Variables
// #################

float savedPulseWidths[3] = {1000, 1500, 2000};

// Has 3 types: 
// [0]: 90deg positional servo
// [1]: 180deg positional servo
// [2]: 360deg rotating servo
int servoType = 1;
int servoTypeAngles[] = {90, 180, 360};

double angle = 90; // Servo Angle
double minAngle = 0; // Using double for future servo calibration function
double maxAngle = 180; // Using double for future servo calibration function
// Speed of angle change
// double speed = 0; 

// ###################
// Joystick Variables
// ###################

double xDeadzone = 0.15;
double yDeadzone = 0.15;

// Joystick offCenter Calibration Values
double xCalibration = 0;
double yCalibration = 0;

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
double speedMultiplier = 0.5; // Change this when changing the loopDelay to keep the same ratio without fine tuning the whole speed function again

// Finer control when selecting distances (recommended to keep below 0.5)
double calibrationSpeedMultiplier = 0.1;

// Precice control for specific cases
double preciseSpeedMultiplier = 0.1;

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
bool enableFocusArrow = true;

int prettyPrintPageDelayMs = 7500;
int ScrollDelayMs = 200;
int ScrollSpacesUntilLoop = 4;

// ######################
// Menu Options Arrays
// ######################

// Main Menu Options
String mainMenuOptions[] = {"Manual Control", "Calibration", "Select Servo Type", "Debug", "Settings", "About"};
int mainMenuLength = sizeof(mainMenuOptions) / sizeof(mainMenuOptions[0]);

// Servo Type Options
String servoTypeOptions[] = {"90 deg", "180 deg", "360 deg", "Go Back"};
int servoTypeLength = sizeof(servoTypeOptions) / sizeof(servoTypeOptions[0]);

// Calibration Menu Options
String calibrationMenuOptions[] = {"Test Sweep", "Pulse Width Calibration", "C.E.A. LASER Calibration", "Joystick Calibration", "Pick Servo Profile", "Go Back"};
int calibrationMenuLength = sizeof(calibrationMenuOptions) / sizeof(calibrationMenuOptions[0]);

// Calibration List Picker Options
String pickCalibrationListMenuOptions[] = {"Basic", "Accurate", "Servophile", "Super Servophile", "MEGA Servophile", "OVERKILL Servophile", "Go Back"};
int pickCalibrationListMenuLength = sizeof(pickCalibrationListMenuOptions) / sizeof(pickCalibrationListMenuOptions[0]);

// Slot Picker Options
String pickCalibrationSlotMenuOptions[] = {"Slot 0", "Slot 1", "Slot 2", "Slot 3", "Slot 4", "Slot 5", "Go Back"};
int pickCalibrationSlotMenuLength = sizeof(pickCalibrationSlotMenuOptions) / sizeof(pickCalibrationSlotMenuOptions[0]);

// Debug Menu Options
String DebugMenuOptions[] = {"Debug Joystick", "Debug Joystick Calibration", "Debug Calibration Angles", "Debug Error Messages", "Debug Custom Scroll",
                             "Debug Settings Variable Updates", "Simulate Errors", "Go Back"};
int DebugMenuLength = sizeof(DebugMenuOptions) / sizeof(DebugMenuOptions[0]);

// Error Message Used for Debugging
const char DebugErrorMessage[] PROGMEM  = "[Intro]/nWe lost everything/nWe had to pay the price/nYeah, we lost everything/nWe had to pay the price/n/n/n[Verse 1]/nI saw in you what life was missing/nYou lit a flame that consumed my hate/nI'm not one for reminiscing but/nI'd trade it all for your sweet embrace/n/n/n[Interlude]/nYeah, 'cause we lost everything/nWe had to pay the price/n/n/n[Verse 2]/nThere's a canvas with two faces/nOf fallen angels who loved and lost/nIt was a passion for the ages/nAnd in the end guess we paid the cost/n/n/n[Chorus]/nA thing of beauty, I know/nWill never fade away/nWhat you did to me, I know/nSaid what you had to say/nBut a thing of beauty/n/n/n[Post-Chorus]/nWill never fade away/nWill never fade away/nWill never fade away/n/n/n[Verse 3]/nI see your eyes, I know you see me/nYou're like a ghost how you're everywhere/nI am your demon never leaving/nA metal soul of rage and fear/n/n/n[Pre-Chorus]/nThat one thing that changed it all/nThat one sin that caused the fall/n/n/n[Chorus]/nA thing of beauty, I know/nWill never fade away/nWhat you did to me, I know/nSaid what you had to say/nBut a thing of beauty, I know/nWill never fade away/nAnd I'll do my duty, I know/nSomehow I'll find a way/nBut a thing of beauty/nWill never fade away/nAnd I'll do my duty/n/n/n[Post-Chorus]/nYeah, we'll never fade away/nWe'll never fade away/nWe'll never fade away/nWe'll never fade away";

enum SettingType
{
  BOOL,
  INT,
  DOUBLE
};

// defaultValue uses double because void* only works as a pointer = can't store a const like 300 or FALSE
struct Setting
{
  const char* name;
  SettingType type;
  void* value;
  double defaultValue;
};

// Error ID List
String ErrorTypes[] = {"Error 101: Dummy Error for Debugging", "Error 104: Non-Existing Menu", "Error 105: Non-Existing Menu Option", "Error 106: Non-Existing Servo Profile", "Error 108: Non-Existing Setting", "Error 109: Unkown Error Specified", "Error 203: Wrong Axis Specified", "Error 204: Wrong Setting Type Specified", "Error 305: Unsupported Servo Type", "Go Back"};
int ErrorTypesLength = sizeof(ErrorTypes) / sizeof(ErrorTypes[0]);

// Added this to make a function work
bool simulateError106 = false;

// settings[] can be found ~200 lines lower (the bottom of the variable setup)

// ########################
// Servo Profile Preferences:
// ########################
// Profile References are:
// [0] = 90° servo
// [1] = 180° servo
// ########################

// ID of the current User Profile
// Uses 0 for Basic, 1 for accurate, 2 for servophile (3 for super servophile)
int usedCalibrationList = 4;
int usedCalibrationListSlot = 0;

// Actual pointer reaching the profile address
// Initialize empty pointers so we don't declare and lose them in functions  
double* pickedAngleList = nullptr;
double* referenceAngleList = nullptr;

// Distance, Picked and Reference Lists all have the same size
int ListSize = 0;

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
// [0] = 90° servo
// [1] = 180° servo
double SavedAnglesBasicReference[2][5] =
{
  {0, 22.5, 45, 67.5, 90},
  {0, 45, 90, 135, 180}
};

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
double SavedAnglesAccurateReference[2][9] =
{
  {0, 11.25, 22.5, 33.75, 45, 56.25, 67.5, 78.75, 90},
  {0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5, 180}
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
double SavedAnglesServophileReference[2][17] =
{
  {
    0,
    5.625, 11.25, 16.875, 22.5,
    28.125, 33.75, 39.375, 45,
    50.625, 56.25, 61.875, 67.5,
    73.125, 78.75, 84.375, 90
  },

  {
    0,
    11.25, 22.5, 33.75, 45,
    56.25, 67.5, 78.75, 90,
    101.25, 112.5, 123.75, 135,
    146.25, 157.5, 168.75, 180
  }
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
double SavedAnglesSuperServophileReference[2][33] =
{
  {
    0,
    2.8125, 5.625, 8.4375, 11.25,
    14.0625, 16.875, 19.6875, 22.5,
    25.3125, 28.125, 30.9375, 33.75,
    36.5625, 39.375, 42.1875, 45,
    47.8125, 50.625, 53.4375, 56.25,
    59.0625, 61.875, 64.6875, 67.5,
    70.3125, 73.125, 75.9375, 78.75,
    81.5625, 84.375, 87.1875, 90
  },

  {
    0,
    5.625, 11.25, 16.875, 22.5,
    28.125, 33.75, 39.375, 45,
    50.625, 56.25, 61.875, 67.5,
    73.125, 78.75, 84.375, 90,
    95.625, 101.25, 106.875, 112.5,
    118.125, 123.75, 129.375, 135,
    140.625, 146.25, 151.875, 157.5,
    163.125, 168.75, 174.375, 180
  }
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

// MEGA Servophile Profile Reference
double SavedAnglesMEGAServophileReference[2][65] =
{
  {
    0,
    1.40625, 2.8125, 4.21875, 5.625,
    7.03125, 8.4375, 9.84375, 11.25,
    12.65625, 14.0625, 15.46875, 16.875,
    18.28125, 19.6875, 21.09375, 22.5,
    23.90625, 25.3125, 26.71875, 28.125,
    29.53125, 30.9375, 32.34375, 33.75,
    35.15625, 36.5625, 37.96875, 39.375,
    40.78125, 42.1875, 43.59375, 45,
    46.40625, 47.8125, 49.21875, 50.625,
    52.03125, 53.4375, 54.84375, 56.25,
    57.65625, 59.0625, 60.46875, 61.875,
    63.28125, 64.6875, 66.09375, 67.5,
    68.90625, 70.3125, 71.71875, 73.125,
    74.53125, 75.9375, 77.34375, 78.75,
    80.15625, 81.5625, 82.96875, 84.375,
    85.78125, 87.1875, 88.59375, 90
  },

  {
    0,
    2.8125, 5.625, 8.4375, 11.25,
    14.0625, 16.875, 19.6875, 22.5,
    25.3125, 28.125, 30.9375, 33.75,
    36.5625, 39.375, 42.1875, 45,
    47.8125, 50.625, 53.4375, 56.25,
    59.0625, 61.875, 64.6875, 67.5,
    70.3125, 73.125, 75.9375, 78.75,
    81.5625, 84.375, 87.1875, 90,
    92.8125, 95.625, 98.4375, 101.25,
    104.0625, 106.875, 109.6875, 112.5,
    115.3125, 118.125, 120.9375, 123.75,
    126.5625, 129.375, 132.1875, 135,
    137.8125, 140.625, 143.4375, 146.25,
    149.0625, 151.875, 154.6875, 157.5,
    160.3125, 163.125, 165.9375, 168.75,
    171.5625, 174.375, 177.1875, 180
  }
};

// ################################
// OVERKILL Servophile Profile:
// ################################

double SavedAnglesOVERKILLServophile[6][129] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                                               {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

// Length of the SavedAnglesMEGAServophile List (129)
const int SavedAnglesOVERKILLServophileLength = sizeof(SavedAnglesOVERKILLServophile[0]) / sizeof(SavedAnglesOVERKILLServophile[0][0]);

// OVERKILL Servophile Profile Reference
double SavedAnglesOVERKILLServophileReference[2][129] =
{
  {
    0,
    0.703125, 1.40625, 2.109375, 2.8125, 3.515625, 4.21875, 4.921875,
    5.625, 6.328125, 7.03125, 7.734375, 8.4375, 9.140625, 9.84375,
    10.546875, 11.25, 11.953125, 12.65625, 13.359375, 14.0625,
    14.765625, 15.46875, 16.171875, 16.875, 17.578125, 18.28125,
    18.984375, 19.6875, 20.390625, 21.09375, 21.796875, 22.5,
    23.203125, 23.90625, 24.609375, 25.3125, 26.015625, 26.71875,
    27.421875, 28.125, 28.828125, 29.53125, 30.234375, 30.9375,
    31.640625, 32.34375, 33.046875, 33.75, 34.453125, 35.15625,
    35.859375, 36.5625, 37.265625, 37.96875, 38.671875, 39.375,
    40.078125, 40.78125, 41.484375, 42.1875, 42.890625, 43.59375,
    44.296875, 45, 45.703125, 46.40625, 47.109375, 47.8125,
    48.515625, 49.21875, 49.921875, 50.625, 51.328125, 52.03125,
    52.734375, 53.4375, 54.140625, 54.84375, 55.546875, 56.25,
    56.953125, 57.65625, 58.359375, 59.0625, 59.765625, 60.46875,
    61.171875, 61.875, 62.578125, 63.28125, 63.984375, 64.6875,
    65.390625, 66.09375, 66.796875, 67.5, 68.203125, 68.90625,
    69.609375, 70.3125, 71.015625, 71.71875, 72.421875, 73.125,
    73.828125, 74.53125, 75.234375, 75.9375, 76.640625, 77.34375,
    78.046875, 78.75, 79.453125, 80.15625, 80.859375, 81.5625,
    82.265625, 82.96875, 83.671875, 84.375, 85.078125, 85.78125,
    86.484375, 87.1875, 87.890625, 88.59375, 89.296875, 90
  },

  {
    0,
    1.40625, 2.8125, 4.21875, 5.625, 7.03125, 8.4375, 9.84375,
    11.25, 12.65625, 14.0625, 15.46875, 16.875, 18.28125, 19.6875,
    21.09375, 22.5, 23.90625, 25.3125, 26.71875, 28.125, 29.53125,
    30.9375, 32.34375, 33.75, 35.15625, 36.5625, 37.96875, 39.375,
    40.78125, 42.1875, 43.59375, 45, 46.40625, 47.8125, 49.21875,
    50.625, 52.03125, 53.4375, 54.84375, 56.25, 57.65625, 59.0625,
    60.46875, 61.875, 63.28125, 64.6875, 66.09375, 67.5, 68.90625,
    70.3125, 71.71875, 73.125, 74.53125, 75.9375, 77.34375, 78.75,
    80.15625, 81.5625, 82.96875, 84.375, 85.78125, 87.1875, 88.59375,
    90, 91.40625, 92.8125, 94.21875, 95.625, 97.03125, 98.4375,
    99.84375, 101.25, 102.65625, 104.0625, 105.46875, 106.875,
    108.28125, 109.6875, 111.09375, 112.5, 113.90625, 115.3125,
    116.71875, 118.125, 119.53125, 120.9375, 122.34375, 123.75,
    125.15625, 126.5625, 127.96875, 129.375, 130.78125, 132.1875,
    133.59375, 135, 136.40625, 137.8125, 139.21875, 140.625,
    142.03125, 143.4375, 144.84375, 146.25, 147.65625, 149.0625,
    150.46875, 151.875, 153.28125, 154.6875, 156.09375, 157.5,
    158.90625, 160.3125, 161.71875, 163.125, 164.53125, 165.9375,
    167.34375, 168.75, 170.15625, 171.5625, 172.96875, 174.375,
    175.78125, 177.1875, 178.59375, 180
  }
};


// ###################
// SETTINGS
// ###################

// added this here to fix a bug
String settingsMessage = "VALUE";

Setting settings[] = 
{
  // =========================
  // Timers / Delays
  // =========================
  {"actionDelay",                 INT,    &actionDelay,                300},
  {"displayDelayMs",              INT,    &displayDelayMs,             50},
  {"joystickDelayMs",             INT,    &joystickDelayMs,            100},
  {"clickDelayMs",                INT,    &clickDelayMs,               20},
  {"selectedDelayMs",             INT,    &selectedDelayMs,            200},
  {"loopDelayMs",                 INT,    &loopDelayMs,                10},

  // =========================
  // Hardware Pins
  // =========================
  {"servoPin",                    INT,    &servoPin,                   38},
  {"xPin",                        INT,    &xPin,                       2},
  {"yPin",                        INT,    &yPin,                       1},
  {"buttonPin",                   INT,    &buttonPin,                  12},

  // =========================
  // Servo
  // =========================
  {"servoType",                   INT,    &servoType,                  1},
  {"angle",                       DOUBLE, &angle,                      90},
  {"minAngle",                    DOUBLE, &minAngle,                   0},
  {"maxAngle",                    DOUBLE, &maxAngle,                   180},

  // =========================
  // Joystick Input
  // =========================
  {"xDeadzone",                   DOUBLE, &xDeadzone,                  0.15},
  {"yDeadzone",                   DOUBLE, &yDeadzone,                  0.15},
  {"xCalibration",                DOUBLE, &xCalibration,               0},
  {"yCalibration",                DOUBLE, &yCalibration,               0},
  {"joystickScaleDivider",        DOUBLE, &joystickScaleDivider,       2048},

  // =========================
  // Axis Inversion / Swapping
  // =========================
  {"xAxisInverted",               BOOL,   &xAxisInverted,              true},
  {"yAxisInverted",               BOOL,   &yAxisInverted,              true},
  {"axisSwapped",                 BOOL,   &axisSwapped,                false},

  // =========================
  // Movement / Speed
  // =========================
  {"maxSpeed",                    DOUBLE, &maxSpeed,                   10.0},
  {"speedMultiplier",             DOUBLE, &speedMultiplier,            0.5},
  {"calibrationSpeedMultiplier",  DOUBLE, &calibrationSpeedMultiplier, 0.1},
  {"preciseSpeedMultiplier",      DOUBLE, &preciseSpeedMultiplier,     0.01},
  {"offCenterSquaredMultiplier",  DOUBLE, &offCenterSquaredMultiplier, 8.0},
  {"offCenterLinearMultiplier",   DOUBLE, &offCenterLinearMultiplier,  3.0},
  {"offCenterAddition",           DOUBLE, &offCenterAddition,          -1.0},

  // =========================
  // Menu UI
  // =========================
  {"maxOptions",                  INT,    &maxOptions,                 8},
  {"maxCharsPerLine",             INT,    &maxCharsPerLine,            20},
  {"enableFocusArrow",            BOOL,   &enableFocusArrow,           true},
  {"currentScrollPosition",       INT,    &currentScrollPosition,      0},
  {"prettyPrintPageDelayMs",      INT,    &prettyPrintPageDelayMs,     7500},

  // =========================
  // Custom Scroll
  // =========================
  {"ScrollDelayMs",               INT,    &ScrollDelayMs,              200},
  {"ScrollSpacesUntilLoop",       INT,    &ScrollSpacesUntilLoop,      4},

  // =========================
  // Debug / Calibration Options
  // =========================
  {"debug",                       BOOL,   &debug,                      false},
  {"joystickCalibrate",           BOOL,   &joystickCalibrate,          true},
  {"simulateError106",            BOOL,   &simulateError106,           false},

  // =========================
  // Servo Profiles
  // =========================
  {"usedCalibrationList",         INT,    &usedCalibrationList,        4},
  {"usedCalibrationListSlot",     INT,    &usedCalibrationListSlot,    0}
};

// + 1 for the "Go Back"
const int settingsMenuLength = sizeof(settings) / sizeof(settings[0]) + 1;

// tempSettingInt is double so small changes don't get perpetually cut out upon rounding
bool tempSettingBool = false;
double tempSettingInt = 0;
double tempSettingDouble = 0;

String settingsMenuOptions[settingsMenuLength] = {""};

// #######################
// End of Variable Setup
// #######################

// Set up Permament Storage
Preferences savedSettings;
Preferences servoProfiles;

// Make a display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Servo TestServo;

void setup() 
{
  // fill in settingsMenuOptions
  for (int i = 0; i < settingsMenuLength - 1; i++)
  {
    settingsMenuOptions[i] = settings[i].name;
  }
  settingsMenuOptions[settingsMenuLength - 1] = "Go Back";

  // Start Memory
  savedSettings.begin("savedSettings", false);
  servoProfiles.begin("servoProfiles", false);

  // Load User Settings and Profiles from Memory
  loadSettings();
  loadProfiles();

  // Reset Settings (if enabled by user)
  if (resetSavedSettings)
  {
    resetSettings();
  }
  
  // Reset Profiles(if enabled by user)
  if (resetSavedProfiles)
  {
    resetProfiles();
  }

  // Update Profile from settings
  updateSelectedProfile();

  Serial.begin(115200);

  // Give buttonPin an internal pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);

  // Set up display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

  // Load Boot Screen
  startupScreen();

  // ALWAYS KEEP THIS AT THE BOTTOM
  TestServo.attach(servoPin);
}


// Main Menu
void loop()
{
  double yOffCenter = 0;
  String yDirection = "NONE";

  unsigned long currentTime = millis();

  // Check for Display update
  if (currentTime - previousDisplayTime >= displayDelayMs)
  {
    previousDisplayTime = currentTime;
    buildMenu(mainMenuOptions, mainMenuLength);
  }

  // Check for Joystick movement
  if (currentTime - previousJoystickTime >= joystickDelayMs)
  {
    previousJoystickTime = currentTime;

    // Use the Joystick's Y axis to navigate
    yOffCenter = YaxisJoystickInfo();
    yDirection = offCenterToDirection(yOffCenter, 1);    
  }


  if (currentTime - previousClickTime >= clickDelayMs)
  {
    previousClickTime = currentTime;

    // Read if button has been pressed (first checks for button on purpose)
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      callMenuOption(0);
    }    
  }

  if (currentTime - previousDirectionTime >= selectedDelayMs)
  {
    previousDirectionTime = currentTime;

    // Update Y based on yDirection
    dirUpdateYselected(yDirection, mainMenuLength);
  }
}

void startupScreen()
{
  display.clearDisplay();

  // Prototype version of a startup display 
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(32,11);
  display.println("S.U.P.");
  display.setTextSize(1);
  display.setCursor(45, 28);  
  display.println(String("v") + version);
  display.setCursor(5, 41);  
  display.println("Developed by Kacenta");

  display.display();
  delay(3000);
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

      if (enableFocusArrow)
      {
        menuOption = customScroll(20, menuOption);
        menuOption = String("> ") + menuOption;
      }

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
          callCalibration();
          break;

        case 2:
          callSelectServoType();
          break;

        case 3:
          callDebug();
          break;

        case 4:
          callSettings();
          break;

        case 5:
          callAbout();
          break;

        default:
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
          callServoSweep();
          break;
        
        case 1:
          callPulseWidthCalibration()
          break;

        case 2:
          callLaserCalibration();
          break;

        case 3:
          callJoystickCalibration();
          break;

        case 4:
          callProfileSelect();
          break;

        // Go Back
        case 5:
          // Do nothing
          break;

        default:
          showErrorMessage(105, "Non-Existing Menu Option: " + String(selected));
          break;
      }
      // Reset selected
      selected = 0;
      break;
    }

    // ########################
    // Select Servo Type Menu
    // ########################
    case 2:
    {
      // Update servoType
      servoType = selected;

      // refresh picked angle list
      updateSelectedProfile();

      break;
    }

    // #############
    // Debug Menu
    // #############
    case 3:
    {
      switch (selected)
      {
        case 0:
          callJoystickDebug();
          break;
        case 1:
          callJoystickCalibrationDebug();
          break;
        case 2:
          // callDebugCalibrationAngles();
          break;
        case 3:
          showErrorMessage(101, DebugErrorMessage);
          break;
        case 4:
        case 5:
        case 6:
          callErrorSimulation();
          break;

        default:
          showErrorMessage(105, "Non-Existing Menu Option: " + String(selected));
          break;
      }
      // Reset selected
      selected = 0;
      break;
    }

    // ################
    // Settings Menu
    // ################
    case 4:
    {
      // unused for now
      // possible TODO

      // showErrorMessage(108, "Non-Existing Setting: " + String(selected));
          
      break;
    }

    // #############
    // About Menu
    // #############
    case 5:
    {
      // unused for now
      // possible TODO
      
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
  delay(actionDelay);

  // Open actual menu
  manualControl();

  // Prevent accidental return to the same option
  delay(actionDelay);
}

void callCalibration()
{
  delay(actionDelay);

  calibrationMenu();

  delay(actionDelay);
}

void callServoSweep()
{
  delay(actionDelay);

  switch (servoType)
  {
  case 0:
    servoSweep(minAngle, maxAngle, 5000);
    break;

  case 1:
    servoSweep(minAngle, maxAngle, 7000);
    break;
  
  case 2:
    servoSweepRotating(5000);
    break;

  default:
    break;
  }

  delay(actionDelay);
}

void callPulseWidthCalibration()
{
  delay(actionDelay);

  switch (servoType)
  {
    case 2:
      pulseWidthCalibration();
      break;
  
    default:
      showErrorMessage(305, "Unsupported Servo Type");
      break;
  }

  delay(actionDelay);
}

bool callPickCalibrationListMenu()
{
  delay(actionDelay);

  bool goback = pickCalibrationListMenu();

  delay(actionDelay);

  if (debug == true)
  {
    pickCalibrationListMenuDebug();
  }

  return goback;
}

void callPickCalibrationListSlot()
{
  delay(actionDelay);

  pickCalibrationListSlot();

  if (debug == true)
  {
    pickCalibrationListSlotMenuDebug();
  }

  delay(actionDelay);
}

void callLaserCalibration()
{
  delay(actionDelay);

  laserCalibration();

  delay(actionDelay);
}

void callJoystickCalibration()
{
  delay(actionDelay);

  joystickCalibration();

  delay(actionDelay);
}

void callProfileSelect()
{
  delay(actionDelay);

  profileSelect();

  delay(actionDelay);
}

void callSelectServoType()
{
  delay(actionDelay);

  selectServoTypeMenu();

  delay(actionDelay);
}

void callDebug()
{
  // Prevent accidental "Go back" reading
  delay(actionDelay);

  debugMenu();

  // Prevent accidental return to same option
  delay(actionDelay);  
}

void callJoystickDebug()
{
  // Prevent accidental "Go back" reading
  delay(actionDelay);

  joystickDebug();

  // Prevent accidental return to same option
  delay(actionDelay);  
}

void callJoystickCalibrationDebug()
{
  // Prevent accidental "Go back" reading
  delay(actionDelay);

  joystickCalibrationDebug();

  // Prevent accidental return to same option
  delay(actionDelay);  
}

void callErrorSimulation()
{
  // Prevent accidental "Go back" reading
  delay(actionDelay);

  errorSimulationMenu();

  // Prevent accidental return to same option
  delay(actionDelay);  
}

void callSettings()
{
  delay(actionDelay);

  settingsMenu();

  delay(actionDelay);
}

void callAbout()
{
  delay(actionDelay);

  About();

  delay(actionDelay);
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

String updateXvalue()
{
  if (settings[selected].type == BOOL)
  {
    double xOffCenter = XaxisJoystickInfo();
    String xDirection = offCenterToDirection(xOffCenter, 0);
    
    if (xDirection == "RIGHT")
    {
      // *(bool*)settings[selected].value = true;
      tempSettingBool = true;
      return "TRUE";
    }
    else if (xDirection == "LEFT")
    {
      // *(bool*)settings[selected].value = false;
      tempSettingBool = false;
      return "FALSE";
    }
    else
    {
      // Just read the settings value and return it
      if (*(bool*)settings[selected].value == true)
      {
        return "TRUE";
      }
      else if (*(bool*)settings[selected].value == false)
      {
        return "FALSE";
      }
    }
  }
  else if (settings[selected].type == INT)
  {
    double xOffCenter = XaxisJoystickInfo();
    double speed = offCenterToSpeed(xOffCenter); 
    // *(int*)settings[selected].value += (speed * speedMultiplier * preciseSpeedMultiplier);
    tempSettingInt += (speed * speedMultiplier * preciseSpeedMultiplier);

    // return String(*(int*)settings[selected].value);
    return String(tempSettingInt);
  }
  else if (settings[selected].type == DOUBLE)
  {
    double xOffCenter = XaxisJoystickInfo();
    double speed = offCenterToSpeed(xOffCenter); 
    // *(double*)settings[selected].value += (speed * speedMultiplier * preciseSpeedMultiplier);
    tempSettingDouble += (speed * speedMultiplier * preciseSpeedMultiplier);

    // return String(*(double*)settings[selected].value);
    return String(tempSettingDouble);
  }
  else
  {
    showErrorMessage(204, "Wrong Setting Type specified: " + String(settings[selected].type));
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
// Also tracks joystick click state
void joystickDebug()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("JOYSTICK DEBUG MENU");
  display.println("-------------------");
  display.println("RESET TO GO BACK");
  display.println("YOU CANNOT CLICK TO RETURN HERE");
  display.display();

  delay(2000);
  
  // Immitate a new loop() function
  while (true)
  {
    int buttonState = digitalRead(buttonPin);
    bool isClicked = false;
    if (!buttonState)
    {
      isClicked = true;
      
      // Go back
      // return;
    }
    
    display.clearDisplay();

    double xOffCenter = XaxisJoystickInfo();
    double yOffCenter = YaxisJoystickInfo();

    double speedX = offCenterToSpeed(xOffCenter);
    double speedY = offCenterToSpeed(yOffCenter);

    String xDirection = offCenterToDirection(xOffCenter, 0);
    String yDirection = offCenterToDirection(yOffCenter, 1);

    display.setCursor(0,0);
    display.setTextSize(2);
    display.println(xOffCenter);
    display.println(xDirection);
    display.println(yOffCenter);
    display.println(yDirection);

    display.setCursor(64, 0);
    display.println("Click");

    display.setCursor(64, 16);
    if (isClicked)
    {
      display.println("TRUE");
    }
    else
    {
      display.println("FALSE");
    }

    display.setCursor(64, 32);
    display.setCursor(64, 48);

    display.display();

    delay(100);
  }
}

void joystickCalibrationDebug()
{
  // Immitate a new loop() function
  while (true)
  {
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {      
      // Go back
      return;
    }
    
    display.clearDisplay();

    double xOffCenter = XaxisJoystickInfo();
    double yOffCenter = YaxisJoystickInfo();

    double xOffCenterCalibrated = calibrateJoystickOffCenter(xOffCenter, 0);
    double yOffCenterCalibrated = calibrateJoystickOffCenter(yOffCenter, 1);

    double speedX = offCenterToSpeed(xOffCenter);
    double speedY = offCenterToSpeed(yOffCenter);

    String xDirection = offCenterToDirection(xOffCenter, 0);
    String yDirection = offCenterToDirection(yOffCenter, 1);

    display.setCursor(0,0);
    display.setTextSize(2);

    display.println(xOffCenter);
    display.println(yOffCenter);

    display.println(speedX);
    display.println(speedY);
    
    display.setCursor(64, 0);

    display.println(xOffCenterCalibrated);
    
    display.setCursor(64, 16);
    
    display.println(yOffCenterCalibrated);
    
    display.setCursor(64, 32);
    
    display.println(xDirection);
    
    display.setCursor(64, 48);
    
    display.println(yDirection);

    display.display();

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
    // returns 0
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

  // Return angle if there is a problem with the range
  return angle;
}

// Check if angle exceeds minAngle or maxAngle
double fixServoAngle(double angle)
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
  minAngle = pickedAngleList[0];

  maxAngle = pickedAngleList[ListSize - 1];

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

    skipableDelay(2000);
    delay(actionDelay);
  }
}

void manualControl() 
{
  // reset previous timers
  resetTimers();

  // update Min and Max Angles based on profile preferences
  updateMinMaxAngles();

  double xOffCenter = 0;
  double angleChange = 0;
  double calibratedAngle = 0;

  String showCurrentAngleDigital = "";
  String showCurrentAngleReal = "";
  String showCurrentSpeed = "";

  int buttonState = 0;

  // Immitate a new loop() function
  while (true)
  {
    unsigned long currentTime = millis();

    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;

      xOffCenter = XaxisJoystickInfo();
      angleChange = offCenterToSpeed(xOffCenter);
      angle += angleChange * speedMultiplier;

      // flattend angle to accepted range
      angle = fixServoAngle(angle);

      // calibrate angle before sending it to Servo.write()
      // uses global variables so it can handle profile change
      calibratedAngle = digitalAngleToCalibrated(angle, ListSize, pickedAngleList, referenceAngleList);
      
      TestServo.write(calibratedAngle);
    }

    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);

      showCurrentAngleDigital = String("Angle (Digital): ") + angle;
      showCurrentAngleReal = String("Angle (Real): ") + calibratedAngle;
      showCurrentSpeed = String("Current Speed: ") + angleChange;

      display.println(showCurrentAngleDigital);
      display.println(showCurrentAngleReal);
      display.println(showCurrentSpeed);

      display.display();      
    }

    buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // Go back
      return;
    }
  }
}

void calibrationMenu()
{
  // Reset timers
  resetTimers();

  // Reset variables
  selected = 0;
  firstVisible = 0;

  double yOffCenter = 0;
  String yDirection = "NONE";

  while(true)
  {
    unsigned long currentTime = millis();

    // Check for Display update
    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      previousDisplayTime = currentTime;

      // Build the Calibration Menu
      buildMenu(calibrationMenuOptions, calibrationMenuLength);
    }


    // Check for Joystick movement
    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;
      
      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();

      // Convert offCenter to "UP" or "DOWN"
      yDirection = offCenterToDirection(yOffCenter, 1);      
    }


    // Check for click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;
      
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
    }


    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // then, second, update Y based on yDirection
      dirUpdateYselected(yDirection, calibrationMenuLength);
    }
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
  display.setTextSize(2);
  display.setTextColor(WHITE);

  String Message = String("List Picked: ") + usedCalibrationList;

  display.println(Message);
  display.display();

  delay(actionDelay);
  skipableDelay(1000 - actionDelay);
}

void pickCalibrationListSlotMenuDebug()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);

  String Message = String("Slot Picked: ") + usedCalibrationListSlot;

  display.println(Message);
  display.display();

  delay(actionDelay);
  skipableDelay(1000 - actionDelay);
}

bool pickCalibrationListMenu()
{
  // Reset timers
  resetTimers();

  // Reset menu variables
  selected = 0;
  firstVisible = 0;

  double yOffCenter = 0;
  String yDirection = "NONE";
  
  while (true)
  {
    unsigned long currentTime = millis();

    // Check for Display update
    if (currentTime - previousDisplayTime >= displayDelayMs)
    { 
      previousDisplayTime = currentTime;

      // Build the Pick Calibration List Menu
      buildMenu(pickCalibrationListMenuOptions, pickCalibrationListMenuLength);
    }


    // Check for Joystick movement
    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;
      
      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();
      yDirection = offCenterToDirection(yOffCenter, 1);
    }


    // Check for click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;
      
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
    }

    
    // Check for selected update
    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // Update Y based on yDirection
      dirUpdateYselected(yDirection, pickCalibrationListMenuLength);
    }
  }
}


void pickCalibrationListSlot()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;

  // Reset timers
  resetTimers();

  double yOffCenter = 0;
  String yDirection = "NONE";

  while (true)
  {
    unsigned long currentTime = millis();

    // Check for display update
    if (currentTime - previousDisplayTime >= clickDelayMs)
    {
      previousDisplayTime = currentTime;

      buildMenu(pickCalibrationSlotMenuOptions, pickCalibrationSlotMenuLength);
    }

    // Check for joystick movement
    if (currentTime - previousJoystickTime >= displayDelayMs)
    {
      previousJoystickTime = currentTime;

      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();
      yDirection = offCenterToDirection(yOffCenter, 1);
    }

    // Check for click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;

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
    }

    // Check for selected update
    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // Update Y based on yDirection
      dirUpdateYselected(yDirection, pickCalibrationSlotMenuLength); 
    }
  }
}

// User picks new list and slot and the profile gets updated automatically
void profileSelect()
{ 
  pickCalibrationListMenu();
  delay(actionDelay);

  pickCalibrationListSlot();
  delay(actionDelay);

  updateSelectedProfile();
}

// Uses a laser strapped to the horn of the servo to calculate servo angle using tg  
void laserCalibration()
{
  // [LIST] Picker
  // Pick Servo Angle Accuracy (the more samples, the bettter)
  // no vars needed cuz it updates usedCalibrationList (global scale var)
  bool goback = callPickCalibrationListMenu();
  if (goback)
  {
    // Go back
    return;
  }
  
  // [SLOT] Picker
  // updates usedCalibrationListSlot
  callPickCalibrationListSlot();

  // Use information to update the chosen profile
  updateSelectedProfile();
  
  // Saved Servo Variables
  double startingDistance = 0;
  double Distances[ListSize];

  // Start up the Script for getting the Laser to wall distance
  startingDistance = getLaserToWallDistance();

  // Wait a bit to prevent accidental clicks 
  delay(actionDelay);

  // distance from laser dot to marking on the wall (in cm)
  double distance = 0;

  for (int i = 0; i < ListSize; i++)
  {
    // add a small delay to not skip the warning
    delay(actionDelay);
    
    // RESET distance
    distance = 0;

    // Reset timers
    resetTimers();

    // Picks current angle based on used list
    double servoAngle = referenceAngleList[i];

    // warn the user to place the dot on the marking by physically rotating the servo
    showWarning(servoAngle);

    waitForClick();
    
    // Set the servo to the corresponding angle 
    TestServo.write(servoAngle);

    double yOffCenter = 0;
    double speed = 0; 

    // wait till user clicks
    while (true)
    {
      unsigned long currentTime = millis();

      // Check for joystick movement
      if (currentTime - previousJoystickTime >= joystickDelayMs)
      {
        previousJoystickTime = currentTime;

        yOffCenter = YaxisJoystickInfo();
        speed = offCenterToSpeed(yOffCenter); 
        distance += (speed * speedMultiplier * calibrationSpeedMultiplier);        
      }
      
      // Check for display update
      if (currentTime - previousDisplayTime >= displayDelayMs)
      {
        previousDisplayTime = currentTime;

        String Message = String("Input ") + servoAngle + ("deg\ndistance");

        // Update Screen to Show Results
        displayDistanceAndSpeedResult(distance, speed, Message);
      }

      // Check for button
      if (currentTime - previousClickTime >= clickDelayMs)
      {
        previousClickTime = currentTime;

        int buttonState = digitalRead(buttonPin);
        if (!buttonState)
        {
          // Save distance
          Distances[i] = distance;

          // Continue
          break;
        }
      }
    }
  }

  display.clearDisplay();

  // Make a String for displaying Distances
  String distancesMessage = String("startingDistance: ") + String(startingDistance);
  distancesMessage += "/nDistances:/n";

  // Make a String for displaying Angles
  String anglesMessage = String("Angles Saved:/n");

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
    distancesMessage += String("/n");

    // Add to Angles Message
    // referenceAngle: calibratedAngle
    anglesMessage += String(referenceAngleList[i]); 
    anglesMessage += String(": ");
    anglesMessage += String(pickedAngleList[i]); 
    anglesMessage += String("/n");
  }

  // Save Profiles
  saveProfiles();

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
  String distanceMessage = String("Distance (cm): ") + inputDistance + String("\n");

  // Print everything to screen
  display.println(TopMessage);
  display.println(distanceMessage);
  display.println(String("Speed: ") + inputSpeed);
  display.setCursor(25,45);
  display.println("CLICK TO SAVE");
  display.display();
}

void showWarning(double Angle)
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);

  display.println("Incoming " + String(Angle) + "deg\nreading...");
  display.println("Rotate the servo...");
  display.println("Place the dot on the marking!");

  display.setCursor(15,45);
  display.println("CLICK TO CONTINUE");

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
    unsigned long currentTime = millis();

    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      currentTime = previousJoystickTime;

      // Read joystick for speed
      double yOffCenter = YaxisJoystickInfo();
      double speed = offCenterToSpeed(yOffCenter); 
      distance += (speed * speedMultiplier * calibrationSpeedMultiplier);
  
      // Fix distance
      if (distance < 0)
      {
        distance = 0;
      }
    }


    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      currentTime = previousDisplayTime;
      
      // Output Message
      displayDistanceAndSpeedResult(distance, speed, String("Input wallDistance"));
    }

    
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      currentTime = previousClickTime;
      
      int buttonState = digitalRead(buttonPin);
      if (!buttonState)
      {
        // Save Distance
        return distance;
      }
    }
  }
}

void joystickCalibration()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("JOYSTICK CALIBRATION");
  display.println("---------------------");
  display.println("CLICK TO CONTINUE");

  display.display();

  while (true)
  {
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      delay(actionDelay);

      // Continue
      break;
    }
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("STARTING CALIBRATION");
  display.println("---------------------");
  display.println("PLEASE WAIT...\n");
  display.println("DO NOT TOUCH THE \nJOYSTICK!");
  display.display();

  delay(2500);

  double XsampleSum = 0;
  double YsampleSum = 0;

  int samples = 500;
  for (int i = 0; i < samples; i++)
  {
    double xOffCenter = XaxisJoystickInfo();
    double yOffCenter = YaxisJoystickInfo();

    XsampleSum += xOffCenter;
    YsampleSum += yOffCenter;

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("TAKING SAMPLES");
    display.println("---------------------");
    display.println("SAMPLE " + String(i + 1) + "\nOUT OF " + String(samples));
    display.println("DO NOT TOUCH THE JOYSTICK!");
    display.display();

    delay(5);
  }

  double xAverage = XsampleSum / samples;
  double yAverage = YsampleSum / samples;
   
  // Show samples. ask user to confirm values with click

  // Directly write into the saved values
  xCalibration = xAverage;
  yCalibration = yAverage;
}

double calibrateJoystickOffCenter(double offCenter, int Axis)
{
  double calibration = 0;
  int sign = 0;
  int valueSign = 0;
  int minusSign = 0;

  // X axis
  if (Axis == 0)
  {
    calibration = xCalibration;
  }
  // Y axis
  else if (Axis == 1)
  {
    calibration = yCalibration;
  }
  else
  {
    showErrorMessage(203, "Wrong Axis specified: " + String(Axis));
  }

  if (offCenter >= calibration)
  {
    sign = 1;
  }
  else
  {
    sign = -1;
  }

  if (offCenter >= 0)
  {
    valueSign = 1;
  }
  else
  {
    valueSign = -1;
  }

  if (calibration > 0)
  {
    minusSign = 1;
  }
  else 
  {
    minusSign = -1;
  }

  // EXAMPLE:
  // calibration = 0.05

  //  sign  =  1
  //  valueSign  =  1  
  //  0.05  =  0.05     - (  1   *        0.05       * (1  - (1         *   0.05 )))
  //  0.50  =  0.50     - (  1   *        0.05       * (1  - (1         *   0.50 )))
  //  1.00  =  1.00     - (  1   *        0.05       * (1  - (1         *   1.00 )))

  //  sign  =  -1
  //  valueSign = 1;
  //  0.00  =  0.00     - (  1   *        0.05       * (1  - (1         *   0 )))
  //  0.04  =  0.04     - (  1   *        0.05       * (1  - (1         *   0.04 )))

  //  sign  =  -1
  //  valueSign = -1;
  // -0.01  =  -0.01    - ( -1   *        0.05       * (1 - (-1         *  -0.01)))
  // -0.05  =  -0.05    - ( -1   *        0.05       * (1 - (-1         *  -0.05)))
  // -0.50  =  -0.50    - ( -1   *        0.05       * (1 - (-1         *  -0.50)))
  // -1.00  =  -1.00    - ( -1   *        0.05       * (1 - (-1         *  -1.00)))

  // EXAMPLE:
  // calibration = -0.05

  //  sign  =  1
  //  valueSign  =  1  
  //  0.05  =  0.05     + (  1   *       -0.05       * (1  - (1         *   0.05 )))
  //  0.50  =  0.50     + (  1   *       -0.05       * (1  - (1         *   0.50 )))
  //  1.00  =  1.00     + (  1   *       -0.05       * (1  - (1         *   1.00 )))

  //  sign  =  1
  //  valueSign = 1;
  //  0.00  =  0.00     + (  1   *       -0.05       * (1  - (1         *   0 )))
  //  0.04  =  0.04     + (  1   *       -0.05       * (1  - (1         *   0.04 )))

  //  sign  =  1
  //  valueSign = -1;
  // -0.01  =  -0.01    + ( -1   *       -0.05       
  // -0.05  =  -0.05    + ( -1   *       -0.05       

  //  sign  =  -1
  //  valueSign = -1;
  // -0.50  =  -0.50    + ( -1   *       -0.05       * (1 - (-1         *  -0.50)))
  // -1.00  =  -1.00    + ( -1   *       -0.05       * (1 - (-1         *  -1.00)))

  double calibratedOffCenter = offCenter - minusSign * ((valueSign) * (calibration * (1 - (valueSign) * offCenter)));

  return calibratedOffCenter;
}

void About()
{
  String aboutMessage = "The Servo Utility /nPlatform is an open-source project made entirely by me - Denislav Tsenov (a.k.a. Kacenta). It is under the MIT Licence which means it is 100% free to use//modify//sell the contents of this /nproject at any given time ;D. /n/nAbout me.../n/nI am a 17-year-old student (as of 2026) living in /nBulgaria./nI love doing lots of things, some of which:/n - Playing on my Electric Guitar/n - Writing Songs/n - Playing Video Games/n - Training/n - Programming/n - and more.../n/nI have been programming for quite some time now, but mainly using AI and wanted to do something more simple but /nentirely written by my 2 hands /n(or 10 fingers, idk XD)./nSo I am proud to /nannounce.../nMy first truly /nAI-FREE Project./n";
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(15, 0);
  display.println("S. U. P.");

  display.setCursor(15, 16);
  display.println("CREDITS");
  display.display();

  prettyPrint(aboutMessage, 6, 32, 1, 19, 4);
}

void showErrorMessage(int ErrorID, String errorMessage)
{
  delay(actionDelay);
  
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

  String messageFragments[200] = {};

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
          delay(actionDelay);
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
          delay(actionDelay);
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

  // Calibrate reading if enabled by user
  if (joystickCalibrate)
  {
    xOffCenter = calibrateJoystickOffCenter(xOffCenter, 0);
  }

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

  // Calibrate reading if enabled by user
  if (joystickCalibrate)
  {
    yOffCenter = calibrateJoystickOffCenter(yOffCenter, 1);
  }

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
  
  // reset timers
  resetTimers();
  
  double yOffCenter = 0;
  String yDirection = "NONE";


  while (true)
  {
    unsigned long currentTime = millis();

    // Check for display update
    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      previousDisplayTime = currentTime;

      buildMenu(DebugMenuOptions, DebugMenuLength);
    }

    // Check for joystick movement
    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;
      
      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();
      yDirection = offCenterToDirection(yOffCenter, 1);
    }

    // Check for button click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;

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
        callMenuOption(3);
      }
    }

    // Check for direction
    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // Update Y based on yDirection
      dirUpdateYselected(yDirection, DebugMenuLength);
    }
  }
}

void settingsMenu()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;
  
  // reset timers
  resetTimers();
  
  double yOffCenter = 0;
  String yDirection = "NONE";

  while (true)
  {
    unsigned long currentTime = millis();

    // Check for display Update
    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      previousDisplayTime = currentTime;

      buildMenu(settingsMenuOptions, settingsMenuLength);
    }

    // Check for joystick movement
    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;

      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();
      yDirection = offCenterToDirection(yOffCenter, 1);
    }

    // Check for button click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;

      // Read if button has been pressed (first checks for button on purpose)
      int buttonState = digitalRead(buttonPin);
      if (!buttonState)
      {
        // Keep "Go Back" last so this always works
        if (selected == settingsMenuLength - 1)
        {
          // Go Back
          return;
        }
        
        delay(actionDelay);

        // Actually get the value of the setting so u don't start from 0
        if (settings[selected].type == BOOL)
          tempSettingBool = *(bool*)settings[selected].value;
        else if (settings[selected].type == INT)
          tempSettingInt = *(int*)settings[selected].value;
        else if (settings[selected].type == DOUBLE)
          tempSettingDouble = *(double*)settings[selected].value;
  
        // temporarily disable the focus arrow
        enableFocusArrow = false;
  
        int charCount = 0;
  
        // Start Changing the Setting
        while (true)
        {
          // Get Char count
          charCount = getSettingValueLength();
  
          currentTime = millis();
  
          if (currentTime - previousJoystickTime >= joystickDelayMs)
          {
            previousJoystickTime = currentTime;
            
            updateSetting(charCount);
          }
    
          // Check for button click
          if (currentTime - previousClickTime >= clickDelayMs)
          {
            previousClickTime = currentTime;
  
            // Read if button has been pressed (first checks for button on purpose)
            buttonState = digitalRead(buttonPin);
            if (!buttonState)
            {
              // Make temp setting permament
              // Changing the real setting at the end prevents weird behaviour during setting change
              if (settings[selected].type == BOOL)
              {
                *(bool*)settings[selected].value = tempSettingBool;
              }
              else if (settings[selected].type == INT)
              {
                *(int*)settings[selected].value = tempSettingInt;
              }
              else if (settings[selected].type == DOUBLE)
              {
                *(double*)settings[selected].value = tempSettingDouble;
              }

              // Reset ALL temp settings (just in case)
              resetTempSettings();
              
              // Update NVS Memory
              saveSettings();
    
              // Action delay before going back the the settings menu
              delay(actionDelay);
    
              // Go Back to the Settings Menu
              break;
            }
          }

          yield();
        }
        
        // re-enable focus arrow after breaking
        enableFocusArrow = true;
      }
    }

    // Check for direction update
    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // Update Y based on yDirection
      dirUpdateYselected(yDirection, settingsMenuLength);
    }

    yield();
  }
}

void updateSetting(int charCounter)
{
  settingsMessage = updateXvalue();

  display.clearDisplay();

  // Make the setting centered and have arrors at the ends
  settingsMessage = centerText(settingsMessage, charCounter);

  // temporarily change the list so you can directly change the value
  String tempMenuOption = settingsMenuOptions[selected];
  settingsMenuOptions[selected] = settingsMessage;

  // build the new menu
  buildMenu(settingsMenuOptions, settingsMenuLength);
  display.display();

  // Make the settingsMenuOption go back to normal
  settingsMenuOptions[selected] = tempMenuOption;
}

String centerText(String message, int charCounter)
{
  int emptySpaces = maxCharsPerLine - charCounter - 4;
  bool oddCharCount = false;

  if(emptySpaces < 0)
  {
    emptySpaces = 0;
  }
  else if(emptySpaces % 2 != 0)
  {
    oddCharCount = true;
    emptySpaces--;
  }

  // add the spaces before the value
  for (int i = 0; i < emptySpaces / 2; i++)
  {
    message = " " + message;
  }

  if (oddCharCount)
  {
    // add the odd space to the right
    message = message + " ";  
  }

  // add the spaces after the value
  for (int i = 0; i < emptySpaces / 2; i++)
  {
    message = message + " ";
  }

  // Add the arrows on the sides
  message = ">>" + message + "<<";

  return message;
}

void errorSimulationMenu()
{
  // Reset menu variables
  selected = 0;
  firstVisible = 0;

  // reset Timers
  resetTimers();

  double yOffCenter = 0;
  String yDirection = "NONE";
  
  while (true)
  {
    unsigned long currentTime = millis();

    // Check for display update
    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      previousDisplayTime = currentTime;

      buildMenu(ErrorTypes, ErrorTypesLength);
    }

    // Check for joystick movement
    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;

      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();
      yDirection = offCenterToDirection(yOffCenter, 1);
    }

    // Check for button click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;

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
    }

    // Check for direction change
    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // Update Y based on yDirection
      dirUpdateYselected(yDirection, ErrorTypesLength);
    }
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
      // Force Error 106: Non-Existing Servo Profile
      simulateError106 = true;
      laserCalibration();
      simulateError106 = false;
      break;

    case 4:
      // Force Error 108: Non-Existing Setting
      selected = 100;
      updateSetting(0);
      break;

    case 5:
      // Force Error 109: Unkown Error Specified
      selected = 100;
      simulateError();
      break;

    case 6:
      // Force Error 203: Wrong Axis Specified
      offCenterToDirection(0, 3);
      break;

    case 7: 
      // Force Error 204: Wrong Setting Type Specified
      // currentSettingType = String("IRVING FORCE - Corporate Killer");
      updateXvalue();
      break;

    case 8:
      // Force Error 305: Unsupported Servo Type
      servoType = -1;
      pulseWidthCalibration();
      break;

    default:
      showErrorMessage(106, "Unkown Error Specified: " + String(selected));
      break;
  }
}

void saveSettings()
{
  for (int i = 0; i < settingsMenuLength - 1; i++)
  {
    Setting currentSetting = settings[i];

    switch (currentSetting.type)
    {
    case BOOL:
      savedSettings.putBool(currentSetting.name, *(bool*)currentSetting.value);
      break;

    case INT:
      savedSettings.putInt(currentSetting.name, *(int*)currentSetting.value);
      break;

    case DOUBLE:
      savedSettings.putDouble(currentSetting.name, *(double*)currentSetting.value);
      break;
    
    default:
      showErrorMessage(204, "Wrong Setting Type specified: " + String(currentSetting.type));
      break;
    }
  }

  reconfigureHardware();
}

void loadSettings()
{
  for (int i = 0; i < settingsMenuLength - 1; i++)
  {
    Setting currentSetting = settings[i];

    switch (currentSetting.type)
    {
    case BOOL:
      *(bool*)currentSetting.value = savedSettings.getBool(currentSetting.name, (currentSetting.defaultValue != 0));
      break;

    case INT:
      *(int*)currentSetting.value = savedSettings.getInt(currentSetting.name, (int)currentSetting.defaultValue);
      break;

    case DOUBLE:
      *(double*)currentSetting.value = savedSettings.getDouble(currentSetting.name, currentSetting.defaultValue);
      break;
    
    default:
      showErrorMessage(204, "Wrong Setting Type specified: " + String(currentSetting.type));
      break;
    }
  }

  reconfigureHardware();
}

void resetSettings()
{
  for (int i = 0; i < settingsMenuLength - 1; i++)
  {
    Setting currentSetting = settings[i];

    switch (currentSetting.type)
    {
    case BOOL:
      *(bool*)currentSetting.value = (currentSetting.defaultValue != 0);
      break;

    case INT:
      *(int*)currentSetting.value = (int)currentSetting.defaultValue;
      break;

    case DOUBLE:
      *(double*)currentSetting.value = currentSetting.defaultValue;
      break;
    
    default:
      showErrorMessage(204, "Wrong Setting Type specified: " + String(currentSetting.type));
      break;
    }
  }
  
  // TODO: CHECK AND REMOVE THIS IF IT WORKS WELL
  
  // servoPin = 38;
  // xPin = 2;
  // yPin = 1;
  // buttonPin = 12;
  // xAxisInverted = true;
  // yAxisInverted = true;
  // axisSwapped = false;
  // maxSpeed = 10.0;
  // speedMultiplier = 0.5;
  // calibrationSpeedMultiplier = 0.1;
  // preciseSpeedMultiplier = 0.01;
  // offCenterSquaredMultiplier = 8;
  // offCenterLinearMultiplier = 3;
  // offCenterAddition = -1;
  // debug = true;
  // maxOptions = 8;
  // maxCharsPerLine = 20;
  // currentScrollPosition = 0;
  // prettyPrintPageDelayMs = 7500;
  // loopDelayMs = 10;
  // ScrollDelayMs = 200;
  // ScrollSpacesUntilLoop = 4;
  // usedCalibrationList = 4;
  // usedCalibrationListSlot = 0;
  // xDeadzone = 0.15;
  // yDeadzone = 0.15;
  // xCalibration = 0;
  // yCalibration = 0;

  reconfigureHardware();
}

void reconfigureHardware()
{
  pinMode(buttonPin, INPUT_PULLUP);

  TestServo.detach();
  TestServo.attach(servoPin);
}

void saveProfiles()
{
  // save user profiles 
  servoProfiles.putBytes("SavedAnglesBasic", SavedAnglesBasic, sizeof(SavedAnglesBasic));
  servoProfiles.putBytes("SavedAnglesAccurate", SavedAnglesAccurate, sizeof(SavedAnglesAccurate));
  servoProfiles.putBytes("SavedAnglesServophile", SavedAnglesServophile, sizeof(SavedAnglesServophile));
  servoProfiles.putBytes("SavedAnglesSuperServophile", SavedAnglesSuperServophile, sizeof(SavedAnglesSuperServophile));
  servoProfiles.putBytes("SavedAnglesMEGAServophile", SavedAnglesMEGAServophile, sizeof(SavedAnglesMEGAServophile));
  servoProfiles.putBytes("SavedAnglesOVERKILLServophile", SavedAnglesOVERKILLServophile, sizeof(SavedAnglesOVERKILLServophile));
}

void loadProfiles()
{
  servoProfiles.getBytes("SavedAnglesBasic", SavedAnglesBasic, sizeof(SavedAnglesBasic));
  servoProfiles.getBytes("SavedAnglesAccurate", SavedAnglesAccurate, sizeof(SavedAnglesAccurate));
  servoProfiles.getBytes("SavedAnglesServophile", SavedAnglesServophile, sizeof(SavedAnglesServophile));
  servoProfiles.getBytes("SavedAnglesSuperServophile", SavedAnglesSuperServophile, sizeof(SavedAnglesSuperServophile));
  servoProfiles.getBytes("SavedAnglesMEGAServophile", SavedAnglesMEGAServophile, sizeof(SavedAnglesMEGAServophile));
  servoProfiles.getBytes("SavedAnglesOVERKILLServophile", SavedAnglesOVERKILLServophile, sizeof(SavedAnglesOVERKILLServophile));
}

void resetProfiles()
{
  // Reset all profiles to their references
  // until 6 Cuz there are 6 profile slots right now
  for (int i = 0; i < 6; i++)
  {
    for (int j = 0; j < SavedAnglesBasicLength; j++)
    {
      SavedAnglesBasic[i][j] = SavedAnglesBasicReference[servoType][j];
    }
    for (int j = 0; j < SavedAnglesAccurateLength; j++)
    {
      SavedAnglesAccurate[i][j] = SavedAnglesAccurateReference[servoType][j];
    }
    for (int j = 0; j < SavedAnglesServophileLength; j++)
    {
      SavedAnglesServophile[i][j] = SavedAnglesServophileReference[servoType][j];
    }
    for (int j = 0; j < SavedAnglesSuperServophileLength; j++)
    {
      SavedAnglesSuperServophile[i][j] = SavedAnglesSuperServophileReference[servoType][j];
    }
    for (int j = 0; j < SavedAnglesMEGAServophileLength; j++)
    {
      SavedAnglesMEGAServophile[i][j] = SavedAnglesMEGAServophileReference[servoType][j];
    }
    for (int j = 0; j < SavedAnglesOVERKILLServophileLength; j++)
    {
      SavedAnglesOVERKILLServophile[i][j] = SavedAnglesOVERKILLServophileReference[servoType][j];
    }
  }

  saveProfiles();
}

void skipableDelay(int delayMs)
{
  for (int delayCounter = 0; delayCounter < (delayMs / 10); delayCounter++)
  {
    int buttonState = digitalRead(buttonPin);
    
    //  skip delay once clicked
    if (!buttonState)
    {
      return;
    }
    
    // divide the big delay into 10ms chunks
    delay(10);
  }
}

void resetTimers()
{
  previousTime = 0;
  previousDisplayTime = 0;
  previousJoystickTime = 0;
  previousClickTime = 0;
  previousDirectionTime = 0;
}

void resetTempSettings()
{
  tempSettingBool = false;
  tempSettingInt = 0;
  tempSettingDouble = 0;
}

// Get the length of the current setting
int getSettingValueLength()
{
  int charCounter = 0; 
  
  // Check setting type
  if (settings[selected].type == BOOL)
  {
    if (*(bool*)settings[selected].value == true)
    {
      // There are 4 chars in "TRUE"
      charCounter = 4;
    }
    else if (*(bool*)settings[selected].value == false)
    {
      // There are 5 chars in "FALSE"
      charCounter = 5;
    }
  }
  else if (settings[selected].type == INT)
  {
    for (char currentChar : String(*(int*)settings[selected].value))
    {
      charCounter++;
    }
  }
  else if (settings[selected].type == DOUBLE)
  {
    for (char currentChar : String(*(double*)settings[selected].value))
    {
      charCounter++;
    }
  }

  return charCounter;
}

void servoSweep(double minAngle, double maxAngle, int timeMs)
{
  int chunks = timeMs / 50;
  double angleDiff = maxAngle - minAngle;

  for (int i = 0; i < chunks; i++)
  {
    double currentAngle = minAngle + i * (angleDiff / chunks);
    TestServo.write(currentAngle);

    delay(50);
  }

  // Go back to start in the end
  TestServo.write(minAngle);
}

void servoSweepRotating(int timeMs)
{ 
  int chunks = timeMs / 50 / 2;

  for (int i = 0; i < chunks; i++)
  {
    TestServo.writeMicroseconds(savedPulseWidths[2]);
    delay(50);
  }

  for (int i = 0; i < chunks; i++)
  {
    TestServo.writeMicroseconds(savedPulseWidths[0]);
    delay(50);
  }
}

void selectServoTypeMenu()
{
  // Reset timers
  resetTimers();

  // Reset variables
  selected = 0;
  firstVisible = 0;

  double yOffCenter = 0;
  String yDirection = "NONE";

  while(true)
  {
    unsigned long currentTime = millis();

    // Check for Display update
    if (currentTime - previousDisplayTime >= displayDelayMs)
    {
      previousDisplayTime = currentTime;

      // Build the Calibration Menu
      buildMenu(servoTypeOptions, servoTypeLength);
    }


    // Check for Joystick movement
    if (currentTime - previousJoystickTime >= joystickDelayMs)
    {
      previousJoystickTime = currentTime;
      
      // Use the Joystick's Y axis to navigate
      yOffCenter = YaxisJoystickInfo();

      // Convert offCenter to "UP" or "DOWN"
      yDirection = offCenterToDirection(yOffCenter, 1);      
    }


    // Check for click
    if (currentTime - previousClickTime >= clickDelayMs)
    {
      previousClickTime = currentTime;
      
      // Read if button has been pressed (first checks for button on purpose)
      int buttonState = digitalRead(buttonPin);
      if (!buttonState)
      {
        // Keep "Go Back" last so it always works
        if (selected == servoTypeLength - 1)
        {
          // Go Back
          return;
        }

        delay(actionDelay);

        // Call to update servoType
        callMenuOption(2);
        
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        
        display.println("SERVO TYPE MENU");
        display.println("---------------------");
        display.println("SERVOTYPE UPDATED: ");
        display.println(String(servoTypeAngles[servoType]) + " DEG");
        display.display();

        delay(2500 - actionDelay);

        return;
      }
    }


    if (currentTime - previousDirectionTime >= selectedDelayMs)
    {
      previousDirectionTime = currentTime;

      // then, second, update Y based on yDirection
      dirUpdateYselected(yDirection, servoTypeLength);
    }
  }  
}

void waitForClick()
{
  // Wait for click to start
  while (true)
  {
    int buttonState = digitalRead(buttonPin);
    if (!buttonState)
    {
      // add a small delay to not skip the reading
      delay(actionDelay);

      break;
    }
  }
}

void pulseWidthCalibration()
{
  // reset previous timers
  resetTimers();

  // update Min and Max Angles based on profile preferences
  updateMinMaxAngles();

  double xOffCenter = 0;
  double speed = 0;
  float pulseWidths[] = {1000, 1500, 2000};

  String showPulseWidth = "";
  String showCurrentSpeed = "";

  int buttonState = 0;

  // Repeat 3 times - one for middle, 1 for min and one for max Pulse Width
  for (int i = 0; i < 3; i++)
  {
    // Immitate a new loop() function
    while (true)
    {
      unsigned long currentTime = millis();

      if (currentTime - previousJoystickTime >= joystickDelayMs)
      {
        previousJoystickTime = currentTime;

        xOffCenter = XaxisJoystickInfo();
        speed = offCenterToSpeed(xOffCenter);
        pulseWidths[i] += speed * speedMultiplier;
        
        TestServo.writeMicroseconds(pulseWidths[i]);
      }

      if (currentTime - previousDisplayTime >= displayDelayMs)
      {
        previousDisplayTime = currentTime;

        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);

        switch (i)
        {
          case 0:
            showPulseWidth = String("Center PulseWidth: ") + pulseWidths[i];
            break;
          case 1:
            showPulseWidth = String("Minimal PulseWidth: ") + pulseWidths[i];
            break;
          case 2:
            showPulseWidth = String("Maximum PulseWidth: ") + pulseWidths[i];
            break;
      
          default:
          break;
        }

        showPulseWidth = String("Current PulseWidth: ") + pulseWidths[i];
        showCurrentSpeed = String("Current Speed: ") + speed;

        display.println(showPulseWidth);
        display.println(showCurrentSpeed);

        display.setCursor(0,32);
        display.println("CLICK TO SAVE");

        display.display();
      }

      // Check for click
      if (currentTime - previousClickTime >= clickDelayMs)
      {
        previousClickTime = currentTime;
        
        buttonState = digitalRead(buttonPin);
        if (!buttonState)
        {
          // Save Pulse width
          savedPulseWidths[i] = pulseWidths[i]; 
          
          // Go back
          break;
        }
      }
    }
  }
}

void updateSelectedProfile()
{
  int ListID = usedCalibrationList;
  int slot = usedCalibrationListSlot;

  // Check if the User wants to force Error 106
  if (simulateError106)
  {
    // Force Error 106: Non-Existing Servo Profile
    ListID = 100;
  }

  // Pick servo profile based on the selected one
  switch (ListID)
  {
    case 0:
    {
      ListSize = SavedAnglesBasicLength;
      pickedAngleList = &SavedAnglesBasic[slot][0]; // Call [0] to get the array address
      referenceAngleList = &SavedAnglesBasicReference[servoType][0];
      break;      
    }

    case 1:
    {
      ListSize = SavedAnglesAccurateLength;
      pickedAngleList = &SavedAnglesAccurate[slot][0];
      referenceAngleList = &SavedAnglesAccurateReference[servoType][0];
      break;
    }

    case 2:
    {
      ListSize = SavedAnglesServophileLength;
      pickedAngleList = &SavedAnglesServophile[slot][0];
      referenceAngleList = &SavedAnglesServophileReference[servoType][0];
      break;
    }

    case 3:
    {
      ListSize = SavedAnglesSuperServophileLength;
      pickedAngleList = &SavedAnglesSuperServophile[slot][0];
      referenceAngleList = &SavedAnglesSuperServophileReference[servoType][0];
      break;
    }

    case 4:
    {
      ListSize = SavedAnglesMEGAServophileLength;
      pickedAngleList = &SavedAnglesMEGAServophile[slot][0];
      referenceAngleList = &SavedAnglesMEGAServophileReference[servoType][0];
      break;
    }

    case 5:
    {
      ListSize = SavedAnglesOVERKILLServophileLength;
      pickedAngleList = &SavedAnglesOVERKILLServophile[slot][0];
      referenceAngleList = &SavedAnglesOVERKILLServophileReference[servoType][0];
      break;
    }

    default:
      showErrorMessage(106, "Non-Existing Servo Profile");
      break;
  }
}