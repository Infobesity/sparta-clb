#pragma once

#include "generic.hpp"
#include "io_packet.hpp"

#include "character_info.hpp"

namespace maplestory
{
	typedef struct _character_data
	{
		unsigned int level;
		unsigned __int64 mesos;
		unsigned __int64 experience;
		unsigned int map_id;

		void decode(network::io_packet& packet)
		{
			unsigned int dbcharFlag_1 = packet.read4();
			unsigned int dbcharFlag_2 = packet.read4();
		
			packet.read1();
		
			for (unsigned int i = 0; i < 3; i++)
			{
				packet.read4();
			}
		
			unsigned char u1 = packet.read1();
		
			if (u1 > 0)
			{
				for (unsigned int i = 0; i < u1; i++)
				{
					packet.read4();
				}
			}
		
			unsigned int buffer_count = packet.read4();
		
			if (buffer_count > 0)
			{
				for (unsigned int i = 0; i < buffer_count; i++)
				{
					packet.read4();
					packet.indent(8); // read_buffer
				}
			}
		
			unsigned char u2 = packet.read1();
		
			if (u2)
			{
				packet.read1(); // Not used in the client itself, only applied for packet alignment. 
			
				for (unsigned int i = 0; i < 2; i++)
				{
					unsigned int u3 = packet.read4();
				
					if (u3 > 0)
					{
						for (unsigned int j = 0; j < u3; j++)
						{
							packet.indent(8); // read_buffer
						}
					}
				}
			}
		
			if (dbcharFlag_1 & 0x01)
			{
				character_info char_info;
				char_info.decode(packet);
			
				this->level = char_info.level;
				this->experience = char_info.stat_exp;
				this->map_id = char_info.map_id;

				packet.read1();
				packet.read1();
			
				for (unsigned int i = 0; i < 3; i++)
				{
					if (packet.read1())
					{
						packet.read_string();
					}
				}
			}
		
			if (dbcharFlag_1 & 0x02)
			{
				this->mesos = packet.read8();
				packet.read1();
				packet.read4();
			}
		
			if ((dbcharFlag_1 & 0x08) || (dbcharFlag_1 & 0x8000000))
			{
				unsigned int u5 = packet.read4();
			
				if (u5 > 0)
				{
					for (unsigned int i = 0; i < u5; i++)
					{
						packet.read4();
						packet.read4();
						packet.read4();
						packet.read8();
					}
				}
			}
		
			for (unsigned int i = 0; i < 5; i++)
			{
				unsigned char inventory_size = 0;
			
				if (dbcharFlag_1 & 0x80)
				{
					inventory_size = packet.read1(); // 1C, 1C, 1C, 1C, 60
				}
			
				if (inventory_size)
				{
					/* (Re)Allocate inventory spaces */
				}
			}
		
			if (dbcharFlag_1 & 0x100000)
			{
				packet.read4(); // CharacterData->aEquipExtExpire[0].dwLowDateTime
				packet.read4(); // CharacterData->aEquipExtExpire[0].dwHighDateTime
			}
		
			/*
			if (dbcharFlag_1 & 0x04)
			{
				// Equipped1 inventory, Equipped2 inventory, Equip inventory tab
				packet.read1();
			
				for (unsigned int i = 0; i < 3; i++)
				{
					for (unsigned int j = 0; j < 57; j++)
					{
						// Set inventory slots to zero
					}			
				
					unsigned short initial_slot_id = packet.read2();
				
					if (initial_slot_id != 0)
					{
						for (unsigned int j = initial_slot_id; (j != 0); j = packet.read2())
						{						
							GW_ItemSlotBase__Decode(iPacket);
						}
					}
				}
			
				// Android inventory (heart)
				packet.read1();
			
				if (packet.read2())
				{
					GW_ItemSlotBase__Decode(iPacket);
				}
			
				// Dragon inventory
				for (unsigned int i = 0; i < 9; i++)
				{
					unsigned short initial_slot_id = packet.read2();
				
					if (initial_slot_id != 0)
					{
						for (unsigned int j = initial_slot_id; (j != 0); j = packet.read2())
						{						
							GW_ItemSlotBase__Decode(iPacket);
						}
					}
				}
			
				// Mechanic inventory
				for (unsigned int i = 0; i < 2; i++)
				{
					unsigned short initial_slot_id = packet.read2();
				
					if (initial_slot_id != 0)
					{
						for (unsigned int j = initial_slot_id; (j != 0); j = packet.read2())
						{						
							GW_ItemSlotBase__Decode(iPacket);
						}
					}
				}
			}
		
			// Normal inventories (Use, Etc, Setup, Cash)
			for (unsigned int i = 0, flag = 1; i < 4; i++, flag *= 2)
			{
				if (dbcharFlag_1 & (i * 8))
				{
					unsigned short initial_slot_id = packet.read1();
				
					if (initial_slot_id != 0)
					{
						for (unsigned int j = initial_slot_id; (j != 0); j = packet.read1())
						{						
							GW_ItemSlotBase__Decode(iPacket);
						}
					}
				}
			}
		
			for (unsigned int i = 0, flag = 2; i < 2; i++, flag *= 2)
			{
				if (dbcharFlag & (i * 8))
				{
					unsigned int loop_count = packet.read4();
				
					if (loop_count > 0)
					{
						for (unsigned int j = loop_count; j != 0; j--)
						{						
							packet.read4();
						}
					}
				}
			}
		
			if (dbcharFlag & 0x4000000)
			{
				unsigned int loop_count = packet.read4();
			
				if (loop_count > 0)
				{
					for (unsigned int i = 0; i < loop_count; i++)
					{
						packet.read4();
						packet.readBuffer(8);
					}
				}
			}
		
			if (dbcharFlag & 0x01)
			{
				unsigned int loop_count = packet.read4();
			
				if (loop_count > 0)
				{
					for (unsigned int i = 0; i < loop_count; i++)
					{
						packet.readBuffer(8);
						packet.readBuffer(8);
					}
				}
			}
		
			if (dbcharFlag & 0x2000000)
			{
				unsigned char initial_value = packet.read1();
			
				if (initial_value != 0)
				{
					for (unsigned int j = initial_value; (j != 0); j = packet.read1())
					{						
						packet.read4();
						packet.read1();
						packet.read1();
						packet.read4();
						packet.read4();
						packet.read4();
						packet.read4();
						packet.read1();
						packet.read4();
					
						packet.readBuffer(8);
						packet.readBuffer(8);
						packet.readBuffer(8);
						packet.readBuffer(8);
					}
				}
			}
		
			if (dbcharFlag & 0x100)
			{
				if (packet.read1())
				{
					unsigned short skill_count = packet.read2();
				
					for (unsigned int i = 0; i < skill_count; i++)
					{
						unsigned int skill_id = packet.read4();
						unsigned int skill_level = packet.read4();
						void* ftEnd = packet.readBuffer(8);
					
						if (true) // is_skill_need_master_level(skill_id)
						{
							unsigned int skill_master_level = packet.read4();
						}
					}
				
					unsigned short loop_count = packet.read2();
				
					if (loop_count > 0)
					{
						for (unsigned int i = 0; i < loop_count; i++)
						{
							packet.read4();
							packet.read2();
						}
					}
				}
				else
				{

				}
			}*/
		}
	} character_data;
}