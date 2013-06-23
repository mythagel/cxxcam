# TODO List #

##Action Plan##
 1. Write ~~some~~ more machining code using jscam
    * Get ~~some~~ more usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Toolpath expansion & intersection tests for rapids
 * Check for stock intersection for Linear & Rapid movements
    - *How to calculate path for rotatry axes?*
    - Complete path functions to expand toolpath
 * Complete Tool class
    - Needs nef model loading / ~~generation~~
    - APT CUTTER style mill tool definition
    - http://www.dtpm.unipa.it/emc/it/apt_doc/manual/prog_toolpath.html#figure69
    - Generate model from this definition.
 * Restore position
    - Ensure move is safe (no intersection with tool or clamps)
    - First version rapid to clearance plane, move, then rapid back to previous z height.
 * Mesh on demand in nef object
    - Invalidate on change.
    - Remesh on request
 * Complete Stock class
    - Needs model loading & ~~generation~~
    - Needs intersection tests
    - Needs material properties
    - Top of stock check at Z0
    - Stock location tolerance - expect stock locations to be +- this tolerance to avoid rapids into stock when moving close to it.
 * Represent workholding, clamps, etc. (for intersection tests)
 * Material removal volume for each operation
    - Calculate volume of material that tool can remove and ensure that volume is appropriate.
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Tracking of Modal codes.
    - May not be that necessary as cxxcam tracks these internally.
    - May be useful in js for jscam.
 * Automatic/heuristic tool selection
 * ~~Allowed axes has to also specify ORDER.~~

## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * Add Interface stability test.

