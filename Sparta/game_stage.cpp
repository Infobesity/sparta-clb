#include "game_stage.hpp"

#include "opcodes.hpp"
#include "system_info.hpp"

#include "output.hpp"
#include "sparta_form.hpp"

#include "character_data.hpp"

namespace maplestory
{
	namespace stage
	{
		game_stage::game_stage(maple_client* client, maplestory::account& account, maplestory::session& session)
			: stage_base(false), client(client), account(account), session(session)
		{

		}

		game_stage::~game_stage()
		{
			printf("[game_stage] leaving stage\n\n");
		}

		bool game_stage::on_enter_stage()
		{
			printf("[game_stage] entering stage\n");

			unsigned char machine_id[16];
			memset(machine_id, 0, sizeof(machine_id));

			if (!system_info::get_machine_id(machine_id))
			{
				return false;
			}

			network::io_packet packet(opcode::out::migrate_in);
			packet.write4(this->account.world_id());
			packet.write4(this->session.character_id());
			packet.write_buffer(machine_id, sizeof(machine_id));
			packet.write1(0x00);
			packet.write1(0x00);
			packet.write8(this->session.session_id());

			return this->send_packet(packet);
		}
		
		bool game_stage::handle_packet(unsigned short header, network::io_packet& packet)
		{
			switch (header)
			{
			case opcode::in::stat_changed:
				return this->on_stat_changed(packet);

			case opcode::in::set_field:
				return this->on_set_field(packet);
				
			case opcode::in::script_message:
				return this->on_script_message(packet);

			default:
				//printf("[game_stage] unhandled packet with header: %04X\n", header);
				break;
			}

			return true;
		}

		bool game_stage::send_start_script()
		{
			network::io_packet packet(opcode::out::start_script);
			packet.write1(0x02);

			return this->send_packet(packet);
		}
		
		bool game_stage::send_user_script_message_answer()
		{
			network::io_packet packet(opcode::out::user_script_message_answer);
			packet.write1(0x02);
			packet.write1(0x01);

			return this->send_packet(packet);
		}
		
		bool game_stage::on_script_message(network::io_packet& packet)
		{
			packet.read2();									// nSpeakerTypeID
			packet.read4();									// nSpeakerTemplateID
			unsigned char message_type = packet.read1();	// nMsgType

			switch (message_type)
			{
			case 0x02:	// CScriptMan::OnAskYesNo
				return this->send_user_script_message_answer();

			default:
				break;
			}

			return true;
		}
		
		bool game_stage::on_stat_changed(network::io_packet& packet)
		{
			packet.read1(); // ExclRequestSent flag
			unsigned int stat_flag_1 = packet.read4();
			unsigned int stat_flag_2 = packet.read4();
			
			if (stat_flag_1 & 1)
			{
				packet.read1(); // skin
			}
			if (stat_flag_1 & 2)
			{
				packet.read4(); // face
			}
			if (stat_flag_1 & 4)
			{
				packet.read4(); // hair
			}
			if (stat_flag_1 & 8)
			{
				packet.indent(8); // aliPetLockerSN[0]
			}
			if (stat_flag_1 & 0x80000)
			{
				packet.indent(8); // aliPetLockerSN[1]
			}
			if (stat_flag_1 & 0x100000)
			{
				packet.indent(8); // aliPetLockerSN[2]
			}
			if (stat_flag_1 & 0x10)
			{
				sparta::gui::sparta_form::get_instance().update(sparta::gui::level, static_cast<unsigned int>(packet.read1()));
			}
			if (stat_flag_1 & 0x20)
			{
				packet.read2(); // nJob
			}
			if (stat_flag_1 & 0x40)
			{
				packet.read2(); // nSTR
			}
			if (stat_flag_1 & 0x80)
			{
				packet.read2(); // nDEX
			}
			if (stat_flag_1 & 0x100 )
			{
				packet.read2(); // nINT			
			}
			if (stat_flag_1 & 0x200 )
			{
				packet.read2(); // nLUK
			}
			if (stat_flag_1 & 0x400 )
			{
				packet.read4(); // nHP
			}
			if (stat_flag_1 & 0x800 )
			{
				packet.read4(); // nMHP
			}
			if (stat_flag_1 & 0x1000 )
			{
				packet.read4(); // nMP
			}
			if (stat_flag_1 & 0x2000 )
			{
				packet.read4(); // nMMP
			}
			if (stat_flag_1 & 0x4000 )
			{
				packet.read2(); // nAP
			}
			if (stat_flag_1 & 0x8000 )
			{
				// nSP -> cbf doing this one.
			}
			if (stat_flag_1 & 0x10000 )
			{
				sparta::gui::sparta_form::get_instance().update(sparta::gui::experience, packet.read8());
			}
			if (stat_flag_1 & 0x20000 )
			{
				packet.read2(); // nPOP
			}
			if (stat_flag_1 & 0x40000 )
			{
				sparta::gui::sparta_form::get_instance().update(sparta::gui::mesos, packet.read8());
			}

			return true;
		}

		bool game_stage::on_set_field(network::io_packet& packet)
		{
			auto CClientOptMan__DecodeOpt = [&](network::io_packet& packet)
			{
				unsigned int nSize = packet.read4();
	
				if (nSize != 0)
				{
					if (nSize > 0)
					{
						for (unsigned int i = 0; i < nSize; i++)
						{
							unsigned int dwType = packet.read4();
							unsigned int nValue = packet.read4();
						}
					}
		
					packet.read1();
				}
			};

			CClientOptMan__DecodeOpt(packet);

			packet.read4(); // dwChannelId

			packet.read1();
			packet.read1();	

			packet.read4();
			packet.read1(); 
	
			packet.read4();
	
			unsigned char bCharacterData = packet.read1(); 
			unsigned short nNotifierCheck = packet.read2();
			
			if (nNotifierCheck != 0)
			{
				std::vector<std::string> asNotifierContent;
				std::string sNotifierTitle = packet.read_string();
		
				if (nNotifierCheck > 0)
				{
					for (unsigned int i = 0; i < nNotifierCheck; i++)
					{
						std::string sNotifierContent = packet.read_string();
						asNotifierContent.push_back(sNotifierContent);
					}
				}
			}
			
			unsigned int map_id = 0;

			if (bCharacterData)
			{
				unsigned int s1 = packet.read4();
				unsigned int s2 = packet.read4();
				unsigned int s3 = packet.read4();
		
				character_data char_data;
				char_data.decode(packet);
				
				sparta::gui::sparta_form::get_instance().update(sparta::gui::level, char_data.level);
				sparta::gui::sparta_form::get_instance().update(sparta::gui::experience, char_data.experience);
				sparta::gui::sparta_form::get_instance().update(sparta::gui::mesos, char_data.mesos);

				map_id = char_data.map_id;
			}
			else
			{
				unsigned char bRevive = packet.read1();
		
				unsigned int dwPosMap = packet.read4();
				unsigned char nPortal = packet.read1();
				unsigned int nHP = packet.read4();
				unsigned char m_bChaseEnable = packet.read1();
		
				if (m_bChaseEnable)
				{
					unsigned int m_nTargetPosition_X = packet.read4();
					unsigned int m_nTargetPosition_Y = packet.read4();
				}

				map_id = dwPosMap;
			}
			
			sparta::gui::sparta_form::get_instance().update(sparta::gui::map_id, map_id);

			if (map_id == 302000000)
			{
				return this->send_start_script();
			}

			return true;
		}
	}
}