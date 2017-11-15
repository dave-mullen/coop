# Arduino Chicken Coop
### (c) 2017 Dave Mullen
### License: MIT

Arduino Chicken Coop is a project built on Arduino to provide automatic or manual opening and closing of a chicken coop door.

Disclaimer: I am not an electrical engineer and am not responsible for damage caused to your hardware or belongings as a result of constructing this.

Hardware required includes:
* Pololu.com: #1587 99:1 Metal Gearmotor 25Dx54L mm
* Pololu.com: #1569 Pololu 25D mm Metal Gearmotor Bracket Pair 
* Sparkfun.com: DEV-11113 5v/16MHz Arduino Pro Mini 328 (prototyped on larger Arduino board)
* Sparkfun.com: ROB-09457 Motor Driver 1A Dual TB6612FNG 
* Sparkfun.com: ROB-12548 Shaft - D-Shaft (Stainless; 1/4"D x 12"L) 
* Sparkfun.com: COM-11966 Metal Pushbutton - Momentary (16mm, Red)
* Magnet or micro switches 
* Photocell
* Resistors
* Strong string
* 6V SLA battery

I will not go into construction detail, but it involves mounting the motor above a vertically sliding door with the string attached to the motor shaft and to the top of the door.  The switches are placed in a way to detect stop points for when the door is fully opened and fully closed.  The button (with feedback LED) is mounted outside and allows manual operation and ability to program light levels.  The LED provides system feedback through different flashing patterns.

## Basic features:
* Object-oriented, event-driven classes for system components with the main script coordinating them
* Low power mode used during idle times, especially at night
* Daylight sensor module with Light sampling, timeout periods to avoid reaction to short term changes in light, programming levels to EEPROM
* Button module to handle different kinds of button presses
* Door module to handle interface to motor and door sensors
* Flasher module to handle different types of LED flashing patterns (solid, blink, blink fast, pulse) and reverting to previous

Note that enabling debugging by uncommenting the "#define DEBUG" in module(s) can output information to the serial monitor in the Arduino IDE while debugging but will make the sketch very large and may have other side-effects.

Have fun!

