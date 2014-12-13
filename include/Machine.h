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
#include <iosfwd>
#include <utility>
#include <stack>
#include <functional>
#include "GCodeWord.h"

namespace cxxcam
{

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
friend std::ostream& operator<<(std::ostream& os, const Machine& machine);
private:
	static const gcode::Word G00;
	static const gcode::Word G01;
	static const gcode::Word G02;
	static const gcode::Word G03;
	static const gcode::Word G04;
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
	static const gcode::Word G55;
	static const gcode::Word G56;
	static const gcode::Word G57;
	static const gcode::Word G58;
	static const gcode::Word G59;
	static const gcode::Word G59_1;
	static const gcode::Word G59_2;
	static const gcode::Word G59_3;

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
		block_RestoreCoordinateSystem = 1 << 2,
		block_RestoreMotion = 1 << 3,
		block_RestoreArcMotion = 1 << 4,
		block_RestoreFeedRateMode = 1 << 5,
		block_RestoreFeedRate = 1 << 6,
		block_RestoreSpindle = 1 << 7,
		block_RestoreTool = 1 << 8,
		block_RestorePosition = 1 << 9
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

	enum class CoordinateSystem
	{
		Active,
		P1,
		P2,
		P3,
		P4,
		P5,
		P6,
		P7,
		P8,
		P9
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
	std::stack<decltype(m_Private)> m_State;
protected:
	void Preamble();
	void UpdatePosition(const Axis& axis);
public:

	explicit Machine(Type type);
	Machine(Type type, Units units, const std::string& gcode_variant, std::function<void(const std::vector<gcode::Word>&, const std::string&)> gcode_callback);

	Machine(const Machine& m);
	Machine& operator=(const Machine& m);
	Machine(Machine&&) = default;
	Machine& operator=(Machine&&) = default;

	void dump() const;

	// push current state to stack
	void PushState();
	// Restore state from head of stack (restore previous state)
	void PopState();
	// pop head off stack & discard (maintain active state)
	void DiscardState();

	// Machine Setup

	bool AddTool(int id, const Tool& tool);
	bool RemoveTool(int id);

	void AddSpindleRange(unsigned long range_start, unsigned long range_end);
	void AddSpindleDiscrete(unsigned long discrete_value);
	void SetSpindleTorque(unsigned long rpm, double torque_nm);

	void SetStock(const Stock& stock);
	Stock GetStock() const;

	/*
	 * Set Global and per-axis feed rate limits.
	 * Units are based on current machine units.
	 * i.e.
	 * linear axes: mm/m or ipm
	 * rotary axes: degrees/second
	 * Note that Global limits apply only to linear axes.
	 */
	void SetGlobalMaxFeedrate(double limit);
	void SetMaxFeedrate(const Axis& axis, double limit);

	/*
	 * Set Global and per-axis rapid traverse rates
	 * Units are based on current machine units.
	 * i.e.
	 * linear axes: mm/m or ipm
	 * rotary axes: degrees/second
	 * Note that Global rate applies only to linear axes.
	 */
	void SetGlobalRapidRate(double rate);
	void SetRapidRate(const Axis& axis, double rate);

	/*
	 * Set available machine axes:
	 * e.g. "XYZ"
	 * or "XYZA"
	 */
	void SetMachineAxes(const std::string& axes);

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

	// Stateful properties

	// TODO possible functions to set coordinate system offsets (i.e. G10 L2)
	// http://www.linuxcnc.org/docs/html/gcode/images/offsets.png
	void SetCoordinateSystem(CoordinateSystem cs);
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

	CoordinateSystem GetCoordinateSystem() const;
	Motion GetMotion() const;
	Motion GetArcMotion() const;
	Units GetUnits() const;
	Plane GetPlane() const;
	std::pair<double, FeedRateMode> GetFeedRate() const;
	std::pair<unsigned long, Rotation> GetSpindleState() const;
	Tool GetTool() const;

	void NewBlock(const std::string& name);
	void EndBlock(int restore = block_PreserveState);

	void OptionalPause(const std::string& comment = {});
	void Comment(const std::string& comment);
	void Dwell(double seconds, const std::string& comment = {});

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
	 */
	void Arc(Direction dir, const std::vector<Axis>& end_pos, const std::vector<Offset>& center, unsigned int turns = 1);

//	void Plunge(double z, double helix);

	struct block_t
	{
		std::vector<gcode::Word> words;
		std::string comment;
		
		bool empty() const
		{
			return words.empty() && comment.empty();
		}
	};
	std::vector<block_t> Generate() const;

	~Machine();
};

std::ostream& operator<<(std::ostream& os, const Machine& machine);

std::string to_string(Machine::Units units);
std::string to_string(Machine::Plane plane);
std::string to_string(Machine::CoordinateSystem cs);
std::string to_string(Machine::Motion motion);
std::string to_string(Machine::FeedRateMode feed_rate_mode);
std::string to_string(Machine::Rotation rotation);

}

#endif /* MACHINE_H_ */
