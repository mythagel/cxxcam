# TODO List #

##Action Plan##
 1. Write machining code using jscam
    * Get usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Current design means that runtime is _unusably_ long.
    - Need to think of alternative designs that improve performance.
 * geom lib implement scale in translate functions
 * geom intersection tests
 * Check for stock intersection for Linear & Rapid movements
    - *How to measure cutting performance?*
    - Define Inputs and Outputs
       - Input
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
          - Provides information on cutting performance.
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
 * Complete Stock class
    - Needs model loading & ~~generation~~
    - Needs intersection tests
    - Needs material properties
    - Stock location tolerance - expect stock locations to be +- this tolerance to avoid rapids into stock when moving close to it.
 * Represent workholding, clamps, etc. (for intersection tests)
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Automatic/heuristic tool selection
 * Ability to explode stock into individual objects when cut
    - nef::explode
    - How should user specify which part remains?
 * Path expansion
    - Validate rotations
       - Correct axes
       - Correct direction
    - UVW mapped into cartesian space?
    - Arc expansion
       - *Implement*
 * Spindle class
    - Torque interpolation
 * nef
    - Meshing 
       - Current meshing parameters are too computationally expensive.
    - Geometry
       - Validate primitive geometry
       - Correcting narrowing warnings.
 * Global Machine accuracy
    - Use when generating tool and stock models.
 * Math
    - Correct faulty equality ops for math::point_3 and math::vector_3.
 * Simulation
    - Ensure feed rate passed to simulation is normalised.


## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * Add Interface stability test.
 * Split tests (one per source file)
 * Improve Arc code (remove duplication)
