#include "Machine.h"
#include "Tool.h"
#include "Axis.h"
#include "Stock.h"
#include "geom/polyhedron.h"
#include "geom/primitives.h"
#include "geom/io.h"
#include <iostream>
#include <fstream>

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill, Machine::Units::Metric, "Generic", {});
	m.SetMachineAxes("XYZA");
	m.AddSpindleRange(100, 1000);
	m.AddSpindleRange(2000, 6000);

	{
		auto end_mill = Tool::Mill{};
		end_mill.type = Tool::Mill::Type::End;
		end_mill.center_cutting = true;
		end_mill.flutes = 4;
		end_mill.flute_length = 25;
		end_mill.core_diameter = 3;
		end_mill.cutting_length = 20;
		end_mill.mill_diameter = 3;
		end_mill.shank_diameter = 6;
		end_mill.length = 60;
		m.AddTool(1, Tool("3mm End Mill", end_mill));
	}
	
	Stock stock;
	stock.Model = geom::make_box({x:0, y:0, z:0}, {x:50, y:50, z:10});
	m.SetStock(stock);

	m.SetFeedRate(100);
	m.ToolChange(1);
	m.StartSpindle(250);

	m.Rapid({Z(11)});
	m.Rapid({X(0), Y(0)});

	for(int i = 0; i < 11; ++i)
	{
		std::cerr << "{z:" << (10-i) << "}\n";
		m.Linear({Z(10-i)});
		std::cerr << "{x:" << 5*i << ", y:" << 5*i << "}\n";
		m.Linear({X(5*i), Y(5*i)});
		//m.linear({x:5*i, y:5*i, z:10-i});
	}
	
	std::ofstream os("playjs.off");
	os << geom::format::off << m.GetStock().Model;
	std::cout << m;
	return 0;
}

