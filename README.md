# WavepassReader

USB HID card reader (cardIO) for official Konami eAmusement slotted and wavepass readers (ICCA, ICCB, ICCC) 

# Demo

https://www.instagram.com/p/CK-HtCKBKH1/

# Acknowledgments

This is essentially a fork of [CrazyRedMachine/PN5180-cardio](https://github.com/CrazyRedMachine/PN5180-cardio).

ACReal_IO and BT5 acio implementation code was very useful, especially for encrypted polls, thanks a lot :)

# Supported devices

USBHID code has been tested on Leonardo, and Pro Micro.
Serial output has been tested on Arduino Mega.

Reader models ICCA (slotted reader), ICCB and ICCC have been tested and work perfectly.

## ICCA

This card reader only supports ISO15693 cards and doesn't support the newer encrypted polling mode (which means you have to change `bool g_encrypted = true;` to `false` in `WavepassReader.ino`).

It tends to malfunction when powered directly by the Arduino, so I recommend using a 12V external PSU for this model. 
 
ICCA has a card locking mechanism which is supported by this firmware : 

- It will lock a recognized card, and will automatically reject an unrecognized one after a user-configurable delay (`#define EJECT_DELAY` on top of `ICCx.cpp`, defaults to 1 second).

- There is an alternate mode where it doesn't lock invalid cards at all, which can be enabled by uncommenting the `#define LOCK_ONLY_ISO15693` on top of `ICCx.cpp`

- If you have a keypad, the unused blank key will trigger a card eject (this behavior can be inhibited by commenting the `#define KEYPAD_BLANK_EJECT` line)

- You can wire a button to GND and the `PIN_EJECT_BUTTON` (configurable in WavepassReader.ino, defaults to gpio 7), then this button will serve as an eject button for the card.

## ICCB, ICCC

These newer readers support both ISO15693 and FeliCa cards. In order to be able to read FeliCa cards you have to keep `bool g_encrypted = true;`. You can still deactivate the encrypted mode in which case only ISO15693 will be detected (there's no real use for this since ISO15693 are also detected in encrypted mode).

While powered by 12V in the cabs, they worked perfectly fine for me when directly powered from the arduino 5V pin.

## passthrough mode

There is a passthrough mode which can be activated by setting `bool g_passthrough = false;` to `true` in `WavepassReader.ino`. In this mode the arduino just acts as a TTL to USB adapter (so no real use for this one either) and will forward any message to and from the card reader to the computer.

# Press key on boot

I repurposed an old motherboard as a bartop, and got error messages on boot with "Press F1 to continue".

I took advantage of this card reader to press F1 for me.

Uncomment the `#define PRESS_KEY_ON_BOOT` to use this feature. You can set the key, the delay before press and the press duration.

# Keypad support

Keypad is supported for all reader models through the 10-pin central connector. 

The official konami keypad is a 4+3 pin matrix.
On the central connector, only 7 of the 10 pin are populated :

ABCDxxEFGx

Pins E F G are for columns 1 2 3 while pins A B C D are for rows 4 3 2 1.

If you don't have a konami 10-key, you can use any kind of 4+3 matrix keypad, as long as you rewire it to the correct pins.

here's an example with a commonly available cheap keypad :

![keypad_pinout](https://github.com/CrazyRedMachine/WavepassReader/blob/main/diagrams/keypad_pinout.png?raw=true)

# Pinout

There are 3 connectors on the card reader unit, with markings on the PCB indicating the first and last pin :

- 3-pin large connector is for power input (Vcc on pin 2, GND on pin 3) 

- 10-pin center connector is for the keypad matrix (see previous section for pinout)

- 3-pin small connector is the serial interface (RX on pin 1, TX on pin 2, GND on pin 3)

ICCB and ICCC Wavepass readers work perfectly fine on 5V straight from the Arduino (even though they are powered with 12V in cabs).

ICCA (slotted reader) didn't work well when powered by the arduino. While it worked from a 5V wall PSU, I recommend using 12V for them.

For all models you **need a rs232 to TTL adapter** (I use a max3232 based one like HW-027 or the Keyestudio RS232 shield).

- Plug the wavepass serial interface into the max3232 on rs232 side, then TTL side into the arduino Serial1 (pin 0/1 on leonardo/pro micro, pins 19/18 on mega).
- Wire 3.3v from the arduino into the max3232 adapter on TTL side
- (ICCA) Wire a 12v PSU into the wavepass reader
- (ICCB, ICCC) Wire 5v from the arduino into the wavepass reader

Here are examples using an ICCB :

With the HW-027 pcb

![leo_pinout](https://github.com/CrazyRedMachine/WavepassReader/blob/main/diagrams/leo_pinout.png?raw=true)

or if you don't want to solder on a small pcb and would rather use the Keyestudio shield : 

![shield_pinout](https://github.com/CrazyRedMachine/WavepassReader/blob/main/diagrams/shield_pinout.png?raw=true)

(**Note:** ICCC follows the same pinout, while ICCA uses a 12V PSU into the power connector instead of going to arduino 5V/GND). 

# How to use

## USBHID

- Download zip
- (ICCA) set `bool g_encrypted = false` in the code, (optional) uncomment the `#define LOCK_ONLY_ISO15693` in ICCx.cpp
- flash the firmware
- unplug the arduino
- connect the reader to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. `spicetools -cardio`).

The keypad should be recognized as an additional USB device.

# Todo

- spiceapi support
- make passthrough and encrypted mode switch via HID messages
