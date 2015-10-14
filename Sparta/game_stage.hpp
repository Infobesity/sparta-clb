#pragma once

#include "generic.hpp"
#include "stage_base.hpp"

#include "maple_client.hpp"

#include "account.hpp"
#include "session.hpp"

namespace maplestory
{
	namespace stage
	{
		class game_stage : public stage_base
		{
		public:
			game_stage(maple_client* client, account& account, session& session);
			~game_stage();
			
		protected:
			bool on_enter_stage();

			bool handle_packet(unsigned short header, network::io_packet& packet);

		private:
			bool send_start_script();
			bool send_user_script_message_answer();
			
			bool on_script_message(network::io_packet& packet);
			bool on_stat_changed(network::io_packet& packet);
			bool on_set_field(network::io_packet& packet);

		private:
			maple_client* client;

			account account;
			session session;
		};
	}
}