#include "Machine.h"
#include "Tool.h"
#include "Axis.h"

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill);
	m.SetFeedRateMode(Machine::FeedRateMode::InverseTime);
	m.SetFeedRate(100);
	m.Rapid({Y(5), X(6)});
	return 0;
}

