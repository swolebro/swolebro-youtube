VNC Remote Desktop
==================

A VNC server can let you access your LinuxCNC installation remotely, with all
the graphical bells and whistles. This is nice for either working on that
computer while sitting in the office (vs. the shop), and it can let you use a
single laptop for accessing all of your machines, as opposed to keeping a
keyboard, mouse, and monitor at each one.

In my experience, controlling my CNC installation via a VNC connection has been
very stable, with two exceptions.

Firstly, there is a slight jitter in the stream of keystrokes going from my
laptop to the CNC. This is imperceptible when typing, but it does make
continuous jogging with the arrow keys kinda spazzy. Either use the +, -
buttons in AXIS or use an XBox 360 controller (plugged directly into the CNC
machine via USB) as workarounds for that.

Secondly, if my laptop is very close to my plasma cutter (eg. resting on top
it), then my wifi connection will drop out while the plasma is running. At
distances of more than 3 feet, this is no longer an issue. If you have a
particularly noisy plasma, this may be more of an issue for you.

With those caveats out of the way, lets get on to setting this up. First, while
still using the standard keyboard, mouse, and monitor::

    sudo apt-get-install vino-server
    vino-preferences # launches a menu; enable control, disable confirmations

Now you should be able to launch the following, and have a VNC server running::

    /usr/lib/vino/vino-server

Run this command as your regular user, not as root.

You will need a VNC client on your laptop in order to connect to it.
Personally, I like Remmina, which is available in the package manager for most
Linux distros. If your client computer is running Windows, Mac, or ChromeOS,
you might have luck with `TightVNC <https://www.teamviewer.com>`_ or
`TeamViewer <https://www.teamviewer.com>`_.

At this point, you should make sure you can connect with whatever client
you're using to the LinuxCNC desktop. You might need the LAN IP of your
LinuxCNC machine, which you can get from running ``ip addr`` and looking
for the one labeled ``192.168.xxx.xxx``. Before going too much further,
you should look into setting up a static IP or having your router provide
DNS (so you can ``ping`` your LinuxCNC machine by it's hostname).

The static IP method is usually simpler and can be set by going to
``Application Menu > Settings > Network Connections > select connection >
Edit IPv4 Settings``` and changing ``Automatic (DHCP)`` to ``Manual``, and
assigning an IP. You might also be able to configure a "static DHCP assignment"
in your router settings if the previous method doesn't work. This way, LinuxCNC
still asks the router for a random IP every time, but the router goes
and gives it the same one (not very random).

Next, we want to make Vino start every time after you log in. So, create
this file:

    .. code-block:: text
        :name: ~/.config/autostart/vino-server.desktop

        # ~/.config/autostart/vino-server.desktop
        [Desktop Entry]
        Type=Application
        Name=Vino VNC server
        Exec=bash -c "/usr/lib/vino/vino-server"
        NoDisplay=true

Restart your LinuxCNC, log in, wait a few seconds, and see if your VNC
client can connect again. If not, try ``ip addr`` to make sure the IP
didn't change, and then try ``ps aux | grep vino`` to make sure the
server is running. Once you got it working, carry on.

Next is to make your account log-in automatically. LightDM is the program
that prompts you for your username and password at boot, so we can tell it
to autologin by adding making sure the following are set:

    .. code-block:: text
        :name: /etc/lightdm/lightdm.conf

        # /etc/lightdm/lightdm.conf
        [LightDM]
        start-default-seat=true

        [SeatDefaults]
        autologin-user=YOUR_USERNAME
        autologin-user-timeout=0

Fill in your username in the above. Also, don't remove lines that are currently
there, and not-commented-out. Just add those three settings under the two
appropriate section headers. The section headers should already be present in
the file. Editing this file will require ``sudo``.

Now, if your reboot, the computer should log in on its own, and you should be
able to VNC in a few seconds later.

Next, is to get it to log in without a monitor attached any longer. Normally,
X11 (the program responsible for displaying everything) won't even start unless
there's a monitor attached, so we need to install an imaginary one::

	sudo apt-get install xserver-xorg-video-dummy

Then create this file, describing the dummy monitor:

	.. code-block:: text
		:name: /etc/X11/xorg.conf.d/50-virtual-screen.conf

		# /etc/X11/xorg.conf.d/50-virtual-screen.conf
		Section "Device"
		    Identifier  "Configured Video Device"
		    Driver      "dummy"
		EndSection

		Section "Monitor"
		    Identifier  "Configured Monitor"
		    HorizSync 31.5-48.5
		    VertRefresh 50-70
		EndSection

		Section "Screen"
		    Identifier  "Default Screen"
		    Monitor     "Configured Monitor"
		    Device      "Configured Video Device"
		    DefaultDepth 24
		    SubSection "Display"
		    Depth 24
		    Modes "1024x800"
		    EndSubSection
		EndSection

Now you should be able to shut down the computer, disconnect the monitor, mouse,
and keyboard, power it back on, wait a minute, then VNC in.

I find it helpful to have the computer beep once the VNC server is ready. That way,
I know when I'm able to connect, and don't need to keep retrying over and over until
it works. For instructions on how to enable the beep command, see ``beep.rst``,
and then use the following in your autostart file::

		Exec=bash -c "(sleep 10; beep)& /usr/lib/vino/vino-server"

Note
====

Some users like using ``x11vnc`` as their VNC server on general Linux systems,
but I have no experience with that, let alone on LinuxCNC.
