# Servo-Utility-Platform (SUP)

A (kinda) compact **ESP32-based tool** for setting up **servos** for your **Robotics Projects**

## Overview

The Servo-Utility-Platform (a.k.a **SUP**) **serv(o)s** as a useful tool for both engineers and hobbyists with its main focus aimed towards **testing**, **calibrating** and **running diagnostics** on servo motors - the backbone of a considerable amount of **Robotics** and **Engineering** projects.

## Planned Features

- Finding the **limits** of operation on servos
- **Calibrating** the motors
- **Testing sweeps** from 0° to 180° for angle servos and **RPM** for 360° rotating servos
- Custom per-module **profiles**
- **Diagnostic** option for automatically setting up a module for use
- Decently **portable design** using a 830-point breadboard (for now no custom enclosure or PCB has been planned)

## Hardware Used

- **ESP-WROOM-32** - x1
- Small I2C OLED 128x64 display - x1
- Joystick modules - x1
- Servo modules (MG90) - x1
- Servo modules (MG90s) - x1-2  
- 830-point breadboard - x1
- Jumper and Dupont wires used - ~25 max  

## Repo Structure

- **/firmware**  
  stores the **Arduino IDE sketch**

- **/hardware**  
   stores **wiring information** and **parts list**

- **/docs**  
  a custom **journal** to help me (because I forget things) and you too, i guess. You could **follow the project** progress from there

## Project Roadmap

- [x] Project planning
- [x] Repository setup
- [ ] Basic servo control
- [ ] OLED interface
- [ ] Menu system
- [ ] Calibration
- [ ] Documentation
- [ ] First release

## Current Status

Planning / Designing and Repo Structure **setup**.
Development will be **starting soon**!

## Licence

MIT License.