# TODO List #

##Action Plan##
 1. Write machining code using jscam
    * Get usage experience
    * Manual everything(feed rate etc.)
    * Reproduce existing gcode programs
 2. ...?

## High Level ##
 * Longer term design goal (on branch postp)
    - Change from current design into independant tools
    - Have a tool that reads gcode and outputs structures objects
    - Simulator works on this structured objects - not tied to machine object.
    - Plain gcode could be analysed.
    - Implement gcode parser.
 * Simulation
    - Ensure feed rate passed to simulation is normalised.
    - Implement additional simulation steps.
 * Path expansion
    - Use path::expand_rotary in Machine when pure rotary motion is detected.
    - UVW mapped into cartesian space?
 * Check for stock intersection for Linear & Rapid movements
    - Rapids (Positioning)
       - Rotate tool along all angular axes
       - glide (minowski sum) rotated tool with box formed by linear axis movement
       - Perform intersection tests.
    - Linear / Arcs (Material removal)
       - Output
          - Chip load per tooth
          - Cutter engagement
          - Cutting time.
       - Performance
          - *How to measure cutting performance?*
          - Path for each flute is trochoidal.
          - Calculate the path that flute tip passes through material.
          - Gives simulated chip load per tooth.
          - add experiment for flute path
 * Higher order curves
 * Complete Tool class - apt_cutter branch
    - APT CUTTER style mill tool definition
    - http://www.dtpm.unipa.it/emc/it/apt_doc/manual/prog_toolpath.html#figure69
    - Generate model from this definition.
 * Complete Stock class
    - Needs material properties
 * Plunge motion
    - Will be implemented as primitve (allows optimisation and clarification of intent)
 * Ability to explode stock into individual objects when cut
    - How should user specify which part remains?
    - geom::explode not needing intermediate Polyhedron_3 objects (spliting based on Nef shells alone)
 * geom
    - polyhedron_t
       - Store and update Polyhedron_3
       - Return copy of Nef_Polyhedron_3
    - Meshing 
       - Current meshing parameters are too computationally expensive.
    - Geometry
       - Validate primitive geometry
       - Correcting narrowing warnings.
    - Wrapper for CGAL/AABB_tree
       - Needed for motion planning (ray intersection tests)
 * Represent workholding, clamps, etc. (for intersection tests)
 * Restore position
    - Ensure move is safe (no intersection with tool or clamps)
    - First version rapid to clearance plane, move, then rapid back to previous z height.
 * Automatic/heuristic tool selection
 * Global Machine accuracy
    - Use when generating tool and stock models.
 * Math
    - Correct faulty equality ops for math::point_3 and math::vector_3.


## Code ##
 * Ensure exceptions do not change state (to allow recovery)
 * Add Interface stability test.
 * Split tests (one per source file)
 * Add expected output to tests and verify.
 * User boost::operators to define operatiors on types.
 * Review boost::test for unit tests
 * Axis / Offset UDL type.
 * Tool::Model() -vs- Stock.Model

