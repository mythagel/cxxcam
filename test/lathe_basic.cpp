#include <cstdio>
#include "Machine.h"
#include "Tool.h"
#include "Axis.h"

int main()
{
	Machine m(Machine::Type::Lathe, "LinuxCNC");

	m.SetUnits(Machine::Units::Imperial);
	m.SetFeedRate(100);
	m.AccuracyExactStop();
	m.SetUnits(Machine::Units::Metric);
	m.SetFeedRate(100);
	m.AccuracyExactPath();

	m.SetMotion(Machine::Motion::Incremental);
	return 0;
}

