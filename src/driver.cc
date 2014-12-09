#include "log_interpreter.h"
#include "rs274ngc_return.hh"
#include <iostream>
#include <string>

int main()
{
	log_interpreter interp;

	std::string line;
	while(std::getline(std::cin, line))
	{
		int status;
		
		status = interp.read(line.c_str());
		if(status != RS274NGC_OK)
		{
			if(status != RS274NGC_EXECUTE_FINISH)
			{
				std::cerr << "Error reading line!: \n";
				std::cerr << line <<"\n";
				return status;
			}
		}
		
		status = interp.execute();
		if(status != RS274NGC_OK)
			return status;
	}

    interp.exit();
    return 0;
}

