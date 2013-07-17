# TODO List #

##Action Plan##
 1. Write machining code using jscam
    * Get usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Implement rotation and translation for nef::polyhedron_t (on branch simulation)
    - Done, needs test verification.
 * Check for stock intersection for Linear & Rapid movements
    - *How to measure cutting performance?*
    - Define Inputs and Outputs
       - Input
          - Discretised path
          - Mutable Stock
          - Mutable Tools
          - Feed Rate (per step or per segment)
          - Spindle speed
       - Output
          - Annotated path
             - Chip load per tooth
             - Volume of material
             - Cutter engagement
    - Simulation
       - Tasks
          - Performs analysis on a specified path.
          - Updates stock model.
          - Possibly updates tool object (wear etc.)
          - Provides information on cutting performance.
    - Take path, expand tool along path with rotation, subtract from stock.
       - Expanding tool along path complete, rotation needs to be checked. (branch simulation)
       - Subtraction from stock pending.
    - Path is discretised, perform analysis at each step.
    - Analysis must be possible on multiple linear / angular segments
       - Non-plane aligned arcs / higher order curves will be represented as a collection of line segments.
       - Does the boundary between motions need to be preserved?
          - No. Paths with similar cutting parameters will be folded.
    - Volume of material removal
    - Cutting speed
    - Performance
       - Path for each flute is trochoidal. 
       - Calculate the path that flute tip passes through material.
       - Gives simulated chip load per tooth.
       - Compare with data (tables? calculated from Material hardness?) for MRR.
    - Interface for feedback?
       - Multiple moves must be able to be coalasced into one for analysis.
       - I.e. use stack push and pop of state to test different configurations to find optimal.
       - Return vector of stats for each step
          - volume, forces, engagement, etc.
          - New functions that augments existing path provided.
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
    - Stock location tolerance - expect stock locations to be +- this tolerance to avoid rapids into stock when moving close to it.
    - ~~Top of stock check at Z0~~
       - ~~Unnecessary.~~
 * Represent workholding, clamps, etc. (for intersection tests)
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Automatic/heuristic tool selection
 * Ability to explode stock into individual objects when cut
    - nef::explode exists - needs testing and use in Stock

## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * Add Interface stability test.
