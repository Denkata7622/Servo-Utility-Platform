# Devlog #6 - THE "ALMOST FINISHED"

### [Some original greeting here]!

Time goes by, so does progress on this calibrator tool. I really started to like it to be honest. It has been really fun making all the additional systems around the calibration, such as the "Debug", "Settings", (soon "About" too), all the UI polish and case handling. Even the NVS storage setup (basically the real ESP32 storage instead of the RAM-like one it usually uses) teached me a lot and I'm proud of how far this project has come.

But enough yapping.  

## So, what did i update in this 1 week?

#### \- Well... A LOT.

If i had to list it:

- Made the settings finally work!
- Fixed a bug where option scroll started when there was no need at all
- Saved all settings, preferences and user profiles to permament NVS storage 
- Changed ALL delay() instances with millis() (Where possible/wanted). Just this took like 4 hours :I
- FINALLY Taped the Laser module to the servo bracket!
- Changed a breadboard because for some reason my jumper wires started losing lots of voltage and the connectors were really loose for some reason
- Polished the UI, A LOT. I probably plan to change from normal lists to Flipper Zero-style menu soon.
- Added a "Simulate Errors" feature in the "Debug" menu to test if my error and case handling works well
- Fixed multiple smaller bugs...

I think that's pretty much it, there could be more smaller things I don't remember rn (sorry about that).

## Notes

- Sadly went a little above 10 hours this time D; \- I sill have lots of things to do - mainly polish messages, small inconsistensies, ect.

- I kinda gave up on adding a keyboard - it is pretty unnecessary and will add a lot more cases to handle for almost no benefit.

- These days I will be filming the final video for the v1.0 release!

- I accept any ideas from you for future features and filming advice.
