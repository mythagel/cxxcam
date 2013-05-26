# TODO List #

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

##Action Plan##
 1. ~~Arcs are a required primitive~~
 2. Write some machining code using jscam
    * Get some usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 3. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Interface changes
    - ~~Experience shows that access to properties is needed~~
    - ~~Create interface with machine object with properties for tool, feed_rate etc.~~
    - ~~Allows read and write access through normal interface.~~
 * ~~Create comments from js.~~
 * ~~Work offsets (G54, etc)~~
    - ~~More important than previously imagined.~~
 * Auto feedrate / spindle speeds
    - ~~Enhance spindle to report spindle speed requests outside of some specified tolerance. (i.e. 30,000rpm request for machine with max 500rpm)~~
 * Complete Tool class
    - Needs nef model loading / ~~generation~~
    - ~~Store properties for different tool types~~
 * Complete Stock class
    - Needs model loading & ~~generation~~
    - Needs intersection tests
    - Needs material properties
    - Top of stock check at Z0
 * Represent workholding, clamps, etc. (for intersection tests)
 * Toolpath expansion & intersection tests for rapids
 * Material removal volume for each operation
    - Calculate volume of material that tool can remove and ensure that volume is appropriate.
 * ~~Arc motion~~
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Restore position
    - Ensure move is safe (no intersection with tool or clamps)
    - First version rapid to clearance plane, move, then rapid back to previous z height.
 * GCode Generation
    - ~~GCode generated to json array~~
    - ~~Export generated GCode as lines and sequences of words (needed for js post-processing)~~
    - ~~Export generated GCode as string~~ (Unneeded - can be generated in js from json)
    - Aligned comments within a gcode block
    - ~~Generate according to Variant (e.g. gcode case and precision are unimplemented)~~
       - ~~Implement 6 digit precision output.~~
    - Document in comments relevant parameters (Stock dimensions, position, tools used, etc.)
 * Machine zero & Limits
 * One file is the complete part
    - With multiple operations underneath
    - Should be able to generate gcode for ops individually.
    - one file is NOT the complete object - only a single part.
 * LOGO / Turtle style interface for experimentation (implemented 100% in js)
 * Output image of toolpath (js or cxxcam)

## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * ~~Change private / protected static methods to free functions.~~
 * ~~Formalise result of nef glide when result is not 2-manifold~~
