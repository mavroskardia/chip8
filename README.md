CHIP-8 Emulator
===============

My version of the Chip-8 interpreter.

It runs just about everything fine, but there are almost certainly a few bugs
that I missed. It's my first C++11 project, so I probably missed out on some
cool new features and/or did things in a slightly backwards way, so feedback
about that is very welcome.

I wrote this using the CodeLite IDE in Windows 7 with the MinGW compiler
(v. 4.8.1).

Building
--------
Opening the workspace in CodeLite and building from there should work best, but
hopefully the Makefile CodeLite produces is also reasonable. When I have a
chance to get back on my Linux box, I'll see what I'm missing to make it easy
to compile there as well.

Dependencies
------------
Requires SDL2 [https://libsdl.org] and SDL2_ttf
d[https://www.libsdl.org/projects/SDL_ttf/]