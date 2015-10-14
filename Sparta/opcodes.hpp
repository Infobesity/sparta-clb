#pragma once

#include "generic.hpp"

namespace maplestory
{
	namespace opcode
	{
		namespace in
		{
			enum // base
			{
				alive_req					= 0x0011,		// keep-alive request
				security_packet				= 0x0016,		// security-client request
			};

			enum // login
			{
				check_guest_id_login_result = 0x0001,		// 
				select_world_result			= 0x0003,		// 
				world_information			= 0x0006,		//
				select_channel_result		= 0x0007,		//
				select_character_result		= 0x0008,		// 
				login_map					= 0x0025,		//
			};

			enum // game
			{
				stat_changed				= 0x0029,		// 
				set_field					= 0x015B,		// 
				script_message				= 0x0404,		// 
			};
		}

		namespace out
		{
			enum // base
			{
				security_packet				= 0x0031,		// security-client response
				alive_ack					= 0x004B,		// keep-alive response
			};

			enum // login
			{
				select_world				= 0x001C,		//
				init_guest_id_login			= 0x0019,		//
				logo_done					= 0x0039,		//
				client_ready				= 0x003B,		//
				handshake					= 0x0043,		// login-server handshake validation
				select_channel				= 0x0047,		//
				select_character			= 0x004C,		//
			};

			enum // game
			{
				migrate_in					= 0x0023,		//
				user_script_message_answer	= 0x0080,		//
				start_script				= 0x01BB,		// 
			};
		}
	}
}