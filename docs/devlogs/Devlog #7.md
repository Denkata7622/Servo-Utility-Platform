# Devlog #7 - Getting Closer
## S.U.P. - v0.9.1

### To be honest, i kinda lied last time...
### There is still a lot of work to do, espesially about the servoType handling

## Important Features Added

- Added **WORKING** Joystick Calibration + integrated additional calibration logic directly in joystick movement (toggable, of course)
- You can finally **use the manualControl() with a fully calibrated custom profile**. Changing the selected profile now **updates automatically**.
- **Huge improvements** to Selecting Servo Type. Now calibration works for **both 90° and 180°** servos.
- Added **Servo Test Sweep**!
- Added a **custom option** in the main menu to **change profiles**
- **Wrote the About me** Section (Finally)

## Less Important Features Added

- **Isolated updateSelectedProfile()** in its own function so i can reuse it (like 100+ lines of code saved there...)
- Added debugger function (in "Debug" menu) for **testing the joystick calibration**
- Added a global int - actionDelay = 300ms. I have always added a delay(300) before changing display screens, starting/ending actions, ect.. to keep the app posished and remove the irritation of "fake"/"accidental" double, triple and spam clicks
- Fixed a **bug** in the normal joystick debugger function

## TODO

- [] Add support for 360 deg servos - basically writing a custom new version of each function but for rotating servos because they are fundamentally different then positional servos (such as 90° and 180°) and don't integrate well into the current ecosystem.

- [] Show which saved profile is for which servo type

- [] Film the video for the v1.0 release (we are really getting closer :D).
