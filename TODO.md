# TODO List #

##Action Plan##
 1. Write ~~some~~ more machining code using jscam
    * Get ~~some~~ more usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * *Review handling of rotational axes.*
    - Change velocity to angular_velocity for ABC axes.
 * Implement generic external axis interface and checked internal interface
    - Possibly use templates to generate all possible axis functions.
 * *Review handling of feed rate*
    - Inverse time needs special attention and possible interface change.
 * Implement Dwell
 * *Machine Limits*
    - *All of the limits need to be reviewed based on incorrect assumptions for rotational axes.*
    - Torque at various rpm (Horsepower)
    - Max Feed rate per axis (z is probably slower)
    - Rapid rate per axis (For calculating movement time)
    - Max travel per axis
       - Probably actually useless given that cxxcam has no way of knowing the machine position for the active coordinate system.
 * Machine configuration struct
    - Many configuration parameters together will simplify machine setup.
 * Implement available axes (i.e. XYZ / XYZA / XYZABC, etc.)
 * Different type for Linear vs Rotational axes guarantees safety in the type system rather than manual checking.
 * Auto feedrate / spindle speeds
 * Complete Tool class
    - Needs nef model loading / ~~generation~~
 * Complete Stock class
    - Needs model loading & ~~generation~~
    - Needs intersection tests
    - Needs material properties
    - Top of stock check at Z0
 * Represent workholding, clamps, etc. (for intersection tests)
 * Toolpath expansion & intersection tests for rapids
 * Material removal volume for each operation
    - Calculate volume of material that tool can remove and ensure that volume is appropriate.
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Restore position
    - Ensure move is safe (no intersection with tool or clamps)
    - First version rapid to clearance plane, move, then rapid back to previous z height.
 * ~~Enforce units via type system.~~
    - ~~Already encountering possible issues with mismatched units.~~
 * ~~Push and Pop state.~~

## Code ##
 * Replace Unchecked Position with code from Position_Metric.
 * Ensure exceptions do not change state (to allow recovery)
 * Rename Machine::line_t to Machine::block_t

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
