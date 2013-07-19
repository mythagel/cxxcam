#include "Machine.h"
#include "Tool.h"
#include "Axis.h"
#include <iostream>

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill);
	
	{
		auto end_mill = Tool::Mill{};
		end_mill.type = Tool::Mill::Type::End;
		end_mill.center_cutting = false;
		end_mill.flutes = 2;
		end_mill.flute_length = 10;
		end_mill.core_diameter = 0.25;
		end_mill.cutting_length = 10;
		end_mill.mill_diameter = 1;
		end_mill.shank_diameter = 3;
		end_mill.length = 30;
		m.AddTool(5, Tool("1mm End Mill", end_mill));
	}
	m.ToolChange(5);
	
	m.SetFeedRateMode(Machine::FeedRateMode::InverseTime);
	m.StartSpindle(300);
	
	m.SetFeedRate(100);
	m.Linear({Y(5), X(6)});
	
	m.SetFeedRate(0.375);
	m.Linear({Y(100)});
	
	std::cout << m;
	return 0;
}

