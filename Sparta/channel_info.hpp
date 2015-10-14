#pragma once

#include "generic.hpp"
#include "io_packet.hpp"

namespace maplestory
{	
	/* CLogin::CHANNELITEM */
	class channel_info
	{
	public:
		channel_info()
			: name_(""), user_number_(0), world_id_(0), id_(0), adult_channel_(0)
		{
			
		}

		std::string& name()
		{
			return name_;
		}
		
		unsigned int& user_number()
		{
			return user_number_;
		}
		
		unsigned char& world_id()
		{
			return world_id_;
		}
		
		unsigned char& id()
		{
			return id_;
		}
		
		unsigned char& adult_channel()
		{
			return this->adult_channel_;
		}

	private:
		std::string name_;
		unsigned int user_number_;
		unsigned char world_id_;
		unsigned char id_;
		unsigned char adult_channel_;
	};
}