#include "login_stage.hpp"

#include "opcodes.hpp"
#include "system_info.hpp"

#include "nexon_api\NMCOClientObject.h"

namespace maplestory
{
	namespace stage
	{
		login_stage::login_stage(maple_client* client, maplestory::account& account)
			: stage_base(false), client(client), account(account), succesfully_logged_in(false)
		{
	
		}

		login_stage::~login_stage()
		{
			printf("[login_stage] leaving stage\n\n");
		}

		bool login_stage::on_enter_stage()
		{
			printf("[login_stage] entering stage\n");
			
			network::io_packet packet(opcode::out::client_ready);
			return this->send_packet(packet);
		}
		
		bool login_stage::handle_handshake(unsigned char locale, unsigned short version_major, unsigned short version_minor)
		{
			network::io_packet packet(opcode::out::handshake);
			packet.write1(locale);
			packet.write2(version_major);
			packet.write2(version_minor);
			packet.write1(0x00);

			return this->send_packet(packet);
		}

		bool login_stage::handle_packet(unsigned short header, network::io_packet& packet)
		{
			switch (header)
			{
			case opcode::in::login_map:
				return this->on_login_map();
		
			case opcode::in::world_information:
				return this->on_world_information(packet);

			case opcode::in::select_world_result:
				return this->on_select_world_result(packet);

			case opcode::in::check_guest_id_login_result:
				return this->on_check_guest_id_login_result(packet);

			case opcode::in::select_channel_result:
				return this->on_select_channel_result(packet);
	
			case opcode::in::select_character_result:
				return this->on_select_character_result(packet);

			default:
				//printf("[login_stage] unhandled packet with header: %04X\n", header);
				break;
			}

			return true;
		}
		
		bool login_stage::send_select_world()
		{
			network::io_packet packet(opcode::out::select_world);
			packet.write2(this->account.world_id());

			return this->send_packet(packet);
		}

		bool login_stage::send_select_channel()
		{
			NMLoginAuthReplyCode auth_reply;

			if ((auth_reply = CNMCOClientObject::GetInstance().LoginAuth(this->account.username().c_str(), this->account.password().c_str())) != kLoginAuth_OK)
			{
				switch (auth_reply)
				{
				case kLoginAuth_BlockedIP:
					printf("Couldn't log in (blocked IP)...\n");
					break;

				case kLoginAuth_WrongID:
				case kLoginAuth_UserNotExists:
					printf("Couldn't log in (wrong username)...\n");
					break;

				case kLoginAuth_WrongPassword:
					printf("Couldn't log in (wrong password)...\n");
					break;

				default:
					printf("Couldn't log in (unknown reason)...\n");
					break;
				}

				return false;
			}

			char nexon_passport[1024];
	
			if (!CNMCOClientObject::GetInstance().GetNexonPassport(nexon_passport))
			{
				return false;
			}

			unsigned char machine_id[16];
			memset(machine_id, 0, sizeof(machine_id));

			if (!system_info::get_machine_id(machine_id))
			{
				return false;
			}

			network::io_packet packet(opcode::out::select_channel);
			packet.write2(0x0003);
			packet.write_string(std::string(nexon_passport));
			packet.write_string(system_info::get_cookie_string());
			packet.write_buffer(machine_id, sizeof(machine_id));
			packet.write4(0x000000000);
			packet.write1(this->account.world_id());
			packet.write1(this->account.channel_id());
			packet.write2(static_cast<unsigned short>(rand()));
			packet.write2(static_cast<unsigned short>(rand()));
			
			return this->send_packet(packet);
		}

		bool login_stage::send_select_character()
		{
			network::io_packet packet(opcode::out::select_character);
			packet.write_string(this->account.pic());
			packet.write4(this->characters[this->account.character_id()].id);
			packet.write1(0x00);
			
			return this->send_packet(packet);
		}
		
		bool login_stage::on_login_map()
		{
			this->worlds.clear();
			
			network::io_packet packet_1(opcode::out::init_guest_id_login);
			network::io_packet packet_2(opcode::out::logo_done);
			
			return (this->send_packet(packet_1) && this->send_packet(packet_2));
		}

		bool login_stage::on_world_information(network::io_packet& packet)
		{
			world_info world;
			world.id() = packet.read1();
	
			if (world.id() == 0xFF)
			{
				return this->send_select_world();
			}

			world.name() = packet.read_string();
			world.state() = packet.read1();
			world.event_description() = packet.read_string();
			world.event_exp_rate() = packet.read2();
			world.event_drop_rate() = packet.read2();
			world.channel_count() = packet.read1();

			for (unsigned char i = 0; i < world.channel_count(); i++)
			{
				channel_info channel;
				channel.name() = packet.read_string();
				channel.user_number() = packet.read4();
				channel.world_id() = packet.read1();
				channel.id() = packet.read1();
				channel.adult_channel() = packet.read1();

				world.channels().push_back(channel);
			}
			
			this->worlds.push_back(world);
			return true;
		}
		
		bool login_stage::on_select_world_result(network::io_packet& packet)
		{
			packet.read2(); // world id
			return this->send_select_channel();
		}
		
		bool login_stage::on_check_guest_id_login_result(network::io_packet& packet)
		{
			if (!CNMCOClientObject::GetInstance().LogoutAuth())
			{
				return false;
			}

			if (packet.read1())
			{
				printf("Failed to log in...\n");
				return false;
			}

			this->client->session().account_id() = packet.read4();	// dwAccountId
			packet.read1();											// 
			packet.read1();											// 
			packet.read1();											// 
			packet.read1();											// 
			packet.read4();											// 
			packet.read4();											// 
			packet.read4();											// 
			packet.read4();											// 
			packet.read4();											// 
			std::string account_name = packet.read_string();		// sNexonClubID

			//printf("Succesfully logged in to %s...\n", account_name.c_str());
			return (this->succesfully_logged_in = true);
		}
		
		bool login_stage::on_select_channel_result(network::io_packet& packet)
		{
			this->characters.clear();

			if (!this->succesfully_logged_in)
			{
				printf("-> Not yet logged in.\n");
				return false;
			}

			packet.read4();
			packet.read8();

			packet.read1();
			unsigned char character_count = packet.read1();

			if (!character_count)
			{
				printf("-> There's no characters available on this server.\n");
				return true;
			}

			for (unsigned char i = 0; i < character_count; i++)
			{
				character_info character_infos;
				character_infos.decode(packet);

				character_look character_looks;
				character_looks.decode(packet);

				packet.read1();	// View from VAC
				unsigned char on_ranking = packet.read1();

				if (on_ranking)
				{
					packet.indent(16); // Rankings (World Rank, World Rank Move, Job Rank, Job Rank Move)
				}
				
				//printf("-> Character (%d): %s\n", i, character_infos.name.c_str());
				this->characters.push_back(character_infos);
			}
			
			packet.read1();
			packet.read1();
			packet.read4();
			packet.read4();
			packet.read4();
			packet.read4();
			packet.read4();
			packet.read1();
			this->client->session().session_id() = packet.read8();

			return this->send_select_character();
		}
		
		bool login_stage::on_select_character_result(network::io_packet& packet)
		{
			packet.read1();
			packet.read1();
	
			unsigned int transfer_ip_1 = packet.read4();
			unsigned short transfer_port_1 = packet.read2();

			unsigned int transfer_ip_2 = packet.read4();
			unsigned short transfer_port_2 = packet.read2();

			this->client->session().character_id() = packet.read4();

			char transfer_ip_string[16];
			memset(transfer_ip_string, 0, sizeof(transfer_ip_string));
			
			unsigned char* transfer_ip_buffer = reinterpret_cast<unsigned char*>(&transfer_ip_1);
			sprintf(transfer_ip_string, "%i.%i.%i.%i", transfer_ip_buffer[0], transfer_ip_buffer[1], transfer_ip_buffer[2], transfer_ip_buffer[3]);
	
			return this->client->migrate(server_type::game, std::string(transfer_ip_string), transfer_port_1);
		}
	}
}