#include "system_info.hpp"

#include <Iphlpapi.h>

#pragma comment(lib, "iphlpapi")
#pragma comment(lib, "netapi32")

namespace system_info
{
	unsigned int get_hdd_serial_number()
	{
		char main_folder[MAX_PATH];

		if (!GetWindowsDirectory(main_folder, MAX_PATH))
		{
			return 0;
		}

		main_folder[3] = 0;

		char volume_name_buffer[MAX_PATH + 1];
		memset(volume_name_buffer, 0, sizeof(volume_name_buffer));

		char file_system_name_buffer[MAX_PATH + 1];
		memset(file_system_name_buffer, 0, sizeof(file_system_name_buffer));

		unsigned long volume_serial_number = 0;
		unsigned long maximum_component_length = 0;
		unsigned long file_system_flags = 0;

		if (!GetVolumeInformation(main_folder, volume_name_buffer, MAX_PATH + 1, &volume_serial_number, 
			&maximum_component_length, &file_system_flags, file_system_name_buffer, MAX_PATH + 1))
		{
			return 0;
		}
	
		return volume_serial_number;
	}
	
	std::string get_local_mac_address()
	{
		typedef struct _ASTAT_
		{
			ADAPTER_STATUS adapter_status;
			NAME_BUFFER name_buffer[30];
		} ASTAT;

		char mac_address[256];
		memset(mac_address, 0, sizeof(mac_address));

		LANA_ENUM lenum;
		memset(&lenum, 0, sizeof(LANA_ENUM));

		NCB ncb;
		memset(&ncb, 0, sizeof(NCB));

		ncb.ncb_command = NCBENUM;
		ncb.ncb_buffer = reinterpret_cast<unsigned char*>(&lenum);
		ncb.ncb_length = sizeof(LANA_ENUM);

		if (Netbios(&ncb) != NRC_GOODRET)
		{
			return std::string("");
		}

		for (unsigned char i = 0; i < lenum.length; i++)
		{
			ASTAT adapter;
			memset(&adapter, 0, sizeof(ASTAT));

			/* reset the LAN adapter */
			memset(&ncb, 0, sizeof(NCB));

			ncb.ncb_command = NCBRESET;
			ncb.ncb_lana_num = lenum.lana[i];
			
			if (Netbios(&ncb) != NRC_GOODRET)
			{
				return std::string("");
			}
			
			/* retrieve the adapter information */
			memset(&ncb, 0, sizeof(NCB));
			ncb.ncb_command = NCBASTAT;
			ncb.ncb_lana_num = lenum.lana[i];
			
			strcpy(reinterpret_cast<char*>(ncb.ncb_callname), "*               ");

			ncb.ncb_buffer = reinterpret_cast<unsigned char*>(&adapter);
			ncb.ncb_length = sizeof(ASTAT);
			
			if (Netbios(&ncb) == NRC_GOODRET)
			{
				char temp_string[256];
				memset(temp_string, 0, sizeof(temp_string));

				sprintf_s(temp_string, "%02X-%02X-%02X-%02X-%02X-%02X", 
					adapter.adapter_status.adapter_address[0], adapter.adapter_status.adapter_address[1],	
					adapter.adapter_status.adapter_address[2], adapter.adapter_status.adapter_address[3],	
					adapter.adapter_status.adapter_address[4], adapter.adapter_status.adapter_address[5]);
				
				if (strlen(mac_address) > 0)
				{
					strcat_s(mac_address, ", ");
				}

				strcat_s(mac_address, temp_string);
			}
		}

		return std::string(mac_address);
	}

	std::string get_local_mac_serial()
	{
		IP_ADAPTER_INFO* adapter_info = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
		ULONG buffer_length = sizeof(IP_ADAPTER_INFO);
	
		if (GetAdaptersInfo(adapter_info, &buffer_length) != ERROR_SUCCESS)
		{
			free(adapter_info);
			adapter_info = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buffer_length));
		
			if (GetAdaptersInfo(adapter_info, &buffer_length) != ERROR_SUCCESS)
			{
				return std::string("");
			}
		}

		unsigned char original_info[6];
		memset(original_info, 0, sizeof(original_info));

		for (IP_ADAPTER_INFO* adapter = adapter_info; adapter != 0; adapter = adapter->Next)
		{
			if (adapter->Type == MIB_IF_TYPE_ETHERNET)
			{
				bool run = true;

				if (adapter->Address[0] == 'D' && adapter->Address[1] == 'E' && adapter->Address[2] == 'S' && 
					adapter->Address[3] == 'T' && adapter->Address[4] == 0 && adapter->Address[5] == 0)
				{
					run = false;
				}

				unsigned char* adapter_address = adapter->Address;
				unsigned char* original_infos = original_info;

				while(run)
				{
					if (adapter_address[0] <= original_infos[0])
					{
						if (adapter_address[0] < original_infos[0])
						{
							run = false;
						}

						adapter_address++;
						original_infos++;
					
						if (adapter_address - adapter->Address < 6)
						{
							continue;
						}
					}

					if (run)
					{
						original_info[0] = adapter->Address[0];
						original_info[1] = adapter->Address[1];
						original_info[2] = adapter->Address[2];
						original_info[3] = adapter->Address[3];
						original_info[4] = adapter->Address[4];
						original_info[5] = adapter->Address[5];
					}

					break;
				}
			}
		}
	
		free(adapter_info);
		
		char mac_address[256] = "";

		sprintf_s(mac_address, "%02X%02X%02X%02X%02X%02X", original_info[0], original_info[1],
			original_info[2], original_info[3], original_info[4], original_info[5]);

		return std::string(mac_address);
	}
	
	bool get_machine_id(unsigned char* machine_id)
	{
		memset(machine_id, 0, 16);

		typedef struct _ASTAT_
		{
			ADAPTER_STATUS adapter_status;
			NAME_BUFFER name_buffer[30];
		} ASTAT;

		LANA_ENUM lenum;
		memset(&lenum, 0, sizeof(LANA_ENUM));

		NCB ncb;
		memset(&ncb, 0, sizeof(NCB));

		ncb.ncb_command = NCBENUM;
		ncb.ncb_buffer = reinterpret_cast<unsigned char*>(&lenum);
		ncb.ncb_length = sizeof(LANA_ENUM);

		if (Netbios(&ncb) != NRC_GOODRET)
		{
			return false;
		}

		for (unsigned char i = 0; i < lenum.length; i++)
		{
			ASTAT adapter;
			memset(&adapter, 0, sizeof(ASTAT));

			/* reset the LAN adapter */
			memset(&ncb, 0, sizeof(NCB));

			ncb.ncb_command = NCBRESET;
			ncb.ncb_lana_num = lenum.lana[i];
			
			if (Netbios(&ncb) != NRC_GOODRET)
			{
				return false;
			}
			
			/* retrieve the adapter information */
			memset(&ncb, 0, sizeof(NCB));
			ncb.ncb_command = NCBASTAT;
			ncb.ncb_lana_num = lenum.lana[i];
			
			strcpy(reinterpret_cast<char*>(ncb.ncb_callname), "*               ");

			ncb.ncb_buffer = reinterpret_cast<unsigned char*>(&adapter);
			ncb.ncb_length = sizeof(ASTAT);
			
			if (Netbios(&ncb) == NRC_GOODRET)
			{
				/* 0xFF == Token Ring adapter | 0xFE == Ethernet adapter */
				if (adapter.adapter_status.adapter_type != 0xFF && adapter.adapter_status.adapter_type != 0xFE)
				{
					if (adapter.adapter_status.adapter_type)
					{
						*reinterpret_cast<unsigned char*>(machine_id) = adapter.adapter_status.adapter_type;
					}
				}
				else
				{
					//*reinterpret_cast<unsigned char*>(machine_id) = adapter.adapter_status.adapter_address[0];
					//*reinterpret_cast<unsigned char*>(machine_id + 1) = adapter.adapter_status.adapter_address[1];
					//*reinterpret_cast<unsigned char*>(machine_id + 2) = adapter.adapter_status.adapter_address[2];
					//*reinterpret_cast<unsigned char*>(machine_id + 3) = adapter.adapter_status.adapter_address[3];
					//*reinterpret_cast<unsigned char*>(machine_id + 4) = adapter.adapter_status.adapter_address[4];
					//*reinterpret_cast<unsigned char*>(machine_id + 5) = adapter.adapter_status.adapter_address[5];
				}

				break;
			}
		}
		
		*reinterpret_cast<unsigned int*>(machine_id + 6) = get_hdd_serial_number();

		unsigned char* machine_id_buffer = machine_id;
		unsigned short machine_id_checksum = 0;

		for (int i = 0, j = 0; i < 6; i++, machine_id_checksum += j, machine_id_buffer += 2, j = i + 3)
		{
			j *= *reinterpret_cast<unsigned short*>(machine_id_buffer);
		}

		*reinterpret_cast<unsigned short*>(machine_id + 14) = machine_id_checksum;
		return true;
	}

	std::string get_cookie_string()
	{
		srand(GetTickCount());

		char cookie_string[24];
		memset(cookie_string, 0, sizeof(cookie_string));

		for (int i = 0; i < (sizeof(cookie_string) - 1); i++)
		{
			char uppercase_rand = 'A' + static_cast<char>(rand() % ('Z' - 'A'));
			char lowercase_rand = 'a' + static_cast<char>(rand() % ('z' - 'a'));
			char numeric_rand = '0' + static_cast<char>(rand() % ('9' - '0'));

			switch (static_cast<unsigned char>(rand()) % 3)
			{
			case 0:
				cookie_string[i] = uppercase_rand;
				break;

			case 1:
				cookie_string[i] = lowercase_rand;
				break;

			case 2:
				cookie_string[i] = numeric_rand;
				break;

			default:
				cookie_string[i] = '?';
				break;
			}
		}

		return std::string(cookie_string);
	}
}