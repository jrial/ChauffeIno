The thermostat is a simple Moteino unit that gathers temperature readings, buffers them, and sends them to the collector on a regular interval, which passes that data along to the controller. It can receive a response from the controller through the collector, which it can use to update the values on its LCD. It will also have two push-buttons which will act as temp-override-up and temp-override-down buttons. Pressing these buttons will wake the unit from its sleep state and alter the requested temperature on-screen. It will wait for 3 seconds after the last keypress, after which it will send the "requested temperature set to X°C" event to the controller. Overridden temperatures are valid until the next programming sequence. For more elaborate overriding, the controller's web interface needs to be used.

Timing is something that needs to be determined. One approach is to have every node check in once every minute or every two minutes, and remain dormant for most of its time. Another approach is to collect data, but only check in when the temperature deviates more than 0.5°C from its last reported value in order to save battery, and at least once every 5 or 10 minutes so that the controller knows the sensor is still alive. And perhaps every minute whenever the burner on event is received so the controller can build a high-res warm-up curve for the room in question.

Need to decide whether the node should de-noise the data by applying trendline calculations to its sensor data, or if it sends the raw data and leaves the post-processing entirely to the controller. This will in part depend on how noisy the raw sensor data turns out to be.

Display example, upon receiving the response "you are in zone Living Room, program sequence 17; your temperature demand is 22°C, which overrides the programmed temperature upwards and the valve for your room is open", it might display something like:
    20.3/22.0 °C         ^
    Living Room          *
This is interpreted as:
    * 20.3°C current temperature
    * 22.0°C requested
    * Programming temporarily overridden upwards (^)
    * Zone called "Living Room"
    * Burner on (*)
