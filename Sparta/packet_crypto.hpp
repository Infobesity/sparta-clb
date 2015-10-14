#pragma once

#include "generic.hpp"
#include "rijndael.hpp"

namespace maplestory
{
	namespace network
	{
		namespace crypto
		{
			class packet_crypto
			{
			private:
				static const unsigned char shuffle[256];

			public:
				static packet_crypto& get_instance()
				{
					static packet_crypto crypto_client;
					return crypto_client;
				}

				void aes_crypt(unsigned char* data, unsigned int length, unsigned int& iv);

				void encode_data(unsigned char* data, unsigned int length);
				void decode_data(unsigned char* data, unsigned int length);

				void create_header(unsigned char* data, unsigned int length, unsigned short version_major, unsigned int iv);
				unsigned int get_packet_length(unsigned char* data);

			private:
				packet_crypto();
				~packet_crypto();

				unsigned int shuffle_iv(unsigned int iv);

				crypto::rijndael aes_crypto;
			};

		}
	}
}