# about

literm is a terminal emulator for Linux first and foremost, but it is also
usable elsewhere (on Mac). The design goal is to be simplistic while still
providing a reasonable amount of features.

If you'd like to talk to people using literm, feel free to pop onto #literm on
freenode.

# status

This probably won't eat your homework, but I'd treat it with a dose of caution
all the same. It has seen a fair amount of real world use through fingerterm,
but there may still be bugs lurking.

This having been said, feel free to give it a shot and file bugs - ideally with
pull requests, but at the least with as much information about how to reproduce
them as possible.

# technology

literm is implemented using QML to provide a fast, and fluid user interface.
The terminal emulator side is in C++ (also using Qt). It is exposed as a plugin
to allow reuse in other applications or contexts.

# building

    qmake && make

.. should get you most of the way there, assuming you have qmake & Qt easily
available. After that, run:

    ./fingerterm

# history

literm started off life as fingerterm, a terminal emulator designed for
touch-based interaction, written by Heikki Holstila for the Nokia N9 and Jolla's
Sailfish OS devices.

I decided to take it a bit further, specifically: giving it a desktop-friendly
interface, adding some odd features here and there, and fixing things up as I
found them.

It is also partly inspired by [Yat, a terminal emulator by Jørgen
Lind](https://github.com/jorgen/yat), but it does not share any code from it.

