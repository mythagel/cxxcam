# TODO List #

##Action Plan##
 1. Write machining code using jscam
    * Get usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Expand start and end tool positions to path
    - Must return sequence of {pos: point, rot: vector3 (quaternions?)} representing tool location and orientation.
    - How to calculate path for rotatry axes?
       - Simple rotation around controlled point.
       - jscam will include post processors to adjust canonical rotations to machine specific configurations i.e. rotary offsets.
 * Check for stock intersection for Linear & Rapid movements
    - Take path, expand tool along path with rotation, subtract from stock.
 * Complete Tool class - apt_cutter branch
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
 * Automatic/heuristic tool selection
 * Ability to explode stock into individual objects when cut - explode branch
    - nef::explode exists - needs testing and use in Stock

## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * Add Interface stability test.
 * Better way of exposing access to nef::polyhedron_t::private_t.
