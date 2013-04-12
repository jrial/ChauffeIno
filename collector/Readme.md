Chauffeino Collector
====================

The Short
---------
The collector is a Moteino connected to the controller (Raspberry Pi). Its task is to listen to incoming sensor reports and pass them along to the Pi. Its secondary function is to collect sensor data from the external temperature sensor, which is passed along to the Pi in the same fashion as the rest of the sensor logs.

Whenever it receives sensor data from a node, it will pass this data to the Pi and listen on the serial interface for the Pi's response. If one arrives, it's sent to the node in question. For this to work, the thermostat will remain listening for a short while after sending its data.