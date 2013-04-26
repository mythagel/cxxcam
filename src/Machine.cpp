/*
 * Machine.cpp
 *
 *  Created on: 26/04/2012
 *      Author: nicholas
 */

#include "Machine.h"
#include "MachineState.h"
#include "GCode.h"
#include "ToolTable.h"
#include "Stock.h"
#include "Spindle.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}

struct Machine::Private
{
	const Type m_Type;

	MachineState m_State;
	GCode m_GCode;
	Spindle m_Spindle;
	ToolTable m_ToolTable;
	Stock m_Stock;

	Private(Type type, const std::string& gcode_variant);
};

Machine::Private::Private(Type type, const std::string& gcode_variant)
 : 	m_Type(type), m_State(), m_GCode(gcode_variant), m_Spindle(), m_ToolTable(), m_Stock()
{
}

const GCodeWord Machine::G00(GCodeWord::G, 0);
const GCodeWord Machine::G01(GCodeWord::G, 1);
const GCodeWord Machine::G17(GCodeWord::G, 17);
const GCodeWord Machine::G18(GCodeWord::G, 18);
const GCodeWord Machine::G19(GCodeWord::G, 19);
const GCodeWord Machine::G17_1(GCodeWord::G, 17.1);
const GCodeWord Machine::G18_1(GCodeWord::G, 18.1);
const GCodeWord Machine::G19_1(GCodeWord::G, 19.1);
const GCodeWord Machine::G20(GCodeWord::G, 20);
const GCodeWord Machine::G21(GCodeWord::G, 21);
const GCodeWord Machine::G40(GCodeWord::G, 40);
const GCodeWord Machine::G49(GCodeWord::G, 49);
const GCodeWord Machine::G54(GCodeWord::G, 54);

const GCodeWord Machine::G61(GCodeWord::G, 61);
const GCodeWord Machine::G61_1(GCodeWord::G, 61.1);
const GCodeWord Machine::G64(GCodeWord::G, 64);

const GCodeWord Machine::G80(GCodeWord::G, 80);
const GCodeWord Machine::G90(GCodeWord::G, 90);
const GCodeWord Machine::G90_1(GCodeWord::G, 90.1);
const GCodeWord Machine::G91(GCodeWord::G, 91);
const GCodeWord Machine::G91_1(GCodeWord::G, 91.1);
const GCodeWord Machine::G93(GCodeWord::G, 93);
const GCodeWord Machine::G94(GCodeWord::G, 94);
const GCodeWord Machine::G95(GCodeWord::G, 95);
const GCodeWord Machine::G97(GCodeWord::G, 97);

const GCodeWord Machine::M01(GCodeWord::M, 1);
const GCodeWord Machine::M02(GCodeWord::M, 2);
const GCodeWord Machine::M03(GCodeWord::M, 3);
const GCodeWord Machine::M04(GCodeWord::M, 4);
const GCodeWord Machine::M05(GCodeWord::M, 5);
const GCodeWord Machine::M06(GCodeWord::M, 6);
const GCodeWord Machine::M09(GCodeWord::M, 9);

void Machine::Preamble()
{
	const Type& m_Type = m_Private->m_Type;
	const MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

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

	GCodeLine line;

	line += G00;

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

	// Select coordinate system 1
	line += G54;

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

GCodeWord Machine::AxisToWord(const Axis& axis)
{
	switch(axis)
	{
		case Axis::Type::X:
			return {GCodeWord::X, axis};
		case Axis::Type::Y:
			return {GCodeWord::Y, axis};
		case Axis::Type::Z:
			return {GCodeWord::Z, axis};

		case Axis::Type::A:
			return {GCodeWord::A, axis};
		case Axis::Type::B:
			return {GCodeWord::B, axis};
		case Axis::Type::C:
			return {GCodeWord::C, axis};

		case Axis::Type::U:
			return {GCodeWord::U, axis};
		case Axis::Type::V:
			return {GCodeWord::V, axis};
		case Axis::Type::W:
			return {GCodeWord::W, axis};
	}

	throw std::logic_error("Unknown Axis.");
}

double Machine::MillFeedRate(double chip_load, int flutes, double spindle_speed)
{
	return chip_load * flutes * spindle_speed;
}

double Machine::MillSpindleSpeed(double cutting_speed, double cutter_diameter)
{
	return cutting_speed / (3.14159 * cutter_diameter);
}

void Machine::UpdatePosition(const Axis& axis)
{
	MachineState& m_State = m_Private->m_State;

	double* val(0);

	switch(axis)
	{
		case Axis::Type::X:
			val = &m_State.m_Current.X;
			break;
		case Axis::Type::Y:
			val = &m_State.m_Current.Y;
			break;
		case Axis::Type::Z:
			val = &m_State.m_Current.Z;
			break;

		case Axis::Type::A:
			val = &m_State.m_Current.A;
			break;
		case Axis::Type::B:
			val = &m_State.m_Current.B;
			break;
		case Axis::Type::C:
			val = &m_State.m_Current.C;
			break;

		case Axis::Type::U:
			val = &m_State.m_Current.U;
			break;
		case Axis::Type::V:
			val = &m_State.m_Current.V;
			break;
		case Axis::Type::W:
			val = &m_State.m_Current.W;
			break;
	}

	if(val)
	{
		switch(m_State.m_Motion)
		{
			case Motion::Absolute:
				*val = axis;
				break;
			case Motion::Incremental:
				*val += static_cast<double>(axis);
				break;
		}
	}
}

Machine::Machine(Type type, const std::string& gcode_variant)
 : m_Private(make_unique<Private>(type, gcode_variant))
{
	const Type& m_Type = m_Private->m_Type;
	MachineState& m_State = m_Private->m_State;

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
Machine::Machine(const Machine& m)
 : m_Private(make_unique<Private>(*m.m_Private))
{
}
Machine& Machine::operator=(const Machine& m)
{
	m_Private = make_unique<Private>(*m.m_Private);
	return *this;
}

void Machine::dump() const
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation != Rotation::Stop)
		std::cerr << m_State.m_SpindleSpeed << " RPM " << (m_State.m_SpindleRotation == Rotation::Clockwise ? "Clockwise" : "Counter-Clockwise") << "\n";
	std::cerr << m_State.m_Current.str() << "\n";
}

bool Machine::AddTool(int id, const Tool& tool)
{
	const Type& m_Type = m_Private->m_Type;
	switch(m_Type)
	{
		case Type::Mill:
			if(tool.ToolType() != Tool::type_Mill)
				throw std::logic_error("Must use Mill tool with Mill.");
			break;
		case Type::Lathe:
			if(tool.ToolType() != Tool::type_Lathe)
				throw std::logic_error("Must use Lathe tool with Lathe.");
			break;
	}
	return m_Private->m_ToolTable.AddTool(id, tool);
}
bool Machine::RemoveTool(int id)
{
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
void Machine::SetTool(int id)
{
	GCode& m_GCode = m_Private->m_GCode;
	ToolTable& m_ToolTable = m_Private->m_ToolTable;

	Tool tool;
	if(id && m_ToolTable.Get(id, &tool))
	{
		m_GCode.AddLine(GCodeLine(GCodeWord(GCodeWord::T, id), "Preload tool " + tool.Name()));
	}
	else if(id == 0)
	{
		m_GCode.AddLine(GCodeLine(GCodeWord(GCodeWord::T, id), "Preload empty tool"));
	}
	else
	{
		// Unknown tool.
		std::stringstream s;
		s << "Preload Unknown tool id: " << id;
		throw std::logic_error(s.str());
	}
}
void Machine::ToolChange(int id)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;
	ToolTable& m_ToolTable = m_Private->m_ToolTable;

	if(m_State.m_CurrentTool != id)
	{
		Tool tool;
		if(id && m_ToolTable.Get(id, &tool))
		{
			m_State.m_CurrentTool = id;

			GCodeLine line;
			line.Comment("Switch to tool " + tool.Name());
			line += GCodeWord(GCodeWord::T, id);
			line += M06;
			m_GCode.AddLine(line);
		}
		else if(id == 0)
		{
			m_State.m_CurrentTool = id;

			GCodeLine line;
			line.Comment("Empty Spindle");
			line += GCodeWord(GCodeWord::T, 0);
			line += M06;
			m_GCode.AddLine(line);
		}
		else
		{
			// Unknown tool.
			std::stringstream s;
			s << "Unknown tool id: " << id;
			throw std::logic_error(s.str());
		}
	}
}

void Machine::NewBlock(const std::string& name)
{
	m_Private->m_GCode.NewBlock(name, m_Private->m_State);
}
void Machine::EndBlock(int restore)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

//	if(restore == block_RestoreState)
//	{
//		m_GCode.AddLine(GCodeLine("Restore State"));
//
//		GCodeBlock& block = m_GCode.CurrentBlock();
//		const MachineState& saved_state = block.State();
//
//		SetUnits(saved_state.m_Units);
//		SetPlane(saved_state.m_Plane);
//		SetMotion(saved_state.m_Motion);
//		SetArcMotion(saved_state.m_ArcMotion);
//		SetFeedRateMode(saved_state.m_FeedRateMode);
//		SetFeedRate(saved_state.m_FeedRate);
//		StartSpindle(saved_state.m_SpindleSpeed, saved_state.m_SpindleRotation);
//		ToolChange(saved_state.m_CurrentTool);
//
//		if(m_State.m_Current != saved_state.m_Current)
//		{
//			// TODO determine how to safely rapid to the previous position.
//		}
//	}
//	else if(restore)
	if(restore)
	{
		m_GCode.AddLine(GCodeLine("Restore State"));

		GCodeBlock& block = m_GCode.CurrentBlock();
		const MachineState& saved_state = block.State();

		if(restore & block_RestoreUnits)
			SetUnits(saved_state.m_Units);
		if(restore & block_RestorePlane)
			SetPlane(saved_state.m_Plane);
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
	m_Private->m_GCode.AddLine(GCodeLine(G61, "Exact Path"));
}
void Machine::AccuracyExactStop()
{
	m_Private->m_GCode.AddLine(GCodeLine(G61_1, "Exact Stop"));
}
void Machine::AccuracyPathBlending()
{
	m_Private->m_GCode.AddLine(GCodeLine(G64, "Path Blend Without Tolerance"));
}
void Machine::AccuracyPathBlending(double p)
{
	GCodeLine line(G64, "Path Blend With Tolerance");
	line += GCodeWord(GCodeWord::P, p);
	m_Private->m_GCode.AddLine(line);
}
void Machine::AccuracyPathBlending(double p, double q)
{
	GCodeLine line(G64, "Path Blend With Tolerance & Folding");
	line += GCodeWord(GCodeWord::P, p);
	line += GCodeWord(GCodeWord::Q, q);
	m_Private->m_GCode.AddLine(line);
}

void Machine::OptionalPause(const std::string& comment)
{
	m_Private->m_GCode.AddLine(GCodeLine(M01, comment));
}

void Machine::SetMotion(Motion m)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_Motion != m)
	{
		m_State.m_Motion = m;

		switch(m_State.m_Motion)
		{
			case Motion::Absolute:
				m_GCode.AddLine(GCodeLine(G90, "Switch to Absolute Motion"));
				break;
			case Motion::Incremental:
				m_GCode.AddLine(GCodeLine(G91, "Switch to Incremental Motion"));
				break;
		}
	}
}
void Machine::SetArcMotion(Motion m)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_ArcMotion != m)
	{
		m_State.m_ArcMotion = m;

		switch(m_State.m_ArcMotion)
		{
			case Motion::Absolute:
				m_GCode.AddLine(GCodeLine(G90_1, "Switch to Absolute Arc Motion"));
				break;
			case Motion::Incremental:
				m_GCode.AddLine(GCodeLine(G91_1, "Switch to Incremental Arc Motion"));
				break;
		}
	}
}
void Machine::SetUnits(Units u)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_Units != u)
	{
		m_State.m_Units = u;

		switch(m_State.m_Units)
		{
			case Units::Metric:
				m_GCode.AddLine(GCodeLine(G21, "Switch to Metric (Millimeters)"));
				break;
			case Units::Imperial:
				m_GCode.AddLine(GCodeLine(G20, "Switch to Imperial (Inches)"));
				break;
		}

		if(m_State.m_FeedRate > 0.0)
		{
			std::stringstream c;
			c << "Active feed rate meaning changed to ";

			switch(m_State.m_FeedRateMode)
			{
				case FeedRateMode::InverseTime:
					c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
					break;
				case FeedRateMode::UnitsPerMinute:
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
				case FeedRateMode::UnitsPerRevolution:
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
			m_GCode.AddLine(GCodeLine(c.str()));
		}
	}
}
void Machine::SetPlane(Plane p)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_Plane != p)
	{
		m_State.m_Plane = p;

		switch(m_State.m_Plane)
		{
			case Plane::XY:
				m_GCode.AddLine(GCodeLine(G17, "Switch to XY Plane"));
				break;
			case Plane::ZX:
				m_GCode.AddLine(GCodeLine(G18, "Switch to ZX Plane"));
				break;
			case Plane::YZ:
				m_GCode.AddLine(GCodeLine(G19, "Switch to YZ Plane"));
				break;
			case Plane::UV:
				m_GCode.AddLine(GCodeLine(G17_1, "Switch to UV Plane"));
				break;
			case Plane::WU:
				m_GCode.AddLine(GCodeLine(G18_1, "Switch to WU Plane"));
				break;
			case Plane::VW:
				m_GCode.AddLine(GCodeLine(G19_1, "Switch to VW Plane"));
				break;
		}
	}
}
void Machine::SetFeedRateMode(FeedRateMode f)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_FeedRateMode != f)
	{
		m_State.m_FeedRateMode = f;

		switch(m_State.m_FeedRateMode)
		{
			case FeedRateMode::InverseTime:
				m_GCode.AddLine(GCodeLine(G93, "Switch to Inverse Time Feed Rate Mode"));
				break;
			case FeedRateMode::UnitsPerMinute:
				m_GCode.AddLine(GCodeLine(G94, "Switch to Units Per Minute Feed Rate Mode"));
				break;
			case FeedRateMode::UnitsPerRevolution:
				m_GCode.AddLine(GCodeLine(G95, "Switch to Units Per Revolution Feed Rate Mode"));
				break;
		}

		if(m_State.m_FeedRate > 0.0)
		{
			std::stringstream c;
			c << "Active feed rate meaning changed to ";

			switch(m_State.m_FeedRateMode)
			{
				case FeedRateMode::InverseTime:
					c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
					break;
				case FeedRateMode::UnitsPerMinute:
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
				case FeedRateMode::UnitsPerRevolution:
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
			m_GCode.AddLine(GCodeLine(c.str()));
		}
	}
}
void Machine::SetFeedRate(double f)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_FeedRate != f)
	{
		m_State.m_FeedRate = f;

		std::stringstream c;
		if(f > 0.0)
		{
			switch(m_State.m_FeedRateMode)
			{
				case FeedRateMode::InverseTime:
					c << "Feed Time: " << 1/f << " minutes";
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

		GCodeLine line(c.str());
		if(m_State.m_FeedRateMode != FeedRateMode::InverseTime)
			line += GCodeWord(GCodeWord::F, f);
		m_GCode.AddLine(line);
	}
}
void Machine::StartSpindle(unsigned long s, Rotation r)
{
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;
	Spindle& m_Spindle = m_Private->m_Spindle;

	unsigned long requested_speed(s);
	s = m_Spindle.Normalise(s);

	if(m_State.m_SpindleSpeed != s || m_State.m_SpindleRotation != r)
	{
		m_State.m_SpindleSpeed = s;
		m_State.m_SpindleRotation = r;

		GCodeLine line;
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
				line += GCodeWord(GCodeWord::S, s);
				c << "Start Spindle Clockwise " << s << " RPM";
				if(m_State.m_SpindleSpeed != requested_speed)
					c << " (" << requested_speed << " RPM Requested)";
				break;
			case Rotation::CounterClockwise:
				line += M04;
				line += GCodeWord(GCodeWord::S, s);
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
	MachineState& m_State = m_Private->m_State;
	GCode& m_GCode = m_Private->m_GCode;

	if(m_State.m_SpindleRotation != Rotation::Stop || m_State.m_SpindleSpeed > 0)
	{
		m_State.m_SpindleSpeed = 0;
		m_State.m_SpindleRotation = Rotation::Stop;

		m_GCode.AddLine(GCodeLine(M05, "Stop Spindle"));
	}
}

void Machine::Rapid(const Axis& axis0)
{
	GCodeLine line;

	line += G00;
	line += AxisToWord(axis0); UpdatePosition(axis0);

	m_Private->m_GCode.AddLine(line);
}
void Machine::Rapid(const Axis& axis0, const Axis& axis1)
{
	GCodeLine line;

	line += G00;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);

	m_Private->m_GCode.AddLine(line);
}
void Machine::Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2)
{
	GCodeLine line;

	line += G00;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);

	m_Private->m_GCode.AddLine(line);
}
void Machine::Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3)
{
	GCodeLine line;

	line += G00;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);
	line += AxisToWord(axis3); UpdatePosition(axis3);

	m_Private->m_GCode.AddLine(line);
}
void Machine::Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4)
{
	GCodeLine line;

	line += G00;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);
	line += AxisToWord(axis3); UpdatePosition(axis3);
	line += AxisToWord(axis4); UpdatePosition(axis4);

	m_Private->m_GCode.AddLine(line);
}
void Machine::Rapid(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4, const Axis& axis5)
{
	GCodeLine line;

	line += G00;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);
	line += AxisToWord(axis3); UpdatePosition(axis3);
	line += AxisToWord(axis4); UpdatePosition(axis4);
	line += AxisToWord(axis5); UpdatePosition(axis5);

	m_Private->m_GCode.AddLine(line);
}

void Machine::Linear(const Axis& axis0)
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw std::logic_error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw std::logic_error("Feedrate is 0.0");

	GCodeLine line;

	line += G01;
	line += AxisToWord(axis0); UpdatePosition(axis0);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
		line += GCodeWord(GCodeWord::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
}
void Machine::Linear(const Axis& axis0, const Axis& axis1)
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw std::logic_error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw std::logic_error("Feedrate is 0.0");

	GCodeLine line;

	line += G01;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
		line += GCodeWord(GCodeWord::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
}
void Machine::Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2)
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw std::logic_error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw std::logic_error("Feedrate is 0.0");

	GCodeLine line;

	line += G01;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
		line += GCodeWord(GCodeWord::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
}
void Machine::Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3)
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw std::logic_error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw std::logic_error("Feedrate is 0.0");

	GCodeLine line;

	line += G01;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);
	line += AxisToWord(axis3); UpdatePosition(axis3);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
		line += GCodeWord(GCodeWord::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
}
void Machine::Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4)
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw std::logic_error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw std::logic_error("Feedrate is 0.0");

	GCodeLine line;

	line += G01;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);
	line += AxisToWord(axis3); UpdatePosition(axis3);
	line += AxisToWord(axis4); UpdatePosition(axis4);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
		line += GCodeWord(GCodeWord::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
}
void Machine::Linear(const Axis& axis0, const Axis& axis1, const Axis& axis2, const Axis& axis3, const Axis& axis4, const Axis& axis5)
{
	MachineState& m_State = m_Private->m_State;

	if(m_State.m_SpindleRotation == Rotation::Stop)
		throw std::logic_error("Spindle is stopped");
	if(m_State.m_FeedRate == 0.0)
		throw std::logic_error("Feedrate is 0.0");

	GCodeLine line;

	line += G01;
	line += AxisToWord(axis0); UpdatePosition(axis0);
	line += AxisToWord(axis1); UpdatePosition(axis1);
	line += AxisToWord(axis2); UpdatePosition(axis2);
	line += AxisToWord(axis3); UpdatePosition(axis3);
	line += AxisToWord(axis4); UpdatePosition(axis4);
	line += AxisToWord(axis5); UpdatePosition(axis5);

	if(m_State.m_FeedRateMode == FeedRateMode::InverseTime)
	{
		std::stringstream c;
		c << "Feed Time: " << 1/m_State.m_FeedRate << " minutes";
		line += GCodeWord(GCodeWord::F, m_State.m_FeedRate, c.str());
	}

	m_Private->m_GCode.AddLine(line);
}

Machine::~Machine()
{
	m_Private->m_GCode.AddLine(GCodeLine(M02, "End of program."));

	std::cout << m_Private->m_GCode.str();
}

