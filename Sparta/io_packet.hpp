#pragma once

#include "generic.hpp"

namespace maplestory
{
	namespace network
	{
		class io_packet
		{
		public:
			io_packet();
			io_packet(unsigned char* data, std::size_t size);
			io_packet(unsigned short header);
			virtual ~io_packet();
	
			void write1(unsigned char data);
			void write2(unsigned short data);
			void write4(unsigned int data);
			void write8(unsigned __int64 data);
			void write_buffer(unsigned char* data, std::size_t size);
			void write_string(std::string const& data);
			void write_zero(std::size_t length);

			unsigned char read1();
			unsigned short read2();
			unsigned int read4();
			unsigned __int64 read8();
			bool read_buffer(unsigned char* buffer, std::size_t length);
			std::string read_string();
	
			template<typename T>
			T read_string_as();

			bool indent(std::size_t length);
	
			unsigned char* get_data();
			unsigned short get_header();
			unsigned int get_size();
			unsigned int get_remaining_size();

		protected:
			template <typename T>
			void write(T data);
	
			template <typename T>
			T read();
	
			bool check_offset(std::size_t size);

		private:
			std::size_t offset;
			std::vector<unsigned char> data;
		};

		template <typename T>
		void io_packet::write(T data)
		{
			unsigned char* temp_data = reinterpret_cast<unsigned char*>(&data);
			std::copy(temp_data, temp_data + sizeof(T), std::back_inserter(this->data));
		}

		template <typename T>
		T io_packet::read()
		{
			if (this->check_offset(sizeof(T)))
			{
				this->offset += sizeof(T);
				return *reinterpret_cast<T*>(this->data.data() + this->offset - sizeof(T));
			}

			return static_cast<T>(0);
		}

		template<typename T>
		T io_packet::read_string_as()
		{
			unsigned short length = this->read<unsigned short>();

			if (this->check_offset(length))
			{
				std::string buffer(reinterpret_cast<char*>(this->data.data() + this->offset), length);
				this->offset += length;

				return static_cast<T>(atoi(buffer.c_str()));
			}

			return static_cast<T>(0);
		}
	}
}