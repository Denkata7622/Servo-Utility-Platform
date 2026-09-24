# S.U.P. Journal

## 23rd July 2026 - Project Take-off

### Achieved today  

- Made the S.U.P. repo
- Created the project structure
- Wrote down README.md
- Set up Arduino IDE Sketch
- Set up the physical breadboard and modules (need to find a way to track IRL progress too)
- Chatted with ChatGPT about what the project would feature and sources to learn from
- Obviously wrote this jounal :D

### Next up:

- [x] Learn how servos work
- [x] Learn how to control servos with the ESP32
- [x] Write the first servo test
  - ~~[ ] Sweep 0° → 180°~~ (skipped, made a cooler prototype instead)
  - ~~[ ] Move to a chosen angle (e.g. 74°)~~ (the pot already does that)

### Session time today (approximate)

- 43 min from hackatime (morning session) doing most of today's work
- 10-15 min setting up hardware
- ~30 min planning project stuff with ChatGPT
- 10 min from hackatime (evening session) writing this daily journal

## 24th July 2026 - First Prototype
### Version: v0.1.0-proto

### Achieved today

- Made the servo move using a pot(potentiometer, but "pot" sounds a lot cooler)
- Fixed a hardware bug: I accidentally connected the pot to 5v instead of 3.3v and wondered why the servo stopped rotating when you twist the pot too much.

### Next up

- [x] Repair the OLED display
  - [x] Solder the broken wire

- [x] Learn joystick input
  - [x] Read X axis
  - [x] Read Y axis
  - [x] Read the push button

- [x] Refresh OLED programming

- [x] Build the first menu
  - [x] Manual Servo Control
  - [x] Full 0°-180° Sweep
  - [x] Change Default Servo Pin

### Session Time today

- 10 minutes (tracked by hackatime) setting up hardware (only had 1 long USB cable so I had to think of something for the power supply)
- ~40 minutes from hackatime (actually Lookout, not sure if there's a difference) 

### What I learned today (Proposed by ChatGPT)

- for controlling servos on your ESP32 you gotta use `ESP32Servo.h` (and not `servo.h`!) 
- ESP32 analog inputs only read from 0 to 3.3V
- `map()` is used to make a pot voltage value into an angle for the servo


### Notes 

- Guess we skipped the simplest "set static servo angle" tests (yay!)
- Also got an idea for a better overview section: something like "this project is for people who want to easily test their servos but either can't wait for delivery or don't want to spend 10$ on a servo tester"

## 25th July 2026 - Learning Break 
### Version v0.1.1-learning

### Achieved today
- Learned how Joysticks work
- Learned how to read the X and Y axis of a joystick
- Learned the basics of most serial protocols (UART, IIC, SPI, CAN) and what each wire in them does
- Watched a tutorial about Wokwi: the ESP32/Arduino/Pi Pico web board simulator  
Link [here](https://wokwi.com/)
- Watched a couple Adafruit 128x64 Display tutorials, but most of them just spent 5 minutes  putting 4 wires and copy pasted some 400+ line "pre-written by them" code. (Interesting... )

### Next up

- [x] **Wire** the Joystick Module
- [x] **Connect** the X axis to the servo motor
- [x] Make an **acceleration-based rotation system** instead of the current static pot pos to servo angle 
- [x] **Fine-tune** the system so it works for both **small adjustments** and **quick rotations**

### Session Time Today (Sadly NONE of this was tracked nor eligible to be tracked for Stardance)

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

### Achieved today

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

### Next up

- [x] Function for joystick **calibration**
- [x] Add a way to **find the Min and Max Angle** of a servo
- [ ] Find a way to know if the servo is **180° or 360°** (Skip to manual select if there's no way to know)
- [x] Start working on the **menu**
- - [x] (First, at least try to learn **how these displays work** LoL)

### What I learned today

- Using a linear-based acceleration system brings 80% of the enjoyment with 20% of the work, but adding a slight boost at the end of the function (like 1/16 * x^2) makes the joystick feel more smooth.
- Lowering the whole function using a negative x^0 is a good idea since it gives the start of the joystick smaller velocity for small rotations 

### Session Time today

- ~3 hours (Recorded with Hackatime)
- - ~120 min working on the code and fixing bugs
- - ~45 minutes writing docs
- - ~15 minutes searching for the Youtube videos' links and polishing the journal

### Notes

- While trying to come up with some cool text-based emoji i found this chubby guy **:I**

## Cool Stuff

- Rolled 784,858,176 on Stardance RNG
- I'm Ranked #2 at the time (got #3 in the final ranking)
- Sadly didn't get the NASA ticket, but that was very coool :D

## 27th July - 13th August (Yeah, a long time)

### Version v0.3.0-colo

### Sorry For missing such a HUGE chunk of time in my journal, but lots of things happened, some of which will remain unfolded, plus I didn't work like a whole week straight and got into the loop where I knew I hadn't worked on the project recently so I just couldn't start again...

### Main events that happened (Project-Related)

- Working OLED Menu with sections
- - Manual Control
- - Calibration
- - Debug
- - Settings (Still in Progress)

- Moved the working rotation prototype to its own "Manual Control" option
- Added LASER Calibration which works by attaching a laser to your servo head and pointing it to a marking on your wall (preferably on a paper if you dont want to permamently have a marking to calibrate your servos XD), then it rotates a certain number of degrees and you measure the distance from the mark. The code takes the distance from the servo to the wall and the one from the marking to the laser dot (both measured manually) and calculates the real angle that corresponds to the digital one.
- Added a Debugger that shows the joysticks offCenter values (for debugging purposes and mainly to see if there's a problem with the wires, which happens occasionally)
- Added a cool scrollbar at the side with a wheel that moves based on the focused option
- Made it so each menu option only uses 1 row and never goes to the next (even when focused) 

### TODO:

 - [x] Add a speed, distance and click state in the joystick debug menu
 - [x] Add a working settings menu
 - [x] Add a way to take the calibrated lists and make new angle maps for the servo
 - [x] Use calibrated servo angle maps for any movement
 - [x] Make menu options longer than 1 row (char count depends on screen but for my 128x64 it's 20) start a scroll so u can see the whole name (right now it just trims them...)

### Session time

- UNKNOWN - Probably a lot (total sums up to 25 hours as of now)

### Notes

- Gotta talk with the admin when I am ready to ship the project, so he gives me the stardust (actually there was recently a change in the way hours get saved and we had like 2 weeks to transfer out work from hackatime to the new platform, but i accidentally transferred all work at once and got over the 10 hour cap limit...)

- I wrote this as a form of redemption so i don't feel as guilty for the big time hole in the journal :D

## 13th August - 1st September (Again, a long time)
### Version v0.6.0-rygi

### Again, I missed a lot of time in journaling, sorry about that, but this time I worked most of the days I could. Lost a week in time, cuz I was on a vacation, but I'm back now :D

### Main events

- Basically finished almost all TODOs from the last log except the settings menu - haven't even started that one yet... and some additional debug functions which may prove useful later.

- - Added Calibration Lists (So now you can torture multiple servos to extract their calibration data XD).

- - Added Working customScroll funtion for when options are longer than max chars per line (it looks cool <3).

- Added Custom Commands like `/n` to prettyPrint which directly end the current line and start a new one (Not Tested)

- Lots of bug hunting, debugging and UI Polish + Code Sanitazing (idk if that's how it's called XD)

### TODO

- [ ] Actually TEST the calibration workflow and check if it really works with a real servo
- [x] Add an X-axis animation to the Settings menu when changing variables
- [ ] Add a way to name list slots
- [ ] Make list slots dynamically generated instead of hardcoded
- - [ ] Add an "add/make new slot" option

- [ ] Make an on-screen keyboard or use a library

## 2nd September - 10th September 2026

### Version v0.7.1

### Main events

- Finally started properly using ESP32 NVS storage with `Preferences`.

  - Added a `savedSettings` namespace for user settings.
  - Added a `ServoProfiles` namespace for calibration data.
  - Settings now have dedicated `saveSettings()`, `loadSettings()` and `resetSettings()` functions.
  - Hardware configuration such as servo and joystick pins is included in the saved settings.

- Finished the Settings menu enough for actual interaction.

  - Boolean settings can be toggled using the joystick.
  - Integer and double settings can be adjusted directly.
  - Added a system that detects the type of each setting and edits it accordingly.
  - Added a centered value display with `>>` and `<<` while editing.
  - Settings are saved when leaving the value editor.

- Expanded joystick configuration.

  - Added configurable X and Y deadzones.
  - Added X and Y axis inversion.
  - Added axis swapping.
  - Added joystick off-center calibration variables.
  - Added more adjustable values for the acceleration system.

- Expanded the calibration system massively.

  - Added six calibration profile types:

    - Basic
    - Accurate
    - Servophile
    - Super Servophile
    - MEGA Servophile
    - OVERKILL Servophile
  - Added six slots for each calibration profile.
  - The calibration system now selects both a profile type and a slot before collecting data.
  - Calibration data is stored in separate arrays for each profile.
  - The reference angles range from 5 points in Basic all the way to 129 points in OVERKILL.

- Continued working on C.E.A. LASER Calibration.

  - The selected calibration profile determines how many measurements are taken.
  - The selected slot receives the generated calibration angles.
  - Added a wall-distance measurement stage before calibration.
  - Added joystick-controlled distance input instead of requiring a separate measuring device.
  - Each measured distance gets converted into an angle using trigonometry.
  - The measured angles are accumulated to build the final calibration map.
  - Added screens for displaying saved distances and calculated angles.

- Added calibrated servo movement.

  - Manual Control now converts the digital joystick angle into a calibrated real angle before sending it to the servo.
  - Added interval-based interpolation between calibration points.
  - The display shows both the digital angle and calibrated real angle.
  - The selected calibration profile affects the servo's minimum and maximum angles.

- Expanded the debugging system.

  - Added a proper Debug menu with several debugging categories.
  - Added simulated error messages.
  - Added error IDs for invalid menus, menu options, servo profiles, settings, axes and setting types.
  - Added a deliberately ridiculous upload-style error message for testing the text renderer.
  - Added debugging screens for calibration list and slot selection.
  - Added joystick debugging for X/Y values, directions and button state.

- Continued improving the OLED UI.

  - Kept the custom scrolling system for long menu options.
  - Added more reusable display functions.
  - Added custom `/n` line-break commands to `prettyPrint()`.
  - Added paginated text output.
  - Added a delay system that lets the user skip pages with the button.
  - Added configurable scrolling and page timing variables.

- Added hardware reconfiguration support.

  - Changing saved hardware settings now triggers `reconfigureHardware()`.
  - The servo gets detached and reattached using the new pin.
  - The button pull-up is reconfigured as well.

### What I worked out

- ESP32 NVS is useful for keeping settings between restarts instead of hardcoding everything.
- A calibration profile is essentially a reference-angle list plus a list of measured real angles.
- More calibration samples means more detailed mapping.
- Having a map between each 2 sample points lets the servo use angles between measured samples much better.
- The calibration system is now structured around reusable profiles and slots instead of one hardcoded calibration.
- The OLED UI is becoming more of an actual interface than I imagined.

### TODO

- [x] Properly save calibration profiles to NVS
- [x] Properly load calibration profiles from NVS
- [x] Add profile reset functionality
- [ ] Test the complete laser calibration workflow with a real servo
- [ ] Implement Pulse Width Calibration
- [x] Implement Joystick Calibration
- [x] Add settings X-axis animation
- [ ] Add names for calibration slots
- [ ] Make calibration slots dynamically generated
- [ ] Add an option to create new slots
- [ ] Add an on-screen keyboard or suitable library
- [ ] Finish the About section
- [ ] Finish remaining Debug menu functions
- [x] Clean up and polish the settings system
- [ ] Finish the persistent servo-profile system
- [ ] Test calibration data across multiple servos

### Notes

The project has gone from "servo + joystick prototype" to something starting to resemble an actual standalone servo testing tool.

The calibration system especially got huge. There are now 6 calibration methods with 6 slots each, ranging from 5 measurements per slot to 129.

Also, I somehow ended up writing my own tiny text rendering system for a 128x64 OLED. :D

## 15th September 2026 - Major Improvements

### Main events

- Spent several sessions restructuring and expanding the code instead of adding isolated features.
- Built a reusable settings system for different variable types, including temporary values, validation and NVS saving.
- Worked on persistent calibration storage, including saving, loading and resetting six calibration types with six slots each.
- Expanded the calibration system to support up to 129 measured points and added interpolation between calibration points.
- Worked on runtime hardware configuration so saved pin settings can be applied without changing the code.
- Expanded the debugging system with error IDs, error simulation and dedicated debugging menus.
- Built and refined my own OLED text system for scrolling, line breaks and paginated messages.
- Spent a lot of time debugging, testing individual systems, fixing small bugs and connecting all the separate systems together.

### Notes

Did lots of things but I am still wondering whether i should continue adding functions or finally ship the v1.0.
