#include "Machine.h"
#include "Stock.h"
#include "geom/io.h"

using namespace cxxcam;

int main()
{
	Machine m(Machine::Type::Mill);
	auto stock = m.GetStock();
	auto stock_object = to_object(stock.Model);
	return 0;
}

