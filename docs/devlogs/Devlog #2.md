Devlog #2 - The OLED Age
SUP!

It has been like a week since the last devlog… and a lot happened to be honest.
(If you don’t wanna read, just go down to see the SUPER cinematic demo I made for you <3 )

If you read the first devlog, you might remember that the plan was basically:

“Make the Servo move”
“Then add a Joystick”

and then:

“Make an OLED Menu”

Well, I just finished the OLED Menu!

THE CODE
This time I spent a LOT more time on the actual code structure (+4 hours wondering why jumper wires (the bare ones with no heads) drop so much voltage, but we don’t talk about that).

The sketch has grown up to like ~700 lines, but more importantly, I started optimising it and making the code reusable instead of just putting everything into loop().

I have broken the code down into independent functions like:

calculateOffCenter()
offCenterToDirection()
buildMenu()
callMenuOption()
drawScrollbar()
...
So now everything is more optimised and easier to expand and debug!

THE MENU
Our menu now consists of:

- "Manual Control"
- "Calibration"
- "Debug"
- "Settings"
I made buildMenu() take an array of options and their length, so I can reuse the same function for all menus.

I also added a scrollbar (cause apparently four lines of text just wasn’t enough).

Calibration Time
This is where things got interesting.

I started working on the Calibration menu.

Right now it has:

- Simple Calibr.
- Pulse Width Calibr.
- C.E.A. LASER Calibr.
- 360deg Servo
- 180deg Servo
- Go Back
Some of these are still placeholders, but LASER Calibration is halfway done.

C.E.A. LASER CALIBRATION
Yes.

I finally found a purpose to my year old laser module :D

The idea is pretty simple.

Tape a laser to the servo head.

Put a piece of paper on a wall.

Point the laser at it.

Move it a certain ammount of degrees (e.g. 5deg)

Then use the distance between the 2 points to figure out how much degrees the servo ACTUALLY moved.

And this is where the math comes in.

If the servo is L centimeters away from the wall, and the laser spot moves X centimeters, we can calculate the angle with:

θ = arctan(X / L)
So instead of trusting the servo when sayin’:

“Yeah, that’s 90° bro”

S.U.P. is able to measure where the servo is actually located.

And the best part is that the user doesn’t need to buy some expensive angle sensor (which is the whole idea of the project btw).

You need:

A laser
A ruler
A piece of paper
A pencil
A wall (I hope you have one C: )
That’s it.

And I think that’s pretty damn cool.

The current prototype goes through:

0°
45°
90°
135°
180°
and lets me enter the measured distance (in cm ofc, no freedom units… ) with the joystick.

It isn’t finished yet, but the basic system is working.

What’s Next?
Finish C.E.A. Laser Calibration
Make the angle calculation automatic
Add proper servo calibration profiles
Work on Pulse Width Calibration
Figure out 180° vs 360° servos
Add joystick calibration
Keep improving the OLED interface
And hopefully make S.U.P. look less like a breadboard project and more like an actual tool.
