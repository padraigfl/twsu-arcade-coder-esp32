# Open Arcade Coder

An open source repository for adding functionality to Tech Will Save Us's ESP32-powered Arcade Coder RGB input matrix.

I am in no way a hardware person so please assume that I could be wrong about many things here. Am very happy to talk about it and help whoever out if possible.

## Why write custom code for a 5 year old product no one knows?

The associated app that came out with this project spent a shockingly small amount of time on the app store and the lack of an Android version means the product is currently ewaste. It's a 12x12 battery powered RGB matrix and a pretty professional enclosure that has a 12x12 associated input pins so I thought it had tons of potential.

I actually made efforts to contact various people associated with the company and got no response at all asides from a group who partnered with them and had no info. So I decided to buy one (thank you person on Vinted who gave me one for the cost of postage!) and see if I could figure it out. It was a very pleasant surprise to find an easily interfaced ESP32 on board.

## How it works

- 9 daisy chained HC595 chips are daisy-chained and used to control 24 RGB LEDs (two of the twelve rows) from 3 ESP32 pins
- 6 multiplexing channels are used to control the rows which the HC595s are in control of, this is done with an IC2012 (LS138) which requires 3 pins; the other 2 channels uses are unknown
- buttons on matrix are partially handled by 6 GPIO pins which are grouped in correspondence with the multiplexed LED channels (seemingly also partially handled by values being distorted by red LEDs)
- 2 status LEDs on the rear of the device are directly controlled by a GPIO pin each, as is the home button on the side
- accelerometer which I have not tested yet

### Notes on existing software

- relies on iOS application (no longer available, not backed up anywhere)
- connects to app via Bluetooth, uses BLE. Likely does not use wifi.
- has serial output (baud rate 230400), seems to only pump out data when turned on (someone already logged that [here](https://gist.githubusercontent.com/borjaburgos/1237dca02802669eef4d2aaa83393478/raw/56f897978e48b74dfc80464bbd04a0f7fbda72c9/gistfile1.md)) and when the home button is pressed
- default software allows pressing of individual buttons on board to create patterns, input seems laggy (could probably improve this at the cost of accuracy) but can handle multiple buttons at a time (including along the same row)
- rather loud high pitched noises come from board while displaying LEDs, this has been somewhat manageable in my custom code by keeping the HC_595 latch pin high as much as possible
- pin 25 seems to trigger a board reset, I'm not 100% on this

### Pinout

HC595:
- Data: GPIO5
- Clock: GPIO17
- Latch: GPIO16
- OE: GPIO4 

ICN2012 (LS138, controls multiplexing to allow control of all 144 RGB LEDs):
- A0: GPIO19
- A1: GPIO18
- A2: GPIO21
- E1: 3.3v
- E2: GND (after a chain of resistors)

Button inputs (tracks if any button in row is pressed):
- 6 and 12: GPIO32
- 5 and 11: GPIO33
- 4 and 10: GPIO34
- 3 and 9: GPIO35
- 2 and 8: GPIO36
- 1 and 7: GPIO39

Other:
- LED1: GPIO22
- LED2: GPIO23
- Home: GPIO2 (seems unreliable)
- Motion sensor: GPIO27/GPIO26 for possible accelerometer in middle of board

Note: Usage of GPIO4 may impact ability to use Wifi from what I can see, will need to verify and see if it can be worked around.

## Reverse engineering process

I'm not a hardware person much at all so I stumbled onto the TTL interface just by knowing it matched my ESP01 flashing devices 6 pin layout (RX and TX pins were labelled). Following that I identified some basic pin actions via debugging code.

At this point I had to go deeper and follow traces on the board with a multimeter. I successfully found most (maybe all) of the pins associated with the main LED drivers (one ICN2012 and 9 HC595 chips). After struggling for ages to find out much of anything about the ICN2012 chip I spotted "138" on some Chinese documentation, this led me to understanding its arguably just a wrapper for an LS138 chip so I was able to get consistent output. Via printing out the values of the HC595 daisy chain I was able to determine the format of the data being sent to the LEDs. On account of button presses involuntarily triggering red LEDs at a very dim level I figured there was a chance the red LEDs could determine which buttons were pressed via process of elimination and this fortunately worked (I have no clue if it is the correct way to do it but it works).

I struggled a lot finding useful information on forums and such but consulting with some chatbots helped a ton. The fact the chips on hand are so primitive made it extremely easy to catch out when they were leading me down a wrong path with nonsense.

## Still to do

- Write explicit code for simplified matrix rendering (i.e. pass a 12x12 matrix of readable values that will print out as expected)
- Break code out into a library of core utility functions
- Add example code with expected outputs for others to test against
a group are pressed by how high the value is
- investigate motion sensor
- there's likely a battery level pin?
- Verify wifi can be used
- Get a grasp on the timings well enough to handle dimmings and a more diverse range of colours.

## Project ideas

As many pins are in use I'd suggest just cutting off some of the ones in use, the easiest ones would be the two status LEDs but you could also cut off the accelerometer or even the button matrix pins (these seem like they'd be very useful if the full process can be figured out though)

- add a microphone and make a visualiser; I'll probably do this.
- visual info board (clock, weather, whatever's possible with ~12 characters and an image)
- Bluetooth controller: I'm sure there's a bunch of ESP32 Stream Deck type projects this could be user for
- ESP-NOW intercom system between multiple Arcade Coders, possibly also doable with bluetooth but less neat. 
- Could potentially make some kind of primitive instrument if you attach a synthesis module, with ESP-NOW multiplayer multi device games would be possible. The 12x12 matrix isn't ideal for that though
- if nothing else you've got a battery powered ESP32 here on a device that's getting thrown out by most the people who own it currently, it's worth taking a punt on that for a fiver!

Potentially available pins:
- GPIO 12-15 and 25 all seem to have floating values by default and I couldn't see any trace signs
- GPIO 26-27 are probably connected to the motion sensor
- GPIO 22-23 control status LEDs so could potentially be swapped, these feel useful for testing though imo
- GPIO's 1 and 3 could conceivably be used if you're not doing any serial output
