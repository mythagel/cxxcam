#include "fold_adjacent.h"
#include <numeric>
#include <vector>
#include <iostream>
#include "die_if.h"
int main()
{
    const std::vector<unsigned int> expected = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    
	{    
		std::vector<unsigned int> v = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		std::adjacent_difference(v.begin(), v.end() - 1, v.begin() + 1, std::plus<int>());
		die_if(v != expected);
    }
    
    {
		std::vector<unsigned int> v = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		fold_adjacent(v.begin(), v.end() - 1, v.begin() + 1, std::plus<int>());
		die_if(v != expected);
    }
    
	return 0;
}
