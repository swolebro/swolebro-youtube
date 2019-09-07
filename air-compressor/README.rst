`Related video. <https://youtu.be/rOZVkkA4Oik>`_

Upload the sketch to an Arduino, and wire it up as shown in the video. You
might need to add some libraries to the Arduino IDE for it to compile, and move
it to its own folder (thanks to inane restrictions impoosed by the IDE).

The Python script is very basic, and just reads off from ``/dev/ttyUSB0``,
which is what your Arduino should show up as when plugged into Linux. If that's
not correct, change the script. You might also need to install pyserial (``pip
install pyserial``) and add yourself to the ``uucp`` user group. Just dick
around with it for a while and read some Linux/GRBL/Arduino tutorials if you
get stuck.

Since the Python script just prints the data to stdout, you can do stupid stuff
like access it from another machine on the network (via SSH) *and* pipe it to a
file, with a command like ``ssh raspberrypi python serial-dump.py | tee
output.txt``.

Hell, you might even have luck skipping Python altogether and doing ``ssh
raspberypi tail -f /dev/ttyUSB0 | tee output.txt``, but then you won't get
timestamps included.
