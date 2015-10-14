#include "generic.hpp"

#include <iomanip>
#pragma comment(lib, "version")

namespace my_time
{
	std::string get_time()
	{
		std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
		std::time_t time_now = std::chrono::system_clock::to_time_t(time_point_now);
		std::put_time(std::localtime(&time_now), "%F %T");

		tm _time_now = *std::localtime(&time_now);

		char time_buffer[32];
		memset(time_buffer, 0, sizeof(time_buffer));
		strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", &_time_now);

		return std::string(time_buffer);
	}
}

namespace utility
{
	std::string upper(std::string const& string)
	{
		std::string upper_string;
		std::transform(string.begin(), string.end(), std::back_inserter(upper_string), toupper);
		return upper_string;
	}
	
	std::string lower(std::string const& string)
	{
		std::string lower_string;
		std::transform(string.begin(), string.end(), std::back_inserter(lower_string), tolower);
		return lower_string;
	}
}