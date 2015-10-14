#include "tcp_socket.hpp"
#include "packet_crypto.hpp"

#include "sparta_form.hpp"

namespace maplestory
{
	namespace network
	{
		tcp_socket::tcp_socket()
			: sock(INVALID_SOCKET), recv_offset(0)
		{
			memset(this->recv_buffer, 0, sizeof(this->recv_buffer));
		}

		tcp_socket::~tcp_socket()
		{
			if (this->sock != INVALID_SOCKET)
			{
				this->try_disconnect();
			}
		}

		bool tcp_socket::try_connect(std::string const& ip, unsigned short port)
		{
			this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if (this->sock == INVALID_SOCKET)
			{
				return false;
			}

			if (WSAAsyncSelect(this->sock, sparta::gui::sparta_form::get_instance().get_handle(), 
				WM_SOCKET, FD_CLOSE | FD_CONNECT | FD_READ | FD_WRITE))
			{
				return false;
			}

			sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
			addr.sin_port = htons(port);

			printf("connecting to %s:%d\n", ip.c_str(), port);

			return ((connect(this->sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != SOCKET_ERROR) || (WSAGetLastError() == WSAEWOULDBLOCK));
		}

		bool tcp_socket::try_disconnect()
		{
			if (this->sock == INVALID_SOCKET)
			{
				return false;
			}
			
			if (shutdown(this->sock, SD_BOTH) == SOCKET_ERROR)
			{
				return false;
			}
			
			if (closesocket(this->sock) == SOCKET_ERROR)
			{
				return false;
			}
			
			return true;
		}
	
		bool tcp_socket::recv_handshake(io_packet& packet)
		{
			unsigned short handshake_length = 0;

			if (!this->raw_recv(reinterpret_cast<unsigned char*>(&handshake_length), sizeof(unsigned short)))
			{
				return false;
			}
			
			unsigned char* handshake_buffer = new unsigned char[handshake_length];
			memset(handshake_buffer, 0, handshake_length);

			if (!this->raw_recv(handshake_buffer, handshake_length))
			{
				delete[] handshake_buffer;
				return false;
			}
			
			packet = network::io_packet(handshake_buffer, handshake_length);

			delete[] handshake_buffer;
			return true;
		}
		
		bool tcp_socket::recv_packets(std::vector<io_packet>& packets, unsigned int& iv)
		{
			for (unsigned int recv_size = 0xDEADBEEF; recv_size != 0 && recv_size != SOCKET_ERROR; 
				this->recv_offset += (recv_size == SOCKET_ERROR ? 0 : recv_size))
			{ 
				recv_size = recv(this->sock, reinterpret_cast<char*>(this->recv_buffer + this->recv_offset), 
					sizeof(this->recv_buffer) - this->recv_offset, 0);
				
				if (recv_size == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
				{
					printf("[stage_base::try_read] SOCKET_ERROR: %d\n", WSAGetLastError());
					return false;
				}
			}
			
			while (this->recv_offset >= 4)
			{
				unsigned int packet_length = network::crypto::packet_crypto::get_instance().get_packet_length(this->recv_buffer);
				unsigned int packet_length_total = 4 + packet_length;

				if (this->recv_offset < packet_length_total)
				{
					break;
				}

				network::crypto::packet_crypto::get_instance().aes_crypt(this->recv_buffer + 4, packet_length, iv);
				packets.push_back(network::io_packet(this->recv_buffer + 4, packet_length));
				
				this->recv_offset -= packet_length_total;
				memcpy(this->recv_buffer, this->recv_buffer + packet_length_total, this->recv_offset);
				memset(this->recv_buffer + this->recv_offset, 0, packet_length_total);
			}

			return true;
		}

		bool tcp_socket::send_packet(io_packet& packet, unsigned short version_major, unsigned int& iv)
		{
			std::size_t packet_size = packet.get_size() + sizeof(unsigned int);
			unsigned char* packet_buffer = new unsigned char[packet_size];

			memset(packet_buffer, 0, packet_size);
			memcpy(packet_buffer + sizeof(unsigned int), packet.get_data(), packet.get_size());

			crypto::packet_crypto::get_instance().create_header(packet_buffer, packet.get_size(), version_major, iv);
			crypto::packet_crypto::get_instance().aes_crypt(packet_buffer + sizeof(unsigned int), packet.get_size(), iv);

			bool result = this->raw_send(packet_buffer, packet_size);

			delete[] packet_buffer;
			return result;
		}
		
		SOCKET tcp_socket::get_desc() const
		{
			return this->sock;
		}

		bool tcp_socket::raw_recv(unsigned char* buffer, std::size_t size)
		{
			for (int data_read = 0, offset = 0, data_to_read = size; data_to_read > 0; data_to_read -= data_read, offset += data_read)
			{
				data_read = recv(this->sock, reinterpret_cast<char*>(buffer) + offset, data_to_read, 0);
	
				if (data_read == 0 || data_read == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("recv() returned %d -> WSAGetLastError() = %d\n", data_read, WSAGetLastError());
					}

					return false;
				}
			}

			return true;
		}

		bool tcp_socket::raw_send(unsigned char const* buffer, std::size_t size)
		{
			for (int data_sent = 0, offset = 0, data_to_send = size; data_to_send > 0; data_to_send -= data_sent, offset += data_sent)
			{
				data_sent = send(this->sock, reinterpret_cast<const char*>(buffer) + offset, data_to_send, 0);
	
				if (data_sent == 0 || data_sent == SOCKET_ERROR)
				{
					printf("send() returned %d -> WSAGetLastError() = %d\n", data_sent, WSAGetLastError());
					return false;
				}
			}

			return true;
		}
	}
}