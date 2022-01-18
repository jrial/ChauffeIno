ChauffeIno
==========

**Superseded by HA configuration. This project is incomplete and no longer maintained.**

Intelligently control the central heating unit (burner, solenoid valves) via wireless thermostats based on [Moteino](http://lowpowerlab.com/moteino/)s (Small low-power radio-enabled Arduino clones) and a [Raspberry Pi](http://www.raspberrypi.org/).

Arduino library requirements:
-----------------------------
* [avr](http://playground.arduino.cc/Main/AVR): comes with avr-libc, required package for arduino-core
* [LiquidCrystal](http://arduino.cc/en/Tutorial/LiquidCrystal): part of the arduino-core package
* [OneWire](http://www.pjrc.com/teensy/td_libs_OneWire.html)
* [RFM12B](https://github.com/LowPowerLab/RFM12B)
* [Dallas Temperature Control Library](http://milesburton.com/Dallas_Temperature_Control_Library)

Arduino hardware requirements:
------------------------------
* [Hitachi HD44780 compatible LCD](http://lcd-linux.sourceforge.net/pdfdocs/hd44780.pdf)
* [Dallas Semiconducor DS18x20](http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
