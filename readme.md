OpenDaisy
=========

OpenDaisy is a project for developing firmware for alternative controllers for daisywheel typewriters.  This project targets the low end portable daisywheel typewriters by companies such as Smith Corona, Brother, Nakajima, and others.  OpenDaisy tries to use the existing circuitry for driving the various sterrper motors, solenoids, and inputs.

The goal is to replace the existing microcontroller in these machines with a modern Arduino compatable board and extend the functionality of these machines.

These machines could then become wifi enabled, used as a printer, teletype, or some other completely original use.


Progress
========

So far, for my model, a Smith Corona DLE 250, I have been able to type out characters in from the program memory.  This took a few weeks of occasional testing to work out the hardware bugs.  I still need to code the keyboard input and also establish some protocol for sending data to/from the computer.


Other Projects
==============

Other projects exist to interface with these old typewriters, but they all interface with the keyboard and send keystrokes and do not give you complete control of the various parts.  This project is a complete reimplementation of firmware designed to give you complete control of the machines.

As such, a considerable amount of work is just tracing the board, testing, and reverse engineering how it works.


Development
===========

I am using PlatformIO for development as opposed to the Arduino IDE.  To use the Arduino IDE you will need to restructure the files and also manually install the dependencies.


Dependencies
============
https://github.com/pololu/pushbutton-arduino
Do not install AccelStepper from the main source.  You will need to use my fork, which is included.



Future
======

I don't expect that this particular project will be able to be used on significantly different models.  It may be a useful base to extend for other projects.


I'm not a professional developer by any means, so if you happen to see odd/bad/wacky code, please be gentle :p.


Leif