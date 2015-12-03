Chauffeino Controller
=====================

The Short
---------
The controller is a BeagleBone Black unit, receiving sensor data through the receiver (Moteino). It holds the programming, per room warm-up data and all the temperature logs from the various sensor nodes. It uses this data to decide when to activate the heating circuits and whether to turn the burner on or off.

Configuration happens via a web interface, with possible future expansions including mobile apps.

After receiving data from a thermostat node through the receiver, the controller can send back data such as "you are in zone Living Room, program sequence 17; your temperature demand is 22&deg;C, which overrides the programmed temperature upwards and the valve for your room is open", which the thermostat node can then use to update the values on its LCD. See the README file in /thermostat for more info.

Design Questions
----------------
Need to investigate whether the premise that keeping water at a certain temperature T by turning the burner on when the temperature drops to T-d degrees consumes the same amount of fuel as allowing the temperature to drop to a lower value B, if under the first schedule the burner is turned on n times where T-B = n*d. Or to put this in numbers: do we consume the same amount of fuel when we try to keep the water at 80&deg;C if we have to turn the burner on 12 times during the day whenever the temperature drops to 75, as opposed to letting the water cool down to 20&deg;C and warm it up again to 80&deg;C? If that's the case, the burner can be switched off whenever there's no heat demand, as long as it's turned on again in time for the water to reach its set temperature by the time the next heat demand occurs.

Basic security needs to be built in: when sensor nodes fail to report for prolonged periods of time, the heat flow to those rooms should be disabled, or possibly throttled based on heuristics: I'm thinking room warm-up and cooldown speed data, taking outside temperature into account. Warnings need to be sent to the user (email, sms, ...?) so they can take appropriate action.

Connecting the SainSmart 16-channel Relay Board
-----------------------------------------------

The SainSmart 16-channel Relay Board needs 12V  + Ground on the 12V terminal connector, a +5V input on the 5V pin. The rest of the pins need to be pulled low in order to activate the corresponding relay. Alternatively, you can put ground on the GND pin, and pull the relay pins low. Either way, the relay pins need to be pulled low to activate the relay.

The relays are toggle switches; when they're active (LED on), the middle pin is connected to the left one. When they're inactive (LED off), the middle pin is connected to the right pin.
