#include "Configuration.h"
#include "Tool.h"
#include "Axis.h"

using namespace cxxcam;

int main()
{
	Configuration c;
	
	auto machine = c.Construct();

	machine->dump();

	return 0;
}

