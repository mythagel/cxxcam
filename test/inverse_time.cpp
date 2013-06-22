#include "Machine.h"
#include "Tool.h"
#include "Axis.h"
#include <iostream>

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill);
	m.SetFeedRateMode(Machine::FeedRateMode::InverseTime);
	m.StartSpindle(300);
	
	m.SetFeedRate(100);
	m.Linear({Y(5), X(6)});
	
	m.SetFeedRate(0.375);
	m.Linear({Y(100)});
	
	std::cout << m;
	return 0;
}

