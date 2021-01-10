# WavepassReader

USB HID card reader (cardIO) for official Konami eAmusement wavepass readers (ICCB, ICCC) 

# Acknowledgments

This is essentially a fork of [CrazyRedMachine/PN5180-cardio](https://github.com/CrazyRedMachine/PN5180-cardio).

# Supported devices

USBHID code has been tested on Leonardo, and Pro Micro.
Serial output has been tested on Arduino Mega.

Wavepass reader models ICCB and ICCC have been tested.

# Pinout

Wavepass readers work perfectly fine on 5V (even though they are powered with 12V in cabs).
You still need a rs232 to TTL adapter (I use a max3232 based one).

- Plug the wavepass serial through the adapter then into arduino interface Serial1 ( pin 0/1 on leonardo/pro micro).
- Wire 3.3v from the arduino into the adapter
- Wire 5v from the arduino into the wavepass reader

# Keypad support (coming soon)

Just plug a keypad to your wavepass reader and it should work.

# How to use

## USBHID

- Download zip
- flash the firmware
- unplug the arduino
- connect the Wavepass to the Arduino.

Congratulations, your device should work just like a real cardio, use 
your favorite tools instructions to play (e.g. `spicetools -cardio`).

The keypad should be recognized as an additional USB device.
