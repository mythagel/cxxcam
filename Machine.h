/*
 * Machine.h
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#ifndef MACHINE_H_
#define MACHINE_H_
#include <string>
#include <memory>
#include "GCodeWord.h"
#include "Axis.h"
#include "Tool.h"

/*
 * The CNC machine itself.
 */
class Machine
{
private:
	static const GCodeWord G00;
	static const GCodeWord G01;
	static const GCodeWord G17;
	static const GCodeWord G18;
	static const GCodeWord G19;
	static const GCodeWord G17_1;
	static const GCodeWord G18_1;
	static const GCodeWord G19_1;
	static const GCodeWord G20;
	static const GCodeWord G21;
	static const GCodeWord G40;
	static const GCodeWord G49;
	static const GCodeWord G54;

	static const GCodeWord G61;
	static const GCodeWord G61_1;
	static const GCodeWord G64;

	static const GCodeWord G80;
	static const GCodeWord G90;
	static const GCodeWord G90_1;
	static const GCodeWord G91;
	static const GCodeWord G91_1;
	static const GCodeWord G93;
	static const GCodeWord G94;
	static const GCodeWord G95;
	static const GCodeWord G97;

	static const GCodeWord M01;
	static const GCodeWord M02;
	static const GCodeWord M03;
	static const GCodeWord M04;
	static const GCodeWord M05;
	static const GCodeWord M06;
	static const GCodeWord M09;
public:
	enum Type
	{
		type_Mill,
		type_Lathe
	};

	enum BlockRestore
	{
		block_PreserveState = 0,
		block_RestoreState = -1,
		block_RestoreUnits = 1 << 0,
		block_RestorePlane = 1 << 1,
		block_RestoreMotion = 1 << 2,
		block_RestoreArcMotion = 1 << 3,
		block_RestoreFeedRateMode = 1 << 4,
		block_RestoreFeedRate = 1 << 5,
		block_RestoreSpindle = 1 << 6,
		block_RestoreTool = 1 << 7,
		block_RestorePosition = 1 << 8
	};

	enum Units
	{
		units_Metric,
		units_Imperial
	};

	enum Plane
	{
	    plane_XY,
	    plane_ZX,
	    plane_YZ,
	    plane_UV,
	    plane_WU,
	    plane_VW
	};

	enum Motion
	{
		motion_Absolute,
		motion_Incremental
	};

	enum FeedRateMode
	{
		feedMode_InverseTime,
		feedMode_UnitsPerMinute,
		feedMode_UnitsPerRevolution
	};

	enum Rotation
	{
		rotation_Stop,
		rotation_Clockwise,
		rotation_CounterClockwise
	};

	enum Direction
	{
		direction_Clockwise,
		direction_CounterClockwise
	};
private:
	struct Private;
	std::shared_ptr<Private> m_Private;
protected:
	void Preamble();

	static GCodeWord AxisToWord(const Axis& axis);
	static double MillFeedRate(double chip_load, int flutes, double spindle_speed);
	static double MillSpindleSpeed(double cutting_speed, double cutter_diameter);

	void UpdatePosition(const Axis& axis);
public:

	/*
	 * TODO design and write an interface for Stock that avoids binding in the interface
	 * have struct defs for each stock type: rectangle, cylinder, import (saved OFF file)
	 */
	Machine(Type type, const std::string& gcode_variant);
	explicit Machine(const Machine& m);
	Machine& operator=(const Machine& m);

	void dump() const;

	// Machine Setup

	bool AddTool(int id, const Tool& tool);
	bool RemoveTool(int id);

	void AddSpindleRange(unsigned long range_start, unsigned long range_end);
	void AddSpindleDiscrete(unsigned long discrete_value);

	// CNC Setup

	// Follow the path given exactly - even if that means coming to a stop
	void AccuracyExactPath();
	// Come to a full stop after each move
	void AccuracyExactStop();
	// Keep feed rate up
	void AccuracyPathBlending();
	// Keep feed rate up and allow deviation by p units from programmed path
	void AccuracyPathBlending(double p);
	// Keep feed rate up and allow deviation by p units from programmed path + fold paths colinear by q units.
	void AccuracyPathBlending(double p, double q);

	void SetMotion(Motion m);
	void SetArcMotion(Motion m);
	void SetUnits(Units u);
	void SetPlane(Plane p);
	void SetFeedRateMode(FeedRateMode f);
	void SetFeedRate(double f);

	void StartSpindle(unsigned long s, Rotation r = rotation_Clockwise);
	void StopSpindle();


	// CNC Machine Setup

	/*
	 * Use SetTool to preload a tool in a tool changer prior to a ToolChange operation
	 *
	 * SetTool(5);
	 * ... other machining operations
	 * ToolChange(5);
	 *
	 * Use ToolChange to set the tool and change it.
	 *
	 * ToolChange(5);
	 */
	void SetTool(int id);
	void ToolChange(int id);

	void NewBlock(const std::string& name);
	void EndBlock(int restore = block_PreserveState);

	void OptionalPause(const std::string& comment = std::string());


	// CNC Motion

	/*
	 * Rapid traverse to the given position.
	 * Note that this is not coordinated motion, each axis could move independently.
	 * TODO: expand the tool motion along all possible paths and check for intersection with the stock
	 */
	void Rapid(const Axis& axis0);
	void Rapid(const Axis& axis0, const Axis& axis1);
	void Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2);
	void Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3);
	void Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4);
	void Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4, const Axis& axis5);

	/*
	 * Coordinated linear motion. Spindle must be on. Feedrate must be non-zero.
	 * TODO: calculate feed rate
	 */
	void Linear(const Axis& axis0);
	void Linear(const Axis& axis0, const Axis& axis1);
	void Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2);
	void Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3);
	void Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4);
	void Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4, const Axis& axis5);

	/*
	 * Coordinated helical motion. Spindle must be on. Feedrate must be non-zero.
	 * TODO Arcs.
	 */
	// void Arc(Direction dir)

//	void Plunge(double z, double helix);

	~Machine();
};

#endif /* MACHINE_H_ */
