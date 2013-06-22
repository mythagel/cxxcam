# TODO List #

##Action Plan##
 1. Write ~~some~~ more machining code using jscam
    * Get ~~some~~ more usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Check for stock intersection for Rapids movements
    - *How to calculate polyline path for rotatry axes?*
 * Check for stock intersection for Linear movements
 * Review handling of feed rate
    - Change 1/f minutes to seconds / minutes.
    - ~~Inverse time needs special attention and possible interface change.~~
       - ~~Feed rate value needs to be specified for every move.~~
       - ~~Stored value is not appropriate.~~
       - ~~Feed rate must be set for each linear / angular cut to have meaning.~~
 * Tracking of Modal codes.
    - May not be that necessary as cxxcam tracks these internally.
    - May be useful in js for jscam.
 * ~~Auto feedrate / spindle speeds~~
 * Complete Tool class
    - Needs nef model loading / ~~generation~~
    - APT CUTTER style mill tool definition
    - http://www.dtpm.unipa.it/emc/it/apt_doc/manual/prog_toolpath.html#figure69
    - Generate model from this definition.
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
 * Toolpath expansion & intersection tests for rapids
 * Material removal volume for each operation
    - Calculate volume of material that tool can remove and ensure that volume is appropriate.
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Restore position
    - Ensure move is safe (no intersection with tool or clamps)
    - First version rapid to clearance plane, move, then rapid back to previous z height.
 * ~~Different type for Linear vs Rotational axes guarantees safety in the type system rather than manual checking.~~
    - ~~Implement generic external axis interface and checked internal interface~~
    - ~~Possibly use templates to generate all possible axis functions.~~
 * ~~Machine configuration struct~~
    - ~~Many configuration parameters together will simplify machine setup.~~

## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * Add Interface stability test.

## Design issues ##
 * At what level should the cxxcam interface exist?
   I.e. should the automatic feed rate code exist in js or c++. There are arguments for both - but leaning towards implementing these functions in js. cxxcam must provide a base on which to build these functions.
    - Determine minimal machining primitives upon which js interface can be developed
    - Depends on specification list for jscam - what are the features to support
       * automatic spindle speed and feed rate (non-trivial)
          - cxxcam must provide primitives to aide in this
          - material removal volume
          - cutter engagement
          - have generic interface for algorithms for speed and feed selection so they can be tested. 
             - Will be implemented in js anyway.
       * intersection tests for rapids (semi-trivial)
       * collision tests for workholding
       * machine limits tests
       * checked gcode generation (check spindle is on for cuts, etc.) (trivial)
       * procedural interface for gcode generation (for loops etc expanded by the engine) (trivial)
       * automatic/heuristic tool selection
       * Finished model export
