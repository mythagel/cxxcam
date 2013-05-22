/* cxxcam - C++ CAD/CAM driver library.
 * Copyright (C) 2013  Nicholas Gill
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <vector>
#include "GCodeWord.h"

class Axis;
class Offset;
class Tool;
class Stock;

gcode::Word AxisToWord(const Axis& axis);
gcode::Word OffsetToWord(const Offset& offset);
double MillFeedRate(double chip_load, int flutes, double spindle_speed);
double MillSpindleSpeed(double cutting_speed, double cutter_diameter);

/*
 * The CNC machine itself.
 */
class Machine
{
private:
	static const gcode::Word G00;
	static const gcode::Word G01;
	static const gcode::Word G02;
	static const gcode::Word G03;
	static const gcode::Word G17;
	static const gcode::Word G18;
	static const gcode::Word G19;
	static const gcode::Word G17_1;
	static const gcode::Word G18_1;
	static const gcode::Word G19_1;
	static const gcode::Word G20;
	static const gcode::Word G21;
	static const gcode::Word G40;
	static const gcode::Word G49;
	static const gcode::Word G54;

	static const gcode::Word G61;
	static const gcode::Word G61_1;
	static const gcode::Word G64;

	static const gcode::Word G80;
	static const gcode::Word G90;
	static const gcode::Word G90_1;
	static const gcode::Word G91;
	static const gcode::Word G91_1;
	static const gcode::Word G93;
	static const gcode::Word G94;
	static const gcode::Word G95;
	static const gcode::Word G97;

	static const gcode::Word M01;
	static const gcode::Word M02;
	static const gcode::Word M03;
	static const gcode::Word M04;
	static const gcode::Word M05;
	static const gcode::Word M06;
	static const gcode::Word M09;
public:
	enum class Type
	{
		Mill,
		Lathe
	};

	enum BlockRestore
	{
		block_PreserveState = 0,
		block_RestoreState = ~0,
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

	enum class Units
	{
		Metric,
		Imperial
	};

	enum class Plane
	{
	    XY,
	    ZX,
	    YZ,
	    UV,
	    WU,
	    VW
	};

	enum class Motion
	{
		Absolute,
		Incremental
	};

	enum class FeedRateMode
	{
		InverseTime,
		UnitsPerMinute,
		UnitsPerRevolution
	};

	enum class Rotation
	{
		Stop,
		Clockwise,
		CounterClockwise
	};

	enum class Direction
	{
		Clockwise,
		CounterClockwise
	};
private:
	struct Private;
	std::unique_ptr<Private> m_Private;
protected:
	void Preamble();
	void UpdatePosition(const Axis& axis);
public:

	Machine(Type type, const std::string& gcode_variant);

	Machine(const Machine& m);
	Machine& operator=(const Machine& m);
	Machine(Machine&&) = default;
	Machine& operator=(Machine&&) = default;

	void dump() const;

	// Machine Setup

	bool AddTool(int id, const Tool& tool);
	bool RemoveTool(int id);

	void AddSpindleRange(unsigned long range_start, unsigned long range_end);
	void AddSpindleDiscrete(unsigned long discrete_value);

	void SetStock(const Stock& stock);

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

	void StartSpindle(unsigned long s, Rotation r = Rotation::Clockwise);
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
	void Rapid(const std::vector<Axis>& axes);

	/*
	 * Coordinated linear motion. Spindle must be on. Feedrate must be non-zero.
	 * TODO: calculate feed rate
	 */
	void Linear(const std::vector<Axis>& axes);

	/*
	 * Coordinated arc motion. Spindle must be on. Feedrate must be non-zero.
	 * TODO Arcs.
	 */
	void Arc(Direction dir, const std::vector<Axis>& end_pos, const std::vector<Offset>& center, unsigned int turns = 1);

//	void Plunge(double z, double helix);

	/*
	 * TODO better interface here...
	 */
	struct block_t
	{
		struct line_t
		{
			std::vector<gcode::Word> words;
			std::string comment;
		};
	
		std::string name;
		std::vector<line_t> lines;
	};
	std::vector<block_t> Generate() const;

	~Machine();
};

#endif /* MACHINE_H_ */
