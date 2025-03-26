# Open Arcade Coder

An open source repository for adding functionality to Tech Will Save Us's ESP32-powered Arcade Coder RGB input matrix.

I am in no way a hardware person so please assume that I could be wrong about many things here. Am very happy to talk about it and help whoever out if possible. If you'd like to assist me I'd be very grateful to receive a couple more of these devices to test with and make projects with.

## Community

Following [a very generous sharing of dozens of Arcade Coders](https://www.reddit.com/r/LinusTechTips/comments/1jgk9cr/how_can_i_stop_all_of_this_from_becoming_ewaste/), there is now a community of developers working on developing software for this board on [Discord](https://discord.gg/QWCUWKAqts) (if this URL does not work please message someone in the reddit thread who has joined).
If you join the community please make an effort to share your discoveries in a public space for future preservation.

## How it works

- 9 daisy chained HC595 chips are daisy-chained and used to control 24 RGB LEDs (two of the twelve rows) from 3 ESP32 pins
- 6 multiplexing channels are used to control the rows which the HC595s are in control of, this is done with an IC2012 (LS138) which requires 3 pins; the other 2 channels uses are unknown
- buttons on matrix are partially handled by 6 GPIO pins which are grouped in correspondence with the multiplexed LED channels (seemingly also partially handled by values being distorted by red LEDs)
- 2 status LEDs on the rear of the device are directly controlled by a GPIO pin each, as is the home button on the side
- accelerometer which I have not tested

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

## Library

The library code is usable now with some examples. I have not dug at all into optimisations around rendering so it's all very glitchy at the moment. There are very likely some low level logic errors. If I get around to optimising things I will document the functions.

For now I would recommend checking out the example code and work from there. If you've worked with things like this before I'd appreciate you sharing any suggestions to improve the code (e.g. the extensive flicking when checking if buttons are pressed).

### Progress on built in functionalities

- LED matrix: implemented, needs brightness controls and refinements
- matrix inputs: can reliably detect the mux channel of an input, some visual and logical bugginess with individual button detection. Can handle multiple buttons at once but gets increasingly unreliable
- accelerometer: no work done on
- built in LEDs: can be directly controlled
- battery levels, status etc: unknown
- home button: seems very erratic, possibly an issue with my board

### Examples

#### buttonLog.ino
Each time a button press is logged, the color of the corresponding LED is updated.

![gif of first working version of demo](/assets/buttonLog.gif)


#### displayDemo.ino
Displays a variety of demo images, swap between images by pressing buttons on the board (currently only 5 images and off)

![gif going through several visual demos of static images](assets/displayDemo.gif)

#### originalDemo.ino
A dumping ground from when I was debugging the board. Does not rely on the library so may be useful for sloppy experimentation

### Still to do

- refine input thresholds; possibly investigate debouncing, etc
- investigate accelerometer
- there's likely a battery level pin?
- Verify wifi can be used
- Get a grasp on the timings well enough to handle dimmings and make demos showcasing a more diverse range of colours

## Notes

### Why write custom code for a 5 year old product no one knows?

The associated app that came out with this project spent a shockingly small amount of time on the app store and the lack of an Android version means the product is currently ewaste. It's a 12x12 battery powered RGB matrix and a pretty professional enclosure that has a 12x12 associated input pins so I thought it had tons of potential.

I actually made efforts to contact various people associated with the company and got no response at all asides from a group who partnered with them and had no info. So I decided to buy one (thank you person on Vinted who gave me one for the cost of postage!) and see if I could figure it out. It was a very pleasant surprise to find an easily interfaced ESP32 on board.

The bulk of these for sale online are pretty overpriced considering it's e-waste.

### Notes on existing software

- relies on iOS application (no longer available, not backed up anywhere and seemingly relied on a closed server to work)
- connects to app via Bluetooth, uses BLE. Likely does not use wifi.
- has serial output (baud rate 230400), seems to only pump out data when turned on (someone already logged that [here](https://gist.githubusercontent.com/borjaburgos/1237dca02802669eef4d2aaa83393478/raw/56f897978e48b74dfc80464bbd04a0f7fbda72c9/gistfile1.md)) and when the home button is pressed
- default software allows pressing of individual buttons on board to create patterns, input seems laggy (could probably improve this at the cost of accuracy) but can handle multiple buttons at a time (including along the same row)
- rather loud high pitched noises come from board while displaying LEDs, this has been somewhat manageable in my custom code by keeping the HC_595 latch pin high as much as possible
- pin 25 seems to trigger a board reset, I'm not 100% on this

I backed up the firmware, here are some details to compare against:
- backup command: `esptool.py -b 115200 --port [port_name] read_flash 0x0 0x400000 flash_4M.bin`
- firmaware size(bytes): `4194304`
- md5: `2ddb762b450e464f76411ca34376f282`

### Reverse engineering process

I'm not a hardware person much at all so I stumbled onto the TTL interface just by knowing it matched my FTDI232 flashing device's 6 pin layout (RX and TX pins are labelled). 

The pins on the board follow this order:
`DTR|TX|RX|VCC|CTS|GND`
GPIO_0 is also exposed nearby these pins, by grounding it when turning on the board you enter boot mode.

Following that I identified some basic pin actions via debugging code. At this point I had to go deeper and follow traces on the board with a multimeter. I successfully found most (maybe all) of the pins associated with the main LED drivers (one ICN2012 and 9 HC595 chips). After struggling for ages to find out much of anything about the ICN2012 chip I spotted "138" on some Chinese documentation, this led me to understanding its arguably just a wrapper for an LS138 chip so I was able to get consistent output. Via printing out the values of the HC595 daisy chain I was able to determine the format of the data being sent to the LEDs. On account of button presses involuntarily triggering red LEDs at a very dim level I figured there was a chance the red LEDs could determine which buttons were pressed via process of elimination and this fortunately worked (I have no clue if it is the correct way to do it but it works).

I struggled a lot finding useful information on forums and such but consulting with some chatbots helped a ton. The fact the chips on hand are so primitive made it extremely easy to catch out when they were leading me down a wrong path with nonsense.

### Project ideas

As many pins are in use I'd suggest just cutting off some of the ones in use, the easiest ones would be the two status LEDs but you could also cut off the accelerometer or even the button matrix pins (these seem like they'd be very useful if the full process can be figured out though)

- add a microphone and make a visualiser; I'll probably do this
- visual info board (clock, weather, whatever's possible with ~12 characters and an image; have demo layouts in displayDemo.ino)
- Bluetooth controller: I'm sure there's a bunch of ESP32 OBS controller Stream Deck type projects this could be user for
- ESP-NOW intercom system between multiple Arcade Coders, possibly also doable with bluetooth but less neat. 
- Could potentially make some kind of primitive instrument if you attach a synthesis module, with ESP-NOW multiplayer multi device games would be possible. The 12x12 matrix isn't ideal for that though
- if nothing else you've got a battery powered ESP32 here on a device that's getting thrown out by most the people who own it currently, it's worth taking a punt on that for a fiver!

Potentially available pins:
- GPIO 12-15 and 25 all seem to have floating values by default and I couldn't see any trace signs
- GPIO 26-27 are probably connected to the motion sensor
- GPIO 22-23 control status LEDs so could potentially be swapped, these feel useful for testing though imo
- GPIO's 1 and 3 could conceivably be used if you're not doing any serial output
