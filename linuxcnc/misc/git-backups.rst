Git for Backups
===============

Near the end of `this video`_, I did a brief overview of how to use ``git``, a tool
that software developers use for tracking changes in code. Since your LinuxCNC
configurations are a type of code too, it makes sense to track them with it as well.
Hell, it's what I use to organize and track the changes in this repository right here.

.. _this video: https://www.youtube.com/watch?v=_c1o8tqoR-0

I'll leave the tutorial to my video, and to the numerous other good resources out there
(such as the `Pro Git e-book`__ and the billion Q&A's you'll find on `StackOverflow`__),
but I did want to cover the idea of using ``git`` for automated backups. The way I'm
using it means manually adding files, committing changes, etc. which is nice and organized,
but is also some work, and I know some people can be lazy about that sort of stuff.

.. __: https://git-scm.com/book/
.. __: https://stackoverflow.com/

An alternative is to use a cronjob to handle it for you. Cronjobs run on a set
schedule, so you don't have to think about it. It'll mean your history isn't as
curated, but it's better to have a messy history than no history.

If you look in your ``/etc/`` directory, you should already have a few subdirectories
such as ``/etc/cron.hourly/`` and ``/etc/cron.daily/`` - if not, then you may need to
``sudo apt-get install anacron`` in order to get the tool that makes those (and see
``installing-software.rst`` for tips on that with LinuxCNC 2.7).

Anything executable in those directories will get run on an hourly, daily, etc. schedule.
So we can write a little script...

.. code-block:: text


    #!/bin/bash
    # file: /etc/cron.daily/git-backups

    user=swolebro # you probably want to change this bit
                  # make it whatever you normally get when running `whoami` at a terminal

    # If running as root, re-run as regular user.
    # This keeps you from making files that require root permissions to read
    # or accidentally botching up any system files too too badly.
    if [[ $EUID -eq 0 ]]; then
       exec sudo -u ${user} ${BASH_SOURCE[0]}
    fi

    # Make a directory in your home for these snapshots.
    cd ~
    mkidr -p git-backups
    cd git-backups

    # If you've already initialized a repository here, then this
    # has no effect. Totally safe.
    git init .

    # Copy over everything from the linuxcnc directory.
    # Note that rsync is picky about its use of slashes, and it has loads of options.
    # It's generally a much more powerful tool than cp though, and has the added speed
    # benefit of not recopying files that haven't changed since the last time it ran.

    rsync -rptgoLD --exclude='.git/' ~/linuxcnc ./

    # This will make a full copy of the linuxcnc directory in ~/git-backups/linuxcnc,
    # even if you're using a symlink for it, except for any pre-existing .git history there.
    # That means you can still keep a curated history there, and use this repo just
    # for automated dumps.

    # You can also copy any other files you want into there, or dump other data that may
    # prove useful later, eg:

    # dpkg -l > installed-packages.txt

    # will give you a list of all the installed packages and their versions.

    git add .   # Just add everything, unless a .gitignore blocks it.
                # The latest version of the blocked file will still exist here
                # but it won't be tracked with git at all.

    git commit -m "autocommit snapshot"     # basically useless message

    # Not a bad idea to set up a remote to push stuff too, just in case your machine dies.
    # This can be another computer on your network, a public or private GitHub project, etc.
    # Just be careful to not go publishing any sensitive info or company proprietary on
    # the web for everybody to see. Hell, even a USB stick plugged into the machine can work,
    # but you will likely lose that too if your shop burns down.

    git push origin


Run through that, line by line, and make sure it works and makes sense to you. Be careful
as well, since this has the potential to either eat up a bunch of hard drive space (eg.
if you go committing videos of your machine running) or publishing sensitive info (eg.
if your store any passwords in a text file that you copy and push to github). Make use
of the ``.gitignore`` files, as discussed in the video, and test to make sure they do
what you want.

Saving this script in the cron directory will probabably require ``sudo``, and then
you'll also have to ``chmod +x /etc/cron.daily/git-backups``, so that Linux knows the file
is executable.

Most importantly, go to your backups directory every so often, run a ``git log -p`` and
make sure any changes you've done lately have shown up in there.
