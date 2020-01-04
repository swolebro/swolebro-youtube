Installing Software
===================

LinuxCNC is based on Debian, a Linux distribution that is also the foundation of
other more popular ones, such as Ubuntu and Mint. To install most software in
this distro, you will use the ``apt-get`` tool. You can read more about that on
its manpages, available by typing ``man apt-get`` into a terminal.

Of note, LinuxCNC v2.7.14 is based on Debian 7 (Wheezy), which is so old it has
passed its end-of-life, and the respositories that ``apt-get`` looks to are no
longer available. In order for you to install additional software, you will
need to update your sources file to the following:

.. code-block:: text
    :name: /etc/apt/sources.list

    # /etc/apt/sources.list
    deb http://archive.debian.org/debian wheezy main contrib non-free
    deb-src http://archive.debian.org/debian wheezy main contrib non-free


All other lines in that file can, and should, either be removed or commented
out (by putting a ``#`` at the start of the line). In particular, if your
sources file has lines refering to ``http.debian.net``, they are no longer
online, and lines referring to ``archive.debian.org/debian-security`` have a
check to prevent you from installing any post-EOL software.

When you install software from ``archive.debian.org/debian``, it will likely be
an old version, and will potentially have security vulnerabilities. In particular,
I'd avoid using the web browser on your CNC machine if I were you.

Once you've fixed your apt sources, you'll need to run ``sudo apt-get update`` to
download the index of available packages. Then ``sudo apt-get install PACKAGE``
will install a given package.

Here are a few that I've personally installed::

    beep      # lets you use the motherboard beeper
    evince    # better PDF reader, edit /etc/mailcap to make it the default
    gedit     # a GUI-based text editor
    gitk      # a GUI-based browser for git repositories
    lshw      # lists hardware
    moreutils # includes tools like tree and sponge
    vim-gtk   # vim w/ mouse ability; requires uninstalling vim-tiny first
    vino      # a VNC/remote desktop server
    xserver-xorg-video-dummy # lets X11 run even when no screen is attached

Other files in here might more detail on how to use these.

Note
====

Supposedly, LinuxCNC v2.8 and v2.9-prerelease can both be run on newer
versions of Ubuntu and Mint bases, but the "stable" release of 2.7.14
cannot, and I don't personally have experience in doing that yet.

If anyone wants to point me towards how that's done, or contribute docs
here on how to do that, please open either an issue or a pull request.
