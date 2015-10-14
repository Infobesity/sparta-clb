#pragma once

#include "generic.hpp"

namespace system_info
{
	unsigned int get_hdd_serial_number();

	std::string get_local_mac_address();
	std::string get_local_mac_serial();

	bool get_machine_id(unsigned char* machine_id);
	std::string get_cookie_string();
}