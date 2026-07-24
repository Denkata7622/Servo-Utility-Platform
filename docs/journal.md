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

- [ ] Learn joystick input
  - [ ] Read X axis
  - [ ] Read Y axis
  - [ ] Read the push button

- [ ] Refresh OLED programming

- [ ] Build the first menu
  - [ ] Manual Servo Control
  - [ ] Full 0°-180° Sweep
  - [ ] Change Default Servo Pin

Session Time today:

- 10 minutes (tracked by hackatime) setting up hardware (only had 1 long USB cable so I had to think of something for the power supply)
- ~40 minutes from hackatime (actually Lookout, not sure if there's a difference) 

What I learned today (Proposed by ChatGPT):

- for controlling servos on your ESP32 you gotta use `ESP32Servo.h` (and not `servo.h`!) 
- ESP32 analog inputs only read from 0 to 3.3V
- `map()` is used to make a pot voltage value into an angle for the servo


Notes: 

- Guess we skipped the simplest "set static servo angle" tests (yay!)
- Also got an idea for a better overview section: something like "this project is for people who want to easily test their sevrvos but either can't wait for delivery or don't want to spend 10$ on a servo tester"
