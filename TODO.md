# TODO List #

##Action Plan##
 1. Write machining code using jscam
    * Get usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. Complete model generation (i.e. remove material from stock object)

## High Level ##
 * Path expansion
    - Correct faults with angular movements
       - With zero linear movement (pure rotary motion)
       - Large angular with small linear
    - UVW mapped into cartesian space?
 * Check for stock intersection for Linear & Rapid movements
    - Rapids
       - Rotate tool along all angular axes
       - improve geom::polyhedron to be able to create from polygons (list of points) (make_polygon)
       - glide (minowski sum) rotated tool with polygon
       - hmmm... polygon or polyhedron....
    - Linear / Arcs
       - *How to measure cutting performance?*
       - Define Inputs and Outputs
          - Input
             - Feed Rate (per step or per segment)
             - Spindle speed
          - Output
             - Annotated path
                - Chip load per tooth
                - Cutter engagement
    - Simulation
       - Tasks
          - Performs analysis on a specified path.
          - Provides information on cutting performance.
          - Cutting time.
    - Cutting speed
    - Performance
       - Path for each flute is trochoidal.
       - Calculate the path that flute tip passes through material.
       - Gives simulated chip load per tooth.
       - Compare with data (tables? calculated from Material hardness?) for MRR.
 * Higher order curves
 * Complete Tool class - apt_cutter branch
    - APT CUTTER style mill tool definition
    - http://www.dtpm.unipa.it/emc/it/apt_doc/manual/prog_toolpath.html#figure69
    - Generate model from this definition.
 * Complete Stock class
    - Needs material properties
 * Plunge motion
    * Will be implemented as primitve (allows optimisation and clarification of intent)
 * Ability to explode stock into individual objects when cut
    - How should user specify which part remains?
    - geom::explode not needing intermediate Polyhedron_3 objects (spliting based on Nef shells alone)
 * geom
    - Meshing 
       - Current meshing parameters are too computationally expensive.
    - Geometry
       - Validate primitive geometry
       - Correcting narrowing warnings.
 * Represent workholding, clamps, etc. (for intersection tests)
 * Restore position
    - Ensure move is safe (no intersection with tool or clamps)
    - First version rapid to clearance plane, move, then rapid back to previous z height.
 * Automatic/heuristic tool selection
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
 * Add expected output to tests and verify.
 * User boost::operators to define operatiors on types.
 * Review boost::test for unit tests

