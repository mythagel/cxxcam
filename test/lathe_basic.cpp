#include <iostream>
#include "Machine.h"
#include "Tool.h"
#include "Axis.h"

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Lathe);

	m.SetUnits(Machine::Units::Imperial);
	m.SetFeedRate(100);
	m.AccuracyExactStop();
	m.SetUnits(Machine::Units::Metric);
	m.SetFeedRate(100);
	m.AccuracyExactPath();

	m.SetMotion(Machine::Motion::Incremental);
	std::cout << m;
	return 0;
}

