cxxcam
======

CNC/CAM library to act as driver for jscam machining tool

Currently capable of generating GCode (LinuxCNC dialect) for CNC lathes and mills. Nine axis (XYZ ABC UVW) generation is supported however only six are simulated (XYZ ABC). Lathe simulation is incomplete.

This library provides checked and simulated gcode generation with no additional logic (i.e. it does not implement higher level machine operations).

Under active development.

See [jscam](https://github.com/mythagel/jscam) as the canonical example.
