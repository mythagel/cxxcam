#include "rs274ngc.hh"
#include "rs274ngc_return.hh"
#include <iostream>
#include <string>

// for canon_pre
FILE * _outfile;

int main()
{
	_outfile = stdout;
	
	if(rs274ngc_init() != RS274NGC_OK)
		return 1;

	std::string line;
	while(std::getline(std::cin, line))
	{
		int status;
		
		status = rs274ngc_read(line.c_str());
		if(status != RS274NGC_OK)
		{
			if(status != RS274NGC_EXECUTE_FINISH)
			{
				std::cerr << "Error readine line!: \n";
				std::cerr << line <<"\n";
				return status;
			}
		}
		
		status = rs274ngc_execute();
		if(status != RS274NGC_OK)
			return status;
	}

    rs274ngc_exit();
    return 0;
}

