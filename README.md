# Servo-Utility-Platform (SUP)

A (kinda) compact **ESP32-based tool** for setting up **servos** for your **Robotics Projects**

## Overview

The Servo-Utility-Platform (a.k.a **SUP**) **serv(o)s** as a useful tool for both engineers and hobbyists with its main focus aimed towards **testing**, **calibrating** and ~~**running diagnostics**~~ on servo motors - the backbone of a considerable amount of **Robotics** and **Engineering** projects.

## Features

- Finding **limits** of operation
- **Calibration** of the motors
- **C.E.A. Laser Calibration**
- (Planned) **Testing sweeps** from 0° to 180°
- (Planned) **RPM** for 360° rotating servos
- Custom saved **profiles** going as precise as per-2.8125° sample calibrations
- (Planned - Likely impossible) **Diagnostic** option for automatically setting up a module for use
- (Planned) Decently **portable design** using a 830-point breadboard (for now no custom enclosure or PCB has been planned)
- **Debug Menu** for **testing features** and **error-handling**
- **Settings Menu** for changing settings.  
Some Options include:  
**1\. Invert** one (or both) **Joystick axis**  
**2\. Swap Joystick axis**  
**3\. Debugging Option** (for developers)  
**4\.** Multiple **Speed Multipliers**  
**5\. Custom Delay lengths** for Different purposes  
**6\.  Current** calibration **profile** used
- (Planned) Make saved profiles **survive** microcontroller **RESET**
- **Polished** and **intuitive UI!**

## Hardware Used

- **ESP32-S3** - *x1*
- Small *I2C OLED* **128x64 display** - *x1*
- **Joystick modules** - *x1*
- **Servo modules** *(MG90) - x1*
- **Servo modules** *(MG90s) - x1-2*  
- *830-point* **breadboard** - *x1*
- Jumper and Dupont **wires** - *~25 max*  

## Repo Structure

```text
/docs
  /devlogs
  /images
  design.md
  journal.md
  TODO.md

/firmware
  /ServoUtilityPlatform
    ServoUtilityPlatform.ino

/working-tests
  /LED-Test
  /Serial-Debuffer
  ...

/hardware
  parts.md
  wiring.md
```

- **/firmware**  
  stores the **Arduino IDE sketch**

- **/hardware**  
   stores **wiring information** and **parts list**

- **/docs**  
  a custom **journal** to help me (because I forget things) and you too, i guess. You could **follow the project** progress from there

## Project Roadmap

- [x] Project planning
- [x] Repository setup
- [x] Basic servo control
- [x] OLED interface
- [x] Menu system
- [x] Calibration
- [x] Documentation
- [ ] Actually TEST the calibration workflow and check if it really works with a real servo
- [x] Add an X-axis animation to the Settings menu when changing variables
- [ ] Add a way to name list slots
- [ ] Make list slots dynamically generated instead of hardcoded
- - [ ] Add an "add/make new slot" option
- [ ] Make an on-screen keyboard or use a library
- [ ] Final Polish
- [ ] Film Demo Montage
- [ ] First release

## Current Status

First release will be **coming soon**!

## Licence

MIT License.
