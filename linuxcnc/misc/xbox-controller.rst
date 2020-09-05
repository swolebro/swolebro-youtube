Using an XBox 360 Controller
============================

The HAL tutorial series goes over how to use an XBox 360 controller
for triggering HAL pins. This functionality should work right out of
the box with LinuxCNC 2.7.14.

That's great and all, but wiring up the HAL pins to use that controller
for jogging would kind of be a pain in the ass, especially when we
already have arrow keys for jogging. Isn't there a way we can use the
controller as a keyboard?

That's called QJoypad.

Of course, after all the nonsense with installing software documented
in ``installing-software.rst``, it turns out that Debian Wheezy has
no package for QJoypad whatsoever. So, what we can do instead is
manually download the package for Debian Jessie (v8), install it,
and hope it's close enough it works. (It does.)

`Here's the page`_ for the package details. Go down to the bottom and
(probably) select the link for the i386 architecture, then pick your mirror.
Note that you're going to want to install the i386 version even if your
processor is 64-bit (aka. amd64 or i686), because the LinuxCNC distro you
downloaded is almost assuredly i386.

.. _Here's the page: https://packages.debian.org/jessie/qjoypad

You can try the amd64 first if you want to make really-really sure. If it's the
wrong architecture, it'll refuse to install at all. No harm, no foul.

The commandline fu for this one::

    wget http://ftp.br.debian.org/debian/pool/main/q/qjoypad/qjoypad_4.1.0-1_i386.deb
    sudo dpkg -i qjoypad_4.1.0-1_i386.deb

Note that we install it with ``dpkg`` rather than ``apt-get`` since we are bypassing all the
dependency checking and package management for this one. (All the dependencies should be
installed already.)

Once it's installed, you can launch QJoypad from ``Application Menu > Games >
QJoypad``. You'll want to double-click the icon that pops up, hit "Add" to
create a named configuration, set all your keys, then hit "Update" to save it.
The "Quick Set" option can help here. When you're done, you want to "Close Dialog,"
**not** "Quit."

Any key you aren't using for a HAL pin is fair game for a keyboard key (ideally
one that maps to a good AXIS shortcut).

I put the joystick on X/Y jog, up/down on the D-pad for Z, the middle X-logo for F1
(e-stop), the button to its left F2 (power on), and the button to its right F3 (go
to the manual control tab). Setting lower thresholds on the joystick for jogging is
a good idea too.

Note #1
-------

While this method is nice and simple, it does have the drawback of basically being
a dumb keyboard. This means if you don't have the window focus on AXIS, whatever
you do have in focus is going to get your keystrokes.

If you are in AXIS, and you want to jog, you must be on the "manual control"
tab - that's why I put F3 on a button.

Note #2
-------

Don't use knock-off XBox 360 controllers. They will not work. Rather, they will work,
and then they will disconnect randomly. Entirely randomly. Not related to the CNC
machine running or any noteworthy event. Just randomly, after as little as ten
minutes or as long as two hours.

When that happens, you can get QJoypad to reconnect by right-clicking its icon and
selecting to reload the joystick devices, but for HAL to reconnect, you need to
restart your AXIS/etc. That bit's a pain in the ass. So, get a real one.

I have accidentally severed the cable on an official Microsoft controller once
already, and replaced it by lopping the USB-C end off one of `these braided,
shielded cables`__, desoldering the stake-ons from the controller PCB, and
soldering on that cable.

.. __: https://www.amazon.com/gp/product/B07SPJV964/

Even with the replacement cable, it does not appear to show the same
disconnection problems that the knock-off controller had. Buy an official one.
