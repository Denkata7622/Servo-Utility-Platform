# Devlog #1 - The Rise of S.U.P.

Hello everyone,

Welcome to the first ever Devlog of the **Servo Utility Platform** (aka. S.U.P.)

This is the project I decided to make for **Stardance**, and the first ever **solely non-AI-dependent** one :D

## Overview

So what even is S.U.P.?

[**Demo** can be found at **the Bottom** if you don't like reading explanations :D]

The Servo Utility Platform is, I won't lie, **nothing super wow**.

The idea is that instead of making a random test circuit every time I wanna test a new servo, I can **use S.U.P.** to quickly **test** it, **calibrate** it, and see if something is **wrong** with it.

At least that's the plan cuz right now it's still a bunch of wires on a breadboard :D

## Hardware Used

Currently, S.U.P. uses:

- **ESP-WROOM-32** x1
- Small I2C OLED 128x64 Display x1
- Joystick Module x1
- MG90S Servo Module x1
- 830-point Breadboard x1
- A suspicious amount of jumper wires...

## The Beginning

The first version was... **really simple** XD.

It was literally an ESP32 shooting electricity through a wire and nothing more.

[Check out the **"Fancy This" picture** at the bottom]

The goal wasn't to make something impressive. It was just to make sure I can do the simplest thing imaginable:

"Can **I make** the servo move?"

The answer was yes (thankfully XD).

After that I added a Volume Knob (**potentiometer**).

[Check **the Picture that features it** (it's really sad that I can't put the pictures in-between text)]

This helped me **control the servo angle**, which made it **more satisfying** to play around with but didn't feel quite right.

It felt like **giving a static position** to the servo, not **making it move**.

The knob might work for a real 10$ servo tester but it's just **not my style**.

## MEET: The Joystick

So I replaced the potentiometer with his cooler Oklahoma bro: **The Joystick**.

[A video of me **"casually"** testing the hardware can be found down below]

And this is where it started feeling like **the gadget of my dreams**.

Instead of:

**Pot position → Static Servo angle**

I first changed it to:

**Joystick Pot position → Static Servo angle**

*wow! amazing!*

So I had to do a lil more work than changing **one wire...**

The **redesigned system** was way better:

**Joystick x-axis position** (still a pot just a *fancy one*) **→ offCenter from 0** (we actaully use the middle of the pot as 0 and the ends as -1 and 1) **→ Velocity → Servo Speed**

Now moving the joystick slightly makes super small adjustments, while pushing it further makes the servo go wayy faster.

To make this feel better, I made my own **acceleration function** instead of just using a simple linear movement.

It took some experimenting, but the final result **feels surprisingly nice**.

## Current Prototype

Right now S.U.P. has:

- A working **joystick-controlled servo system**
- **Acceleration-based movement**
- **Speed settings**
- **Adjustable servo limits**
- **Joystick deadzones**
- A **debugging system** because manually adding and removing `Serial.println()` gets old **really quickly** :I

[Now look at the **Super Cinematic** picture of that MG90S Servo]

## Things I Learned

A few things I learned while making this:

- Joysticks are basically just **two pots and a button**
- The ESP32 ADC reads **voltage**, not "positions"
- **Tuning acceleration functions** is really worth it for making the movement *feel* good

## What's Next?

The next steps are:

- Add joystick **calibration**
- Find servo **minimum and maximum angles** (if even possible idk)
- Figure out how to detect **180° vs 360°** servos
  - If there is no good way, add **manual selection**
- Start building the **OLED menu system** (tbh sounds like the most interesting and hard part at the same time 🙏)
- First, I need to stop avoiding the display and actually learn how **I2C OLEDs** work LoL (and pray I find some good source where people don't talk 10 minutes about the wiring and proceed to paste a 300 line snippet in the IDE without explaining a single line)