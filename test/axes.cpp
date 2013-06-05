#include "Limits.h"

using namespace cxxcam;
using namespace cxxcam::limits;

int main()
{
	{
		AvailableAxes x;
		x.validate(Axis::Type::X);
	}
	
	{
		AvailableAxes x({Axis::Type::X, Axis::Type::Y});
		x.validate(Axis::Type::X);
	}
	return 0;
}
