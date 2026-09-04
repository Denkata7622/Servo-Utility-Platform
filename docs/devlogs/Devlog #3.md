# Devlog #3 - Servo Profiles

Hello everyone,

We just hit a **new milestone** in the development of this project: **Laser Calibration Profiles**

## How do They Work?

Right now we have a menu where you can choose between **3 calibration profiles** each having a different ammount of samples (**the more, the better, but slower** due to the higher ammount of measurements)

- **Basic** (*5 samples*)  
[0, 45, 90, 135, 180]

- **Accurate** (*9 samples*)  
[ 0, 22.5, 45, 67.5, 90,  
112.5, 135, 157.5, 180 ]  

- **Servophile** (*17 samples*)  
[ 0, 11.25, 22.5, 33.75, 45,  
56.25, 67.5, 78.75, 90,  
101.25, 112.5, 123.75, 135,  
146.25, 157.5, 168.75, 180 ]  

- (I plan on adding a **Super Servophile** profile with *33 samples* soon!)

After choosing the profile (and - *in the future* - **one of the 3 slots** each list has), the user will go through with **recording distances** for each angle using a **marking on the wall** (use a pencil or just tape a paper and draw on it)

**That's it!** (At least for the *manual input* XD)

Then the list with colleced measurements gets put in a script and thrown out as a list of angles.

**Now that's it!**

## Next Features

- I plan on adding an **expected measurement** section in the UI so the user has a better idea what the expected output should be during calibration (also **helps with debugging ;)** )

- Turning the List of angles into a **complex map** that can directly **transform digital angles to calibrated** ones

- Actually **using the calilbration profiles** when moving servos

- **Adding all environment variables to settings** so you can change them during runtime!