`Related video. <https://www.youtube.com/watch?v=bbWnj-0GbP8>`_

Basically, this is a dumb sketch for doing PWM as per a potentiometer.

In the video, I showed this waveform on the screen of a DSO138 Chiniseum
Extreme oscilloscope. Then I sent it to a Digital Concepts ACD1U treadmill
power board (originally paird with a DGE1U upper display on a circa 2004 Star
Trac treadmill) in order to control a 3 phase motor. Basically, it's a
special purpose VFD, and I managed to bumble through the pins so I can
use it for more general shop purposes.



Here are some notes I took on the pinout, and of course, I'm typing this up
months after I jotted it down and did the video, so hopefully I can still piece
this together. lol

If you look at the cable, you've got two plugs, with 2x10 pin headers. There's
an orientation nub on both, and if you hold it so that the nub is along the
upper side, then the ribbon cable is marked with red along the left. The ribbon
cable is symmetric in that there's conductivity between each respective pin in
the two 2x10 headers, but it's also asymmetric in that on one end, the ribbon
cable feeds out from the nub side (this plugs in to the upper control panel),
and on the other end, the ribbon feeds out opposite from the nub side (which
plugs into the lower control panel).


In any case, when looking at it with the nub along the upper side and the red
on the left, it's the upper leftmost pin tat eventually plugs into the ground
I identified at 12:34 in the video. We're going to call that pin 1, since it's
labeled as such on the PCB. Then left to right we get to pin 10, and row #2
will be pin 11 to 20 working left to right.

The sockets latch onto the sides and have a notch that lines up with the
orientation nub.

Note that when I'm pointing out pins at times like 26:00 in the video, you're
seeing it all reversed, so pin #1 is the green wire on the upper far right.
Clear as mud, right?

On my breakout board shown in the video, I've got three rows, of 7, 7, and 6
pins. Below, I'm writing down each pin as its labeled on the original PCB, and
then again in parentheses with what row.pin-num it has on my breakout board.
My breakout board is all over the place, just because I was trying to space the
traces out.

1 (3.5) - ground
2 (2.5) - disconnecting this one made the lift move
3 (1.5) -
4 (3.4) -
5 (2.4) - PWM for controlling VFD (approx 0-5k RPM, originally 85Hz, though it appears to work at other frqeuencies)
6 (1.4) - 5V from the VFD board
7 (3.3) - something to do with raising the lift? (when drawn to ground?)
8 (2.3) - normally pulled to 5V by the upper board; need to do likewise (ie. short to pin 6) for it to work
9 (1.3) -
10 (3.2) -
11 (1.6) - some whacky ~300Hz wave around 0.35V; drops to 0.2V when decelerating, goes to 5V when accelerating
12 (2.6)
13 (3.6) - frequency feedback for reading motor speed (150Hz per 1mph)
14 (1.7) -
15 (2.7) -
16 (1.1) -
17 (2.1) - something to do with lowering the lift? (when drawn to ground?)
18 (3.1) -
19 (1.2) -
20 (2.2) -

So clearly, there's still a lot to figure out! Such is the way with shop projects.

And if I can find the files for that breakout board, I'll try and upload that
here too. Hahah.
