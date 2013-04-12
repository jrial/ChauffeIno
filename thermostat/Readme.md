Chauffeino Thermostat
=====================

The short
---------
The thermostat is a simple [Moteino](http://lowpowerlab.com/moteino/) unit with an LCD and two push-buttons. The LCD displays the room name and the current and requested temperature. The push buttons can be used to override the programming to demand more or less heat.


How it works
------------
The thermostat gathers temperature readings from a Dallas Semiconductor [DS18B20](http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf), buffers them, and sends them to the collector on a regular interval. The collector then passes that data along to the controller. The thermostat can receive a response from the controller through the collector, which it can use to update the values on its LCD. The unit will be sleeping between measurements, but can be woken from the sleep state when one of the push buttons is pressed. It will wait for 3 seconds after the last keypress, after which it will send the "requested temperature set to X°C" event to the controller. Overridden temperatures are valid until the next programming sequence.

Design Questions
----------------
Timing is something that needs to be determined. One approach is to have every node check in once every minute or every two minutes, and remain dormant for most of its time. Another approach is to collect data, but only check in when the temperature deviates more than 0.5°C from its last reported value in order to save battery, and at least once every 5 or 10 minutes so that the controller knows the sensor is still alive. And perhaps every minute whenever the burner on event is received so the controller can build a high-res warm-up curve for the room in question.

Display example, upon receiving the response "you are in zone Living Room, program sequence 17; your temperature demand is 22°C, which overrides the programmed temperature upwards and the valve for your room is open", it might display something like:

    20.3/22.0 C      ^
    Living Room      *

This is interpreted as:

* 20.3&deg;C current temperature
* 22.0&deg;C requested
* Programming temporarily overridden upwards (^)
* Zone called "Living Room"
* Burner on (*)
