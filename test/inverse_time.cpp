#include "Machine.h"
#include "Tool.h"
#include "Axis.h"
#include <iostream>

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill);
	m.SetFeedRateMode(Machine::FeedRateMode::InverseTime);
	m.SetFeedRate(100);
	m.StartSpindle(300);
	m.Linear({Y(5), X(6)});
	
	std::cout << m;
	return 0;
}

