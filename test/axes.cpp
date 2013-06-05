#include "Limits.h"

using namespace cxxcam;
using namespace cxxcam::limits;

int main()
{
	{
		AvailableAxes x;
		x.Validate(Axis::Type::X);
	}
	
	{
		AvailableAxes x({Axis::Type::X, Axis::Type::Y});
		x.Validate(Axis::Type::X);
	}
	return 0;
}
