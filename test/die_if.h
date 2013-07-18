#ifndef CXXCAM_ASSERT_H_
#define CXXCAM_ASSERT_H_
#include <string>
#include <stdexcept>

inline void die_if(bool cond, std::string message = {})
{
	if(cond)
		throw std::runtime_error(message);
}

#endif /* CXXCAM_ASSERT_H_ */
