# S.U.P. Journal

## 23rd July 2026 - Project Take-off

Achieved today:  

- Made the S.U.P. repo
- Created the project structure
- Wrote down README.md
- Set up Arduino IDE Sketch
- Set up the physical breadboard and modules (need to find a way to track IRL progress too)
- Chatted with ChatGPT about what the project would feature and sources to learn from
- Obviously wrote this jounal :D

Next up:

- [x] Learn how servos work
- [x] Learn how to control servos with the ESP32
- [x] Write the first servo test
  - ~~[ ] Sweep 0° → 180°~~ (skipped, made a cooler prototype instead)
  - ~~[ ] Move to a chosen angle (e.g. 74°)~~ (the pot already does that)

Session time today (approximate):

- 43 min from hackatime (morning session) doing most of today's work
- 10-15 min setting up hardware
- ~30 min planning project stuff with ChatGPT
- 10 min from hackatime (evening session) writing this daily journal

## 24th July 2026 - First Prototype
### Version: v0.1.0-proto

Achieved today:

- Made the servo move using a pot(potentiometer, but "pot" sounds a lot cooler)
- Fixed a hardware bug: I accidentally connected the pot to 5v instead of 3.3v and wondered why the servo stopped rotating when you twist the pot too much.

Next up:

- [ ] Repair the OLED display
  - [ ] Solder the broken wire

- [x] Learn joystick input
  - [x] Read X axis
  - [x] Read Y axis
  - [x] Read the push button

- [ ] Refresh OLED programming

- [ ] Build the first menu
  - [ ] Manual Servo Control
  - [ ] Full 0°-180° Sweep
  - [ ] Change Default Servo Pin

Session Time today:

- 10 minutes (tracked by hackatime) setting up hardware (only had 1 long USB cable so I had to think of something for the power supply)
- ~40 minutes from hackatime (actually Lookout, not sure if there's a difference) 

What I learned today: (Proposed by ChatGPT)

- for controlling servos on your ESP32 you gotta use `ESP32Servo.h` (and not `servo.h`!) 
- ESP32 analog inputs only read from 0 to 3.3V
- `map()` is used to make a pot voltage value into an angle for the servo


Notes: 

- Guess we skipped the simplest "set static servo angle" tests (yay!)
- Also got an idea for a better overview section: something like "this project is for people who want to easily test their servos but either can't wait for delivery or don't want to spend 10$ on a servo tester"

## 25th July 2026 - Learning Break 
### Version v0.1.1-learning

Achieved today:
- Learned how Joysticks work
- Learned how to read the X and Y axis of a joystick
- Learned the basics of most serial protocols (UART, IIC, SPI, CAN) and what each wire in them does
- Watched a tutorial about Wokwi: the ESP32/Arduino/Pi Pico web board simulator  
Link [here](https://wokwi.com/)
- Watched a couple Adafruit 128x64 Display tutorials, but most of them just spent 5 minutes  putting 4 wires and copy pasted some 400+ line "pre-written by them" code. (Interesting... )

Next up:

- [x] **Wire** the Joystick Module
- [x] **Connect** the X axis to the servo motor
- [x] Make an **acceleration-based rotation system** instead of the current static pot pos to servo angle 
- [x] **Fine-tune** the system so it works for both **small adjustments** and **quick rotations**

Session Time Today: (Sadly NONE of this was tracked nor eligible to be tracked for Stardance)

- ~1hr watching youtube videos:
- - [ESP32 OLED Project You Can Finish Today!](https://youtu.be/wiu2lC0JfTA?si=8zMZFI8RQJd8i_Y4)
- - [How to Use OLED Displays with ESP32 Boards | ESP32 with OLED display](https://youtu.be/7XdNR2ou_-g?si=TwcTZVJ8-935wedq)
- - [Getting Started with ESP32: Joystick tutorial](https://youtu.be/yGU9-jSJi9M?si=FXxS9wNZByFKbsuI)
- - [Serial Communications Explained: UART, I2C, and SPI](https://youtu.be/IyGwvGzrqp8?si=gcXxU1ZzALxxgCwK)
- - [The ESP32 Simulator you've been looking for!](https://youtu.be/8vFMAr2jyUY?si=Ht9inRxQ0Dmj-j-o)
- - [How 2-Axis Joystick works ? | 3D Animated 🔥| Detailed Explanation](https://youtu.be/UUlXBcakcdI?si=F5DYa_fxEkv0lKy_)
- - [Arduino with OLED Display | Full Tutorial](https://youtu.be/___p9JYbTc0?si=4MZdKXGIyOfaMXU7)

## 26th July 2026 - Working Joystick Acceleration
### Version v0.2.0-roto

Achieved today:

- Finished a **reliably working Prototype** of the **acceleration-based** rotation system
- Added configurable variables at the top for easier **fine-tuning**  
These include: 
- - **Min/Max angle**
- - **Max Speed**
- - **Speed Multiplier**
- - And more...
- Added **Deadzones** (Still not added to the top vars, but that's some 5 minute work I'm not ready to face up XD)
- Added a **debugger function** (so I don't have to write Serial.println() lines and remove them every time :I) 
- - cuz adding debugging lines means u need a higher delay, otherwise you have to unplug your board and hold BOOT when plugging it back to enter download mode to send new code sketches every single time...

Next up:
- [ ] Debugging function for joystick **calibration**
- [ ] Add a way to **find the Min and Max Angle** of a servo
- [ ] Find a way to know if the servo is **180° or 360°** (Skip to manual select if there's no way to know)
- [ ] Start working on the **menu**
- - [ ] (First, at least try to learn **how these displays work** LoL)

What I learned today:

- Using a linear-based acceleration system brings 80% of the enjoyment with 20% of the work, but adding a slight boost at the end of the function (like 1/16 * x^2) makes the joystick feel more smooth.
- Lowering the whole function using a negative x^0 is a good idea since it gives the start of the joystick smaller velocity for small rotations 

Session Time today:

- ~3 hours (Recorded with Hackatime)
- - ~120 min working on the code and fixing bugs
- - ~45 minutes writing docs
- - ~15 minutes searching for the Youtube videos' links and polishing the journal

Notes: 

- While trying to come up with some cool text-based emoji i found this chubby guy **:I**

## Random Event

- Rolled 784,858,176 on Stardance RNG
- Ranked #2 at the time
- Sadly not the NASA ticket, but still a ridiculous roll :D