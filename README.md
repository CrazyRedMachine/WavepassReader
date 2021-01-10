# WavepassReader

USB HID card reader (cardIO) for official Konami eAmusement slotted and wavepass readers (ICCA, ICCB, ICCC) 

# Acknowledgments

This is essentially a fork of [CrazyRedMachine/PN5180-cardio](https://github.com/CrazyRedMachine/PN5180-cardio).

# Supported devices

USBHID code has been tested on Leonardo, and Pro Micro.
Serial output has been tested on Arduino Mega.

Reader models ICCA (slotted reader), ICCB and ICCC have been tested.

ICCA only supports ISO15693 cards. ICCB and ICCC support both ISO15693 and FeliCa.

# Pinout

ICCB and ICCC Wavepass readers work perfectly fine on 5V straight from the Arduino (even though they are powered with 12V in cabs).
ICCA (slotted reader) worked from a 5V wall PSU but not from the arduino. It's better to use 12V in this case.
In all cases you still need a rs232 to TTL adapter (I use a max3232 based one).

- Plug the wavepass serial through the adapter then into arduino interface Serial1 ( pin 0/1 on leonardo/pro micro).
- Wire 3.3v from the arduino into the adapter
- (ICCA) Wire a 12v PSU into the wavepass reader
- (ICCB, ICCC) Wire 5v from the arduino into the wavepass reader

# Keypad support (coming soon)

Just plug a keypad to your reader and it should work.

# How to use

## USBHID

- Download zip
- (ICCA) set `bool g_encrypted = false` in the code
- flash the firmware
- unplug the arduino
- connect the reader to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. `spicetools -cardio`).

The keypad should be recognized as an additional USB device.

# Todo

- draw wiring diagram
- keypad support
- spiceapi support
- better ICCA support (lock mechanism)
