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
	Machine m(Machine::Type::Mill, Machine::Units::Metric, "Generic");
	m.AddSpindleRange(100, 3000);

	{
		auto end_mill = Tool::Mill{};
		end_mill.type = Tool::Mill::Type::End;
		end_mill.center_cutting = true;
		end_mill.flutes = 4;
		end_mill.flute_length = 25;
		end_mill.core_diameter = 2;
		end_mill.cutting_length = 20;
		end_mill.mill_diameter = 3;
		end_mill.shank_diameter = 6;
		end_mill.length = 60;
		m.AddTool(1, Tool("3mm End Mill", end_mill));
	}
	
	Stock stock;
	stock.Model = geom::make_box({x:0, y:0, z:0}, {x:100, y:100, z:10});
	m.SetStock(stock);

	m.SetFeedRate(100);
	m.ToolChange(1);
	m.StartSpindle(250);

	m.Rapid({Z(11)});
	m.Rapid({X(0), Y(0)});
	
	m.Linear({Z(9)});
	m.Linear({X(100), Y(100)});
	m.Linear({Z(11)});
	
	m.Linear({X(0)});

	for(int i = 0; i < 11; ++i)
	{
		std::cerr << "{z:" << (10-i) << "}\n";
		m.Linear({Z(10-i)});
		std::cerr << "{x:" << 5*i << ", y:" << 5*i << "}\n";
		m.Linear({X(5*i), Y(5*i)});
		//m.linear({x:5*i, y:5*i, z:10-i});
	}
	
	std::ofstream os("play.off");
	os << geom::format::off << m.GetStock().Model;
	return 0;
}

