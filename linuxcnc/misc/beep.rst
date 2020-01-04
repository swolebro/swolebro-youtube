Many desktop computers have the ability to issue a beep sounds, direct from the
motherboard. This isn't a full blown speaker or anything, just a single,
usually annoying, beep sound. This can be helpful to have  when you want your
machine to get your attention.

First, install the ``beep`` package::

    sudo apt-get install beep

Then, add the following to your modules::

    # /etc/modules
    snd_pcsp

Some other modules may already be listed in that file. That's OK, just add
``snd_pcsp`` to the list. It's not loaded by default precisely because it is
such an annoying sound.

Once you reboot, the ``beep`` command should work. If not, your hardware might
require different modules - some people have reported ``pcspkr`` doing the
trick.
