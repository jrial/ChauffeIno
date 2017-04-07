#!/usr/bin/env python
import time
from datetime import datetime

import serial
ser = serial.Serial('/dev/ttyUSB4', 115200, timeout=1)
while True:
    dt = datetime.now()
    ser.write('node: 10\n')
    ser.write('date: %s\n' % dt.strftime('%Y-%m-%d'))
    ser.write('time: %s\n' % dt.strftime('%H:%M'))
    ser.write('temp: %.1f\n' % 22.5)
    ser.write('burn: %i\n' % False)
    ser.write('valv: %i\n' % False)
    ser.write('transmit\n')
    ser.flush()
    line = True
    while line != '':
        line = ser.readline().strip()
        print line
    time.sleep(5)
