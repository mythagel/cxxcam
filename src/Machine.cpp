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
 * Machine.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Machine.h"
#include "MachineState.h"
#include "GCode.h"
#include "Axis.h"
#include "Offset.h"
#include "Tool.h"
#include "ToolTable.h"
#include "Stock.h"
#include "Spindle.h"
#include "Limits.h"
#include "Path.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "Error.h"
#include "make_unique.h"

#include "Simulation.h"
#include "fold_adjacent.h"

#include "geom/translate.h"

namespace cxxcam
{

using gcode::Word;
using gcode::Line;
using gcode::Block;
using gcode::Code;

struct Machine::Private
{
	const Type m_Type;

	MachineState m_State;
	Code m_GCode;
	Spindle m_Spindle;
	ToolTable m_ToolTable;
	Stock m_Stock;
	
	limits::FeedRate m_FeedRateLimit;
	limits::Rapids m_RapidsRate;
	limits::AvailableAxes m_Axes;

	Private(Type type, const std::string& gcode_variant);
};

Machine::Private::Private(Type type, const std::string& gcode_variant)
 : 	m_Type(type), m_State(), m_GCode(gcode_variant), m_Spindle(), m_ToolTable(), m_Stock()
{
}

const Word Machine::G00(Word::G, 0);
const Word Machine::G01(Word::G, 1);
const Word Machine::G02(Word::G, 2);
const Word Machine::G03(Word::G, 3);
const Word Machine::G04(Word::G, 4);
const Word Machine::G17(Word::G, 17);
const Word Machine::G18(Word::G, 18);
const Word Machine::G19(Word::G, 19);
const Word Machine::G17_1(Word::G, 17.1);
const Word Machine::G18_1(Word::G, 18.1);
const Word Machine::G19_1(Word::G, 19.1);
const Word Machine::G20(Word::G, 20);
const Word Machine::G21(Word::G, 21);
const Word Machine::G40(Word::G, 40);
const Word Machine::G49(Word::G, 49);

const Word Machine::G54(Word::G, 54);
const Word Machine::G55(Word::G, 55);
const Word Machine::G56(Word::G, 56);
const Word Machine::G57(Word::G, 57);
const Word Machine::G58(Word::G, 58);
const Word Machine::G59(Word::G, 59);
const Word Machine::G59_1(Word::G, 59.1);
const Word Machine::G59_2(Word::G, 59.2);
const Word Machine::G59_3(Word::G, 59.3);

const Word Machine::G61(Word::G, 61);
const Word Machine::G61_1(Word::G, 61.1);
const Word Machine::G64(Word::G, 64);

const Word Machine::G80(Word::G, 80);
const Word Machine::G90(Word::G, 90);
const Word Machine::G90_1(Word::G, 90.1);
const Word Machine::G91(Word::G, 91);
const Word Machine::G91_1(Word::G, 91.1);
const Word Machine::G93(Word::G, 93);
const Word Machine::G94(Word::G, 94);
const Word Machine::G95(Word::G, 95);
const Word Machine::G97(Word::G, 97);

const Word Machine::M01(Word::M, 1);
const Word Machine::M02(Word::M, 2);
const Word Machine::M03(Word::M, 3);
const Word Machine::M04(Word::M, 4);
const Word Machine::M05(Word::M, 5);
const Word Machine::M06(Word::M, 6);
const Word Machine::M09(Word::M, 9);

Word AxisToWord(const Axis& axis)
{
	switch(axis)
	{
		case Axis::Type::X:
			return {Word::X, axis};
		case Axis::Type::Y:
			return {Word::Y, axis};
		case Axis::Type::Z:
			return {Word::Z, axis};

		case Axis::Type::A:
			return {Word::A, axis};
		case Axis::Type::B:
			return {Word::B, axis};
		case Axis::Type::C:
			return {Word::C, axis};

		case Axis::Type::U:
			return {Word::U, axis};
		case Axis::Type::V:
			return {Word::V, axis};
		case Axis::Type::W:
			return {Word::W, axis};
	}

	throw std::logic_error("Unknown Axis.");
}

gcode::Word OffsetToWord(const Offset& offset)
{
	switch(offset)
	{
		case Offset::Type::I:
			return {Word::I, offset};
		case Offset::Type::J:
			return {Word::J, offset};
		case Offset::Type::K:
			return {Word::K, offset};
	}

	throw std::logic_error("Unknown Offset.");
}

double MillFeedRate(double chip_load, int flutes, double spindle_speed)
{
	return chip_load * flutes * spindle_speed;
}

double MillSpindleSpeed(double cutting_speed, double cutter_diameter)
{
	return cutting_speed / (3.14159 * cutter_diameter);
}

void Machine::Preamble()
{
	const auto& m_Type = m_Private->m_Type;
	const auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	std::stringstream c;
	c << "cxxcam  ";

	switch(m_Type)
	{
		case Type::Mill:
			c << "Mill  ";
			break;
		case Type::Lathe:
			c << "Lathe  ";
			break;
	}

	Line line;

	switch(m_State.m_Plane)
	{
		case Plane::XY:
			c << "XY  ";
			line += G17;
			break;
		case Plane::ZX:
			c << "ZX  ";
			line += G18;
			break;
		case Plane::YZ:
			c << "YZ  ";
			line += G19;
			break;
		case Plane::UV:
			c << "UV  ";
			line += G17_1;
			break;
		case Plane::WU:
			c << "WU  ";
			line += G18_1;
			break;
		case Plane::VW:
			c << "VW  ";
			line += G19_1;
			break;
	}

	switch(m_State.m_Units)
	{
		case Units::Metric:
			c << "Metric  ";
			line += G21;
			break;
		case Units::Imperial:
			c << "Imperial  ";
			line += G20;
			break;
	}

	// Cancel Cutter Compensation
	line += G40;

	// Cancel Tool Length Compensation
	line += G49;

	// Select coordinate system
	switch(m_State.m_CoordinateSystem)
	{
		case CoordinateSystem::Active:
			throw error("Active coordinate system invalid in preamble.");
			break;
		case CoordinateSystem::P1:
			c << "CS 1  ";
			line += G54;
			break;
		case CoordinateSystem::P2:
			c << "CS 2  ";
			line += G55;
			break;
		case CoordinateSystem::P3:
			c << "CS 3  ";
			line += G56;
			break;
		case CoordinateSystem::P4:
			c << "CS 4  ";
			line += G57;
			break;
		case CoordinateSystem::P5:
			c << "CS 5  ";
			line += G58;
			break;
		case CoordinateSystem::P6:
			c << "CS 6  ";
			line += G59;
			break;
		case CoordinateSystem::P7:
			c << "CS 7  ";
			line += G59_1;
			break;
		case CoordinateSystem::P8:
			c << "CS 8  ";
			line += G59_2;
			break;
		case CoordinateSystem::P9:
			c << "CS 9  ";
			line += G59_3;
			break;
	}

	// Cancel canned cycles
	line += G80;

	switch(m_State.m_Motion)
	{
		case Motion::Absolute:
			c << "Absolute  ";
			line += G90;
			break;
		case Motion::Incremental:
			c << "Incremental  ";
			line += G91;
			break;
	}

	switch(m_State.m_ArcMotion)
	{
		case Motion::Absolute:
			c << "Absolute Arc  ";
			line += G90_1;
			break;
		case Motion::Incremental:
			c << "Incremental Arc  ";
			line += G91_1;
			break;
	}

	switch(m_State.m_FeedRateMode)
	{
		case FeedRateMode::InverseTime:
			c << "Inverse Time  ";
			line += G93;
			break;
		case FeedRateMode::UnitsPerMinute:
			c << "Units Per Minute  ";
			line += G94;
			break;
		case FeedRateMode::UnitsPerRevolution:
			c << "Units Per Revolution  ";
			line += G95;
			break;
	}

	// Spindle speed is in RPM
	line += G97;

	// Coolant off
	line += M09;

	// Spindle off
	line += M05;

	m_GCode.NewBlock(c.str(), m_State);
	m_GCode.AddLine(line);
	m_GCode.EndBlock();
}

void Machine::UpdatePosition(const Axis& axis)
{
	auto& m_State = m_Private->m_State;
	auto& m_Units = m_State.m_Units;

	units::length* linear = nullptr;
	units::plane_angle* rotary = nullptr;
	switch(axis)
	{
		case Axis::Type::X:
			linear = &m_State.m_Current.X;
			break;
		case Axis::Type::Y:
			linear = &m_State.m_Current.Y;
			break;
		case Axis::Type::Z:
			linear = &m_State.m_Current.Z;
			break;

		case Axis::Type::A:
			rotary = &m_State.m_Current.A;
			break;
		case Axis::Type::B:
			rotary = &m_State.m_Current.B;
			break;
		case Axis::Type::C:
			rotary = &m_State.m_Current.C;
			break;

		case Axis::Type::U:
			linear = &m_State.m_Current.U;
			break;
		case Axis::Type::V:
			linear = &m_State.m_Current.V;
			break;
		case Axis::Type::W:
			linear = &m_State.m_Current.W;
			break;
	}

	if(!linear && !rotary)
		throw std::logic_error("Unknown Axis");

	switch(m_State.m_Motion)
	{
		case Motion::Absolute:
		{
			switch(m_Units)
			{
				case Units::Metric:
				{
					if(linear)
						*linear = units::length{axis * units::millimeters};
					else
						*rotary = units::plane_angle{axis * units::degrees};
					break;
				}
				case Units::Imperial:
				{
					if(linear)
						*linear = units::length{axis * units::inches};
					else
						*rotary = units::plane_angle{axis * units::degrees};
					break;
				}
			}
			break;
		}
		case Motion::Incremental:
		{
			switch(m_Units)
			{
				case Units::Metric:
				{
					if(linear)
						*linear += units::length{axis * units::millimeters};
					else
						*rotary += units::plane_angle{axis * units::degrees};
					break;
				}
				case Units::Imperial:
				{
					if(linear)
						*linear += units::length{axis * units::inches};
					else
						*rotary += units::plane_angle{axis * units::degrees};
					break;
				}
			}
			break;
		}
	}
}

Machine::Machine(Type type)
 : m_Private(make_unique<Private>(type, "Generic"))
{
	const auto& m_Type = m_Private->m_Type;
	auto& m_State = m_Private->m_State;

	switch(m_Type)
	{
		case Type::Mill:
			m_State.m_Plane = Plane::XY;
			m_State.m_FeedRateMode = FeedRateMode::UnitsPerMinute;
			break;
		case Type::Lathe:
			m_State.m_Plane = Plane::ZX;
			m_State.m_FeedRateMode = FeedRateMode::UnitsPerRevolution;
			break;
	}
	
	Preamble();
}

Machine::Machine(Type type, Units units, const std::string& gcode_variant, std::function<void(const std::vector<gcode::Word>&, const std::string&)> gcode_callback)
 : m_Private(make_unique<Private>(type, gcode_variant))
{
	const auto& m_Type = m_Private->m_Type;
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	m_State.m_Units = units;

	switch(m_Type)
	{
		case Type::Mill:
			m_State.m_Plane = Plane::XY;
			m_State.m_FeedRateMode = FeedRateMode::UnitsPerMinute;
			break;
		case Type::Lathe:
			m_State.m_Plane = Plane::ZX;
			m_State.m_FeedRateMode = FeedRateMode::UnitsPerRevolution;
			break;
	}

    m_GCode.SetCallback(gcode_callback);
	
	Preamble();
}
Machine::Machine(const Machine& m)
 : m_Private(make_unique<Private>(*m.m_Private))
{
}
Machine& Machine::operator=(const Machine& m)
{
	m_Private = make_unique<Private>(*m.m_Private);
	return *this;
}

void Machine::PushState()
{
	m_State.push(make_unique<Private>(*m_Private));
}
void Machine::PopState()
{
	if(m_State.empty())
		throw error("No saved state to restore.");
	
	m_Private = std::move(m_State.top());
	m_State.pop();
}
void Machine::DiscardState()
{
	if(m_State.empty())
		throw error("No saved state to discard.");
	
	m_State.pop();
}

void Machine::dump() const
{
	auto& m_State = m_Private->m_State;

	std::cerr << "Units:            " << to_string(m_State.m_Units) << '\n';
	std::cerr << "Plane:            " << to_string(m_State.m_Plane) << '\n';
	std::cerr << "CoordinateSystem: " << to_string(m_State.m_CoordinateSystem) << '\n';
	std::cerr << "Motion:           " << to_string(m_State.m_Motion) << '\n';
	std::cerr << "ArcMotion:        " << to_string(m_State.m_ArcMotion) << '\n';
	std::cerr << "Spindle:          ";
	if(m_State.m_SpindleRotation != Machine::Rotation::Stop)
		std::cerr << m_State.m_SpindleSpeed << " RPM ";
	std::cerr << to_string(m_State.m_SpindleRotation) << '\n';
	
	if(m_State.m_FeedRate > 0.0)
	{
		std::cerr << "FeedRate:         ";
		switch(m_State.m_FeedRateMode)
		{
			case FeedRateMode::InverseTime:
			{
				auto minutes = 1/m_State.m_FeedRate;
				if(minutes > 1)
					std::cerr << "Feed Time: " << minutes << " minutes";
				else
					std::cerr << "Feed Time: " << minutes * 60 << " seconds";
				break;
			}
			case FeedRateMode::UnitsPerMinute:
			{
				switch(m_State.m_Units)
				{
					case Units::Metric:
						std::cerr << m_State.m_FeedRate << "mm per minute";
						break;
					case Units::Imperial:
						std::cerr << m_State.m_FeedRate << "\" per minute";
						break;
				}
				break;
			}
			case FeedRateMode::UnitsPerRevolution:
			{
				switch(m_State.m_Units)
				{
					case Units::Metric:
						std::cerr << m_State.m_FeedRate << "mm per revolution";
						break;
					case Units::Imperial:
						std::cerr << m_State.m_FeedRate << "\" per revolution";
						break;
				}
				break;
			}
		}
		std::cerr << '\n';
	}
	else
	{
		std::cerr << "FeedRate:         Zero\n";
	}

	if(m_State.m_CurrentTool)
	{
		Tool tool;
		m_Private->m_ToolTable.Get(m_State.m_CurrentTool, &tool);
		std::cerr << "Tool:             " << tool.Name() << '\n';
	}
	else
	{
		std::cerr << "Tool:             None\n";
	}

	std::cerr << "Position:         \n" << m_State.m_Current.str() << '\n';
}

bool Machine::AddTool(int id, const Tool& tool)
{
	if(id == 0)
		throw error("Tool ID must be > 0");

	const auto& m_Type = m_Private->m_Type;
	switch(m_Type)
	{
		case Type::Mill:
			if(tool.ToolType() != Tool::Type::Mill)
				throw error("Must use Mill tool with Mill.");
			break;
		case Type::Lathe:
			if(tool.ToolType() != Tool::Type::Lathe)
				throw error("Must use Lathe tool with Lathe.");
			break;
	}
	return m_Private->m_ToolTable.AddTool(id, tool);
}
bool Machine::RemoveTool(int id)
{
	if(id == 0)
		throw error("Tool ID must be > 0");

	return m_Private->m_ToolTable.RemoveTool(id);
}
void Machine::AddSpindleRange(unsigned long range_start, unsigned long range_end)
{
	m_Private->m_Spindle.AddRange(range_start, range_end);
}
void Machine::AddSpindleDiscrete(unsigned long discrete_value)
{
	m_Private->m_Spindle.AddDiscrete(discrete_value);
}
void Machine::SetSpindleTorque(unsigned long rpm, double torque_nm)
{
	m_Private->m_Spindle.SetTorque(rpm, units::torque{torque_nm * units::newton_meters});
}
void Machine::SetStock(const Stock& stock)
{
	static const auto inches_to_mm = 25.4;
	auto& m_State = m_Private->m_State;
	
	switch(m_State.m_Units)
	{
		case Units::Metric:
			m_Private->m_Stock = stock;
			break;
		case Units::Imperial:
			m_Private->m_Stock = stock;
			m_Private->m_Stock.Model = scale(stock.Model, inches_to_mm);
			break;
	}
}
Stock Machine::GetStock() const
{
	static const auto mm_to_inches = 0.0393700787;
	auto& m_State = m_Private->m_State;
	
	switch(m_State.m_Units)
	{
		case Units::Metric:
			return m_Private->m_Stock;
		case Units::Imperial:
		{
			Stock stock = m_Private->m_Stock;
			stock.Model = scale(stock.Model, mm_to_inches);
			return stock;
		}
	}
	return m_Private->m_Stock;
}

void Machine::SetGlobalMaxFeedrate(double limit)
{
	auto& m_State = m_Private->m_State;
	auto& m_FeedRateLimit = m_Private->m_FeedRateLimit;
	
	switch(m_State.m_Units)
	{
		case Units::Metric:
			m_FeedRateLimit.SetGlobal(units::velocity{limit * units::millimeters_per_minute});
			break;
		case Units::Imperial:
			m_FeedRateLimit.SetGlobal(units::velocity{limit * units::inches_per_minute});
			break;
	}
}
void Machine::SetMaxFeedrate(const Axis& axis, double limit)
{
	auto& m_State = m_Private->m_State;
	auto& m_FeedRateLimit = m_Private->m_FeedRateLimit;
	auto& m_Axes = m_Private->m_Axes;

	m_Axes.Validate(axis);

	if(is_linear(axis))
	{
		switch(m_State.m_Units)
		{
			case Units::Metric:
				m_FeedRateLimit.Set(axis, units::velocity{limit * units::millimeters_per_minute});
				break;
			case Units::Imperial:
				m_FeedRateLimit.Set(axis, units::velocity{limit * units::inches_per_minute});
				break;
		}
	}
	else
	{
		m_FeedRateLimit.Set(axis, units::angular_velocity{limit * units::degrees_per_second});
	}
}

void Machine::SetGlobalRapidRate(double rate)
{
	auto& m_State = m_Private->m_State;
	auto& m_RapidsRate = m_Private->m_RapidsRate;
	
	switch(m_State.m_Units)
	{
		case Units::Metric:
			m_RapidsRate.SetGlobal(units::velocity{rate * units::millimeters_per_minute});
			break;
		case Units::Imperial:
			m_RapidsRate.SetGlobal(units::velocity{rate * units::inches_per_minute});
			break;
	}
}
void Machine::SetRapidRate(const Axis& axis, double rate)
{
	auto& m_State = m_Private->m_State;
	auto& m_RapidsRate = m_Private->m_RapidsRate;
	auto& m_Axes = m_Private->m_Axes;

	m_Axes.Validate(axis);

	if(is_linear(axis))
	{
		switch(m_State.m_Units)
		{
			case Units::Metric:
				m_RapidsRate.Set(axis, units::velocity{rate * units::millimeters_per_minute});
				break;
			case Units::Imperial:
				m_RapidsRate.Set(axis, units::velocity{rate * units::inches_per_minute});
				break;
		}
	}
	else
	{
		m_RapidsRate.Set(axis, units::angular_velocity{rate * units::degrees_per_second});
	}
}

void Machine::SetMachineAxes(const std::string& axes)
{
	auto& m_Axes = m_Private->m_Axes;

	std::vector<Axis::Type> available;
	for(auto axis : axes)
		available.push_back(to_axis(axis));
	
	m_Axes = limits::AvailableAxes(available);
}

void Machine::SetTool(int id)
{
	auto& m_GCode = m_Private->m_GCode;
	auto& m_ToolTable = m_Private->m_ToolTable;

	Tool tool;
	if(id && m_ToolTable.Get(id, &tool))
	{
		m_GCode.AddLine(Line(Word(Word::T, id), "Preload tool " + tool.Name()));
	}
	else if(id == 0)
	{
		m_GCode.AddLine(Line(Word(Word::T, id), "Preload empty tool"));
	}
	else
	{
		// Unknown tool.
		std::stringstream s;
		s << "Preload Unknown tool id: " << id;
		throw error(s.str());
	}
}
void Machine::ToolChange(int id)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;
	auto& m_ToolTable = m_Private->m_ToolTable;

	if(m_State.m_CurrentTool != id)
	{
		Tool tool;
		if(id && m_ToolTable.Get(id, &tool))
		{
			m_State.m_CurrentTool = id;

			Line line;
			line.Comment("Switch to tool " + tool.Name());
			line += Word(Word::T, id);
			line += M06;
			m_GCode.AddLine(line);
		}
		else if(id == 0)
		{
			m_State.m_CurrentTool = id;

			Line line;
			line.Comment("Empty Spindle");
			line += Word(Word::T, 0);
			line += M06;
			m_GCode.AddLine(line);
		}
		else
		{
			// Unknown tool.
			std::stringstream s;
			s << "Unknown tool id: " << id;
			throw error(s.str());
		}
	}
}

auto Machine::GetCoordinateSystem() const -> CoordinateSystem
{
	auto& m_State = m_Private->m_State;
	return m_State.m_CoordinateSystem;
}
auto Machine::GetMotion() const -> Motion
{
	auto& m_State = m_Private->m_State;
	return m_State.m_Motion;
}
auto Machine::GetArcMotion() const -> Motion
{
	auto& m_State = m_Private->m_State;
	return m_State.m_ArcMotion;
}
auto Machine::GetUnits() const -> Units
{
	auto& m_State = m_Private->m_State;
	return m_State.m_Units;
}
auto Machine::GetPlane() const -> Plane
{
	auto& m_State = m_Private->m_State;
	return m_State.m_Plane;
}
auto Machine::GetFeedRate() const -> std::pair<double, FeedRateMode>
{
	auto& m_State = m_Private->m_State;
	return {m_State.m_FeedRate, m_State.m_FeedRateMode};
}
auto Machine::GetSpindleState() const -> std::pair<unsigned long, Rotation>
{
	auto& m_State = m_Private->m_State;
	return {m_State.m_SpindleSpeed, m_State.m_SpindleRotation};
}
Tool Machine::GetTool() const
{
	auto& m_State = m_Private->m_State;
	Tool tool;
	if(m_Private->m_ToolTable.Get(m_State.m_CurrentTool, &tool))
		return tool;
	
	throw error("Unknown tool");
}

void Machine::NewBlock(const std::string& name)
{
	m_Private->m_GCode.NewBlock(name, m_Private->m_State);
}
void Machine::EndBlock(int restore)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(restore)
	{
		m_GCode.AddLine(Line("Restore State"));

		const auto& block = m_GCode.CurrentBlock();
		const auto& saved_state = block.State();

		if(restore & block_RestoreUnits)
			SetUnits(saved_state.m_Units);
		if(restore & block_RestorePlane)
			SetPlane(saved_state.m_Plane);
		if(restore & block_RestoreCoordinateSystem)
			SetCoordinateSystem(saved_state.m_CoordinateSystem);
		if(restore & block_RestoreMotion)
			SetMotion(saved_state.m_Motion);
		if(restore & block_RestoreArcMotion)
			SetArcMotion(saved_state.m_ArcMotion);
		if(restore & block_RestoreFeedRateMode)
			SetFeedRateMode(saved_state.m_FeedRateMode);
		if(restore & block_RestoreFeedRate)
			SetFeedRate(saved_state.m_FeedRate);
		if(restore & block_RestoreSpindle)
			StartSpindle(saved_state.m_SpindleSpeed, saved_state.m_SpindleRotation);
		if(restore & block_RestoreTool)
			ToolChange(saved_state.m_CurrentTool);
		if(restore & block_RestorePosition)
		{
			if(m_State.m_Current != saved_state.m_Current)
			{
				// TODO determine how to safely rapid to the previous position.
			}
		}
	}
	m_GCode.EndBlock();
}

void Machine::AccuracyExactPath()
{
	m_Private->m_GCode.AddLine(Line(G61, "Exact Path"));
}
void Machine::AccuracyExactStop()
{
	m_Private->m_GCode.AddLine(Line(G61_1, "Exact Stop"));
}
void Machine::AccuracyPathBlending()
{
	m_Private->m_GCode.AddLine(Line(G64, "Path Blend Without Tolerance"));
}
void Machine::AccuracyPathBlending(double p)
{
	Line line(G64, "Path Blend With Tolerance");
	line += Word(Word::P, p);
	m_Private->m_GCode.AddLine(line);
}
void Machine::AccuracyPathBlending(double p, double q)
{
	Line line(G64, "Path Blend With Tolerance & Folding");
	line += Word(Word::P, p);
	line += Word(Word::Q, q);
	m_Private->m_GCode.AddLine(line);
}

void Machine::OptionalPause(const std::string& comment)
{
	m_Private->m_GCode.AddLine(Line(M01, comment));
}

void Machine::Comment(const std::string& comment)
{
	m_Private->m_GCode.AddLine(Line(comment));
}

void Machine::Dwell(double seconds, const std::string& comment)
{
	Line line(G04, comment);
	line += Word(Word::P, seconds);
	m_Private->m_GCode.AddLine(line);
}

void Machine::SetCoordinateSystem(CoordinateSystem cs)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_CoordinateSystem != cs)
	{
		m_State.m_CoordinateSystem = cs;

		switch(m_State.m_CoordinateSystem)
		{
			case CoordinateSystem::Active:
				throw error("Cannot change to Active coordinate system");
				break;
			case CoordinateSystem::P1:
				m_GCode.AddLine(Line(G54, "Switch to CS 1"));
				break;
			case CoordinateSystem::P2:
				m_GCode.AddLine(Line(G55, "Switch to CS 2"));
				break;
			case CoordinateSystem::P3:
				m_GCode.AddLine(Line(G56, "Switch to CS 3"));
				break;
			case CoordinateSystem::P4:
				m_GCode.AddLine(Line(G57, "Switch to CS 4"));
				break;
			case CoordinateSystem::P5:
				m_GCode.AddLine(Line(G58, "Switch to CS 5"));
				break;
			case CoordinateSystem::P6:
				m_GCode.AddLine(Line(G59, "Switch to CS 6"));
				break;
			case CoordinateSystem::P7:
				m_GCode.AddLine(Line(G59_1, "Switch to CS 7"));
				break;
			case CoordinateSystem::P8:
				m_GCode.AddLine(Line(G59_2, "Switch to CS 8"));
				break;
			case CoordinateSystem::P9:
				m_GCode.AddLine(Line(G59_3, "Switch to CS 9"));
				break;
		}
	}
}

void Machine::SetMotion(Motion m)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_Motion != m)
	{
		m_State.m_Motion = m;

		switch(m_State.m_Motion)
		{
			case Motion::Absolute:
				m_GCode.AddLine(Line(G90, "Switch to Absolute Motion"));
				break;
			case Motion::Incremental:
				m_GCode.AddLine(Line(G91, "Switch to Incremental Motion"));
				break;
		}
	}
}
void Machine::SetArcMotion(Motion m)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_ArcMotion != m)
	{
		m_State.m_ArcMotion = m;

		switch(m_State.m_ArcMotion)
		{
			case Motion::Absolute:
				m_GCode.AddLine(Line(G90_1, "Switch to Absolute Arc Motion"));
				break;
			case Motion::Incremental:
				m_GCode.AddLine(Line(G91_1, "Switch to Incremental Arc Motion"));
				break;
		}
	}
}
void Machine::SetUnits(Units u)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_Units != u)
	{
		m_State.m_Units = u;

		switch(m_State.m_Units)
		{
			case Units::Metric:
				m_GCode.AddLine(Line(G21, "Switch to Metric (Millimeters)"));
				break;
			case Units::Imperial:
				m_GCode.AddLine(Line(G20, "Switch to Imperial (Inches)"));
				break;
		}

		if(m_State.m_FeedRate > 0.0)
		{
			std::stringstream c;
			c << "Active feed rate meaning changed to ";

			switch(m_State.m_FeedRateMode)
			{
				case FeedRateMode::InverseTime:
				{
					auto minutes = 1/m_State.m_FeedRate;
					if(minutes > 1)
						c << "Feed Time: " << minutes << " minutes";
					else
						c << "Feed Time: " << minutes * 60 << " seconds";
					break;
				}
				case FeedRateMode::UnitsPerMinute:
				{
					switch(m_State.m_Units)
					{
						case Units::Metric:
							c << m_State.m_FeedRate << "mm per minute";
							break;
						case Units::Imperial:
							c << m_State.m_FeedRate << "\" per minute";
							break;
					}
					break;
				}
				case FeedRateMode::UnitsPerRevolution:
				{
					switch(m_State.m_Units)
					{
						case Units::Metric:
							c << m_State.m_FeedRate << "mm per revolution";
							break;
						case Units::Imperial:
							c << m_State.m_FeedRate << "\" per revolution";
							break;
					}
					break;
				}
			}
			m_GCode.AddLine(Line(c.str()));
		}
	}
}
void Machine::SetPlane(Plane p)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_Plane != p)
	{
		m_State.m_Plane = p;

		switch(m_State.m_Plane)
		{
			case Plane::XY:
				m_GCode.AddLine(Line(G17, "Switch to XY Plane"));
				break;
			case Plane::ZX:
				m_GCode.AddLine(Line(G18, "Switch to ZX Plane"));
				break;
			case Plane::YZ:
				m_GCode.AddLine(Line(G19, "Switch to YZ Plane"));
				break;
			case Plane::UV:
				m_GCode.AddLine(Line(G17_1, "Switch to UV Plane"));
				break;
			case Plane::WU:
				m_GCode.AddLine(Line(G18_1, "Switch to WU Plane"));
				break;
			case Plane::VW:
				m_GCode.AddLine(Line(G19_1, "Switch to VW Plane"));
				break;
		}
	}
}
void Machine::SetFeedRateMode(FeedRateMode f)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_FeedRateMode != f)
	{
		m_State.m_FeedRateMode = f;

		switch(m_State.m_FeedRateMode)
		{
			case FeedRateMode::InverseTime:
				m_GCode.AddLine(Line(G93, "Switch to Inverse Time Feed Rate Mode"));
				break;
			case FeedRateMode::UnitsPerMinute:
				m_GCode.AddLine(Line(G94, "Switch to Units Per Minute Feed Rate Mode"));
				break;
			case FeedRateMode::UnitsPerRevolution:
				m_GCode.AddLine(Line(G95, "Switch to Units Per Revolution Feed Rate Mode"));
				break;
		}

		if(m_State.m_FeedRate > 0.0)
		{
			std::stringstream c;
			c << "Active feed rate meaning changed to ";

			switch(m_State.m_FeedRateMode)
			{
				case FeedRateMode::InverseTime:
				{
					auto minutes = 1/m_State.m_FeedRate;
					if(minutes > 1)
						c << "Feed Time: " << minutes << " minutes";
					else
						c << "Feed Time: " << minutes * 60 << " seconds";
					break;
				}
				case FeedRateMode::UnitsPerMinute:
				{
					switch(m_State.m_Units)
					{
						case Units::Metric:
							c << m_State.m_FeedRate << "mm per minute";
							break;
						case Units::Imperial:
							c << m_State.m_FeedRate << "\" per minute";
							break;
					}
					break;
				}
				case FeedRateMode::UnitsPerRevolution:
				{
					switch(m_State.m_Units)
					{
						case Units::Metric:
							c << m_State.m_FeedRate << "mm per revolution";
							break;
						case Units::Imperial:
							c << m_State.m_FeedRate << "\" per revolution";
							break;
					}
					break;
				}
			}
			m_GCode.AddLine(Line(c.str()));
		}
	}
}
void Machine::SetFeedRate(double f)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_FeedRate != f)
	{
		m_State.m_FeedRate = f;

		std::stringstream c;
		if(f > 0.0)
		{
			switch(m_State.m_FeedRateMode)
			{
				case FeedRateMode::InverseTime:
					break;
				case FeedRateMode::UnitsPerMinute:
					switch(m_State.m_Units)
					{
						case Units::Metric:
							c << f << "mm per minute";
							break;
						case Units::Imperial:
							c << f << "\" per minute";
							break;
					}
					break;
				case FeedRateMode::UnitsPerRevolution:
					switch(m_State.m_Units)
					{
						case Units::Metric:
							c << f << "mm per revolution";
							break;
						case Units::Imperial:
							c << f << "\" per revolution";
							break;
					}
					break;
			}
		}

		if(m_State.m_FeedRateMode != FeedRateMode::InverseTime)
		{
			Line line(c.str());
			line += Word(Word::F, f);
			m_GCode.AddLine(line);
		}
	}
}
void Machine::StartSpindle(unsigned long s, Rotation r)
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;
	auto& m_Spindle = m_Private->m_Spindle;

	unsigned long requested_speed(s);
	s = m_Spindle.Normalise(s);

	if(m_State.m_SpindleSpeed != s || m_State.m_SpindleRotation != r)
	{
		m_State.m_SpindleSpeed = s;
		m_State.m_SpindleRotation = r;

		Line line;
		std::stringstream c;
		switch(m_State.m_SpindleRotation)
		{
			case Rotation::Stop:
				m_State.m_SpindleSpeed = 0;
				line += M05;
				c << "Stop Spindle";
				break;
			case Rotation::Clockwise:
				line += M03;
				line += Word(Word::S, s);
				c << "Start Spindle Clockwise " << s << " RPM";
				if(m_State.m_SpindleSpeed != requested_speed)
					c << " (" << requested_speed << " RPM Requested)";
				break;
			case Rotation::CounterClockwise:
				line += M04;
				line += Word(Word::S, s);
				c << "Start Spindle Counter Clockwise " << s << " RPM";
				if(m_State.m_SpindleSpeed != requested_speed)
					c << " (" << requested_speed << " RPM Requested)";
				break;
		}
		line.Comment(c.str());
		m_GCode.AddLine(line);
	}
}
void Machine::StopSpindle()
{
	auto& m_State = m_Private->m_State;
	auto& m_GCode = m_Private->m_GCode;

	if(m_State.m_SpindleRotation != Rotation::Stop || m_State.m_SpindleSpeed > 0)
	{
		m_State.m_SpindleSpeed = 0;
		m_State.m_SpindleRotation = Rotation::Stop;

		m_GCode.AddLine(Line(M05, "Stop Spindle"));
	}
}

void Machine::Rapid(const std::vector<Axis>& axes)
{
	auto start = m_Private->m_State.m_Current;
	
	auto& m_Axes = m_Private->m_Axes;

	Line line;
	line += G00;
	for(auto& axis : axes)
	{
		m_Axes.Validate(axis);
		line += AxisToWord(axis);
		UpdatePosition(axis);
	}
	m_Private->m_GCode.AddLine(line);
	
	auto end = m_Private->m_State.m_Current;
	// calculate possible motion path (not a line but a polyhedron.)
	auto path = path::expand_linear(start, end, m_Axes);
}

void Machine::Linear(const std::vector<Axis>& axes)
{
	auto linear_start = m_Private->m_State.m_Current;

	auto& m_State = m_Private->m_State;
	auto& m_Axes = m_Private->m_Axes;
	auto& m_Stock = m_Private->m_Stock;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw error("Feedrate is 0.0");
	if(m_State.m_CurrentTool == 0)
		throw error("No tool loaded.");

	Line line;

	line += G01;
	for(auto& axis : axes)
	{
		m_Axes.Validate(axis);
		line += AxisToWord(axis);
		UpdatePosition(axis);
	}

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		auto minutes = 1/m_State.m_FeedRate;
		if(minutes > 1)
			c << "Feed Time: " << minutes << " minutes";
		else
			c << "Feed Time: " << minutes * 60 << " seconds";
		line += Word(Word::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
	
	auto linear_end = m_Private->m_State.m_Current;
	// line from start to end expand tool along path and subtract tool path from stock.
	auto path = path::expand_linear(linear_start, linear_end, m_Axes, 1);
	
	simulation::simulation_t sim;
	sim.steps = path;
	sim.stock = m_Stock;
	sim.tool = GetTool();
	
	auto result = run(sim);
	
	// Update local state
	m_Stock = result.stock;
}

void Machine::Arc(Direction dir, const std::vector<Axis>& end_pos, const std::vector<Offset>& center, unsigned int turns)
{
	auto angular_start = m_Private->m_State.m_Current;
	
	auto& m_State = m_Private->m_State;
	auto& m_Axes = m_Private->m_Axes;
	auto& m_Stock = m_Private->m_Stock;
	auto& m_Units = m_State.m_Units;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw error("Feedrate is 0.0");
	if(m_State.m_CurrentTool == 0)
		throw error("No tool loaded.");
	if(end_pos.empty() && center.empty())
		throw error("Must specify end position or center.");

    // TODO check for zero radius arc.
	
	Line line;
	switch(dir)
	{
		case Direction::Clockwise:
			line += G02;
			break;
		case Direction::CounterClockwise:
			line += G03;
			break;
	}
	
	/*
	 * TODO cleanup code (remove duplication)
	 * TODO Implement additional logic checks (atm will accept empty arc command which is invalid
	 */
	switch(m_State.m_Plane)
	{
		case Plane::XY:
		{
			for(auto& axis : end_pos)
			{
				m_Axes.Validate(axis);
				switch(axis)
				{
					case Axis::Type::X:
					case Axis::Type::Y:
					{
						line += AxisToWord(axis);
						UpdatePosition(axis);
						break;
					}
					case Axis::Type::Z:
					{
						auto word = AxisToWord(axis);
						word.Comment("Helix");
						line += word;
						UpdatePosition(axis);
						break;
					}
					default:
					{
						line += AxisToWord(axis);
						UpdatePosition(axis);
						break;
					}
				}
			}

			for(auto& offset : center)
			{
				switch(offset)
				{
					case Offset::Type::I:
					case Offset::Type::J:
					{
						line += OffsetToWord(offset);
						break;
					}
					default:
						throw error("Allowed offsets: I & J");
				}
			}
			break;
		}
		case Plane::ZX:
		{
			for(auto& axis : end_pos)
			{
				m_Axes.Validate(axis);
				switch(axis)
				{
					case Axis::Type::X:
					case Axis::Type::Z:
					{
						line += AxisToWord(axis);
						UpdatePosition(axis);
						break;
					}
					case Axis::Type::Y:
					{
						auto word = AxisToWord(axis);
						word.Comment("Helix");
						line += word;
						UpdatePosition(axis);
						break;
					}
					default:
					{
						line += AxisToWord(axis);
						UpdatePosition(axis);
						break;
					}
				}
			}

			for(auto& offset : center)
			{
				switch(offset)
				{
					case Offset::Type::I:
					case Offset::Type::K:
					{
						line += OffsetToWord(offset);
						break;
					}
					default:
						throw error("Allowed offsets: I & K");
				}
			}
			break;
		}
		case Plane::YZ:
		{
			for(auto& axis : end_pos)
			{
				m_Axes.Validate(axis);
				switch(axis)
				{
					case Axis::Type::Y:
					case Axis::Type::Z:
					{
						line += AxisToWord(axis);
						UpdatePosition(axis);
						break;
					}
					case Axis::Type::X:
					{
						auto word = AxisToWord(axis);
						word.Comment("Helix");
						line += word;
						UpdatePosition(axis);
						break;
					}
					default:
					{
						line += AxisToWord(axis);
						UpdatePosition(axis);
						break;
					}
				}
			}

			for(auto& offset : center)
			{
				switch(offset)
				{
					case Offset::Type::J:
					case Offset::Type::K:
					{
						line += OffsetToWord(offset);
						break;
					}
					default:
						throw error("Allowed offsets: J & K");
				}
			}
			break;
		}
		case Plane::UV:
		case Plane::WU:
		case Plane::VW:
			throw error("Arc defined only on Planes XY, ZX, & YZ");
			break;
	}
	
	if(turns > 1)
		line += Word(Word::P, turns);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		auto minutes = 1/m_State.m_FeedRate;
		if(minutes > 1)
			c << "Feed Time: " << minutes << " minutes";
		else
			c << "Feed Time: " << minutes * 60 << " seconds";
		line += Word(Word::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
	
	auto angular_end = m_Private->m_State.m_Current;
	// arc from start to end expand tool along path.
	
	auto incremental2absolute = [&m_State, &angular_start, &m_Units](const Offset& offset) -> units::length
	{
		auto offset2length = [&m_Units](const Offset& offset) -> units::length
		{
			switch(m_Units)
			{
				case Units::Metric:
					return units::length{offset * units::millimeters};
				case Units::Imperial:
					return units::length{offset * units::inches};
				default:
					throw std::logic_error("Unrecognised units.");
			}
		};
		
		switch(m_State.m_ArcMotion)
		{
			case Motion::Absolute:
				return offset2length(offset);
			
			case Motion::Incremental:
			{
				switch(offset)
				{
					case Offset::Type::I:
						return angular_start.X + offset2length(offset);
					case Offset::Type::J:
						return angular_start.Y + offset2length(offset);
					case Offset::Type::K:
						return angular_start.Z + offset2length(offset);
				}
				break;
			}
		}
		throw std::logic_error("incremental2absolute");
	};
	auto center2arc_center = [incremental2absolute](const std::vector<Offset>& center) -> Position_Cartesian
	{
		Position_Cartesian arc_center;
		for(auto& offset : center)
		{
			switch(offset)
			{
				case Offset::Type::I:
					arc_center.X = incremental2absolute(offset);
					break;
				case Offset::Type::J:
					arc_center.Y = incremental2absolute(offset);
					break;
				case Offset::Type::K:
					arc_center.Z = incremental2absolute(offset);
			}
		}
		return arc_center;
	};
	auto dir2arcdir = [](Direction dir) -> path::ArcDirection
	{
		switch(dir)
		{
			case Direction::Clockwise:
				return path::ArcDirection::Clockwise;
			case Direction::CounterClockwise:
				return path::ArcDirection::CounterClockwise;
			default:
				throw std::logic_error("Unknown Arc Direction");
		}
	};
	auto plane2vector_3 = [](Plane plane) -> math::vector_3
	{
		switch(plane)
		{
			case Plane::XY:
				return math::vector_3{0, 0, 1};
			case Plane::ZX:
				return math::vector_3{0, 1, 0};
			case Plane::YZ:
				return math::vector_3{1, 0, 0};
			default:
				throw std::logic_error("Unsupported Arc Plane");
		}
	};
	
	auto path = path::expand_arc(angular_start, angular_end, center2arc_center(center), dir2arcdir(dir), plane2vector_3(m_State.m_Plane), turns, m_Axes, 1);
	
	simulation::simulation_t sim;
	sim.steps = path;
	sim.stock = m_Stock;
	sim.tool = GetTool();
	
	auto result = run(sim);
	
	// Update local state
	m_Stock = result.stock;
}

auto Machine::Generate() const -> std::vector<block_t>
{
	auto& m_GCode = m_Private->m_GCode;
	
	std::vector<block_t> lines;
	for(auto& block : m_GCode)
	{
		lines.push_back({{}, block.Name()});

		for(auto& line : block)
			lines.push_back({ {line.begin(), line.end()}, line.Comment() });
		
		lines.emplace_back();
	}
	
	lines.push_back({ {M02}, "End of program."});
	
	return lines;
}

Machine::~Machine()
{
}

std::ostream& operator<<(std::ostream& os, const Machine& machine)
{
	auto gcode = machine.m_Private->m_GCode;
	gcode.AddLine(Line(Machine::M02, "End of program."));
	os << gcode;
	return os;
}

std::string to_string(Machine::Units units)
{
	switch(units)
	{
		case Machine::Units::Metric:
			return "Metric";
		case Machine::Units::Imperial:
			return "Imperial";
	}
	throw std::logic_error("Unknown units");
}
std::string to_string(Machine::Plane plane)
{
	switch(plane)
	{
		case Machine::Plane::XY:
			return "XY";
		case Machine::Plane::ZX:
			return "ZX";
		case Machine::Plane::YZ:
			return "YZ";
		case Machine::Plane::UV:
			return "UV";
		case Machine::Plane::WU:
			return "WU";
		case Machine::Plane::VW:
			return "VW";
	}
	throw std::logic_error("Unknown plane");
}
std::string to_string(Machine::CoordinateSystem cs)
{
	switch(cs)
	{
		case Machine::CoordinateSystem::Active:
			return "Active";
		case Machine::CoordinateSystem::P1:
			return "CS 1";
		case Machine::CoordinateSystem::P2:
			return "CS 2";
		case Machine::CoordinateSystem::P3:
			return "CS 3";
		case Machine::CoordinateSystem::P4:
			return "CS 4";
		case Machine::CoordinateSystem::P5:
			return "CS 5";
		case Machine::CoordinateSystem::P6:
			return "CS 6";
		case Machine::CoordinateSystem::P7:
			return "CS 7";
		case Machine::CoordinateSystem::P8:
			return "CS 8";
		case Machine::CoordinateSystem::P9:
			return "CS 9";
	}
	throw std::logic_error("Unknown coordinate system");
}
std::string to_string(Machine::Motion motion)
{
	switch(motion)
	{
		case Machine::Motion::Absolute:
			return "Absolute";
		case Machine::Motion::Incremental:
			return "Incremental";
	}
	throw std::logic_error("Unknown motion");
}
std::string to_string(Machine::FeedRateMode feed_rate_mode)
{
	switch(feed_rate_mode)
	{
		case Machine::FeedRateMode::InverseTime:
			return "Inverse Time";
		case Machine::FeedRateMode::UnitsPerMinute:
			return "Units Per Minute";
		case Machine::FeedRateMode::UnitsPerRevolution:
			return "Units Per Revolution";
	}
	throw std::logic_error("Unknown feed rate mode");
}
std::string to_string(Machine::Rotation rotation)
{
	switch(rotation)
	{
		case Machine::Rotation::Stop:
			return "Stop";
		case Machine::Rotation::Clockwise:
			return "Clockwise";
		case Machine::Rotation::CounterClockwise:
			return "Counter-Clockwise";
	}
	throw std::logic_error("Unknown rotation");
}

}

