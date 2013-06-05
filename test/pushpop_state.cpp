#include "Machine.h"
#include "Tool.h"
#include "Axis.h"
#include <iostream>

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill);
	m.AddSpindleDiscrete(100);
	m.AddSpindleDiscrete(250);

	m.NewBlock("Setup");
	// Roughing
	m.AccuracyPathBlending(0.01, 0.01);

	m.NewBlock("Program");
	m.OptionalPause("Pause before program");

	{
		auto end_mill = Tool::Mill{};
		end_mill.type = Tool::Mill::Type::End;
		end_mill.center_cutting = false;
		end_mill.flutes = 4;
		end_mill.flute_length = 30;
		end_mill.core_diameter = 5;
		end_mill.cutting_length = 28;
		end_mill.mill_diameter = 10;
		end_mill.shank_diameter = 10;
		end_mill.length = 60;
		m.AddTool(4, Tool("10mm End Mill", end_mill));
	}
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
	m.SetTool(4);

	m.PushState();
	m.PushState();

	m.Rapid({Y(5), X(6), A(1.0/3.0)});
	m.ToolChange(4);

	m.StartSpindle(300);

	m.NewBlock("Positioning");
	m.StopSpindle();
	m.ToolChange(5);
	m.SetMotion(Machine::Motion::Incremental);
	m.Rapid({X(1), Y(1)});
	m.Rapid({X(1)});
	m.Rapid({X(1)});
	m.Rapid({X(1), Z(2)});
	m.Rapid({X(1)});
	m.EndBlock(Machine::block_RestoreSpindle); // Don't restore tool


	m.NewBlock("Cutting");
	m.StartSpindle(100);
	m.SetFeedRate(100);
	m.SetMotion(Machine::Motion::Absolute);
	m.Rapid({X(0), Y(0), Z(0)});
	m.Linear({X(5), Y(5), Z(5)});

	m.SetFeedRateMode(Machine::FeedRateMode::InverseTime);
	m.Linear({X(5), Y(5), Z(5)});

	m.EndBlock(Machine::block_RestoreState);

	m.NewBlock("Epilogue");
	m.StopSpindle();

	std::cerr << "state at EOF\n";	
	m.dump();
	
	m.DiscardState();
	std::cerr << "state after discarding\n";
	m.dump();
	
	m.PopState();
	std::cerr << "state after restoring\n";
	m.dump();
	return 0;
}

