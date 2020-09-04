#!/usr/bin/env python

"""Basic script to read the serial output from an Arduino and dump it to standard out,
with time stamps attached. This makes it like the Arduino IDE serial monitor, only
it's easier to log to files, use over SSH, etc. but it also doesn't have options for
sending data to the Arduino, yet.

It assumes the /dev/ttyUSB0 port and baud rate. Very not fancy.

You might need to `pip install pyserial` and adjust your user/group permissions if you
can't read the tty.

Conrol+C to quit.
"""

import datetime
import serial

ser = serial.Serial(port='/dev/ttyUSB0', baudrate=9600)

while True:
    try:
        time = datetime.datetime.now().isoformat()
        line = ser.readline().decode('ascii').strip()
        print(time, line, flush=True)
    except KeyboardInterrupt:
        # Quit on Control+C.
        break
    except Exception as e:
        # Ignore other exceptions.
        pass

ser.close()
