![AutoGoni Logo](https://c1.staticflickr.com/5/4170/34052339760_4a80fe05f5_z.jpg)

# AutoGoni

The AutoGoni is a digital goniometer designed to make the gathering of joint angle measurements easier.

## See the AutoGoni in Action

[![AutoGoni Video Demonstration](https://c1.staticflickr.com/3/2866/33594629674_bba5082603_z.jpg)](https://www.youtube.com/watch?v=PIq4bduV_98 "AutoGoni Video Demonstration")

## Background

The AutoGoni was conceived, developed, and prototyped by students in Marquette University's Biocomputing Design Course under the supervision of David Vitale. Tasked with the goal of filling a need in the medical device industry, the students interviewed various Physical Therapists, then designed all hardware and software functionality seen above.

## Technical Details

The Atmel328p (commonly known as an Arduino) was the microcontroller used in the device, running software modules written for the uC/OSII operating system. The PC Client side records-keeping program was written in Python/TKinter. All other significant hardware modules were bought from Sparkfun. See the "Documentation" folder for more details.

## Using this Repository

### ArduinoSrc

This folder contains all uC/OSII software modules needed to run on the Arduino. Atmel Studio was used to edit the code and flash the 328P's EEPROM with a "J-TAG" unit.

### ClientPCSrc

This folder contains the Python program that can run in coordination with the AutoGoni device. When all 3 buttons are pressed on the AutoGoni, all records from the AutoGoni will sync to this program. This data can be exported to view in Excel, or other medical device software suites.

### Documentation

This folder contains all software and hardware technical specifications, system design specifications, powerpoint presentations about the device, etc.

## Credits

The following students contributed to the completion of this project:

* David Vitale
* Jalen Battle
* Paul Dang
* Ben Durette
* Emmali Hanson
* Justin Hauter
* JP Rivera
* Gleb Skylr
* Kevin Wright

Special thanks to the following individuals for their input, guidance, and support:

* Dr. Robert Scheidt
* Casey Houlihan
* Jeff Wilkens
* Albojay Deacon
