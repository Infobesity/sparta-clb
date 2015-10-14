#pragma once

#include "generic.hpp"
#include "io_packet.hpp"

#include "tcp_socket.hpp"

namespace maplestory
{
	namespace stage
	{
		class stage_base
		{
		public:
			stage_base(bool monitor);
			virtual ~stage_base();

			bool try_read();
			bool connect(std::string const& ip, unsigned short port);

			SOCKET get_desc() const;
			
			bool send_packet(network::io_packet& packet);

		protected:
			bool recv_packets(std::vector<network::io_packet>& packets);
			
			virtual bool on_enter_stage() = 0;

			virtual bool handle_handshake(unsigned char locale, unsigned short version_major, unsigned short version_minor);
			virtual bool handle_packet(unsigned short header, network::io_packet& packet) = 0;
			
		private:
			bool forward_handshake();
			bool forward_packet(network::io_packet& packet);
			
			bool on_alive_req(network::io_packet& packet);
			bool on_security_packet(network::io_packet& packet);

		private:
			std::unique_ptr<network::tcp_socket> tcp_socket;
			
			bool monitor;
			bool handshake;

			unsigned char locale;
			unsigned short version_major;
			unsigned short version_minor;
			unsigned int send_iv;
			unsigned int recv_iv;
		};
	}
}