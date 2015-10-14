#include "maple_client.hpp"

#include "config.hpp"
#include "io_packet.hpp"

#include "game_stage.hpp"
#include "login_stage.hpp"

namespace maplestory
{
	maple_client::maple_client(maplestory::account& account)
		: account_(account)
	{
		this->current_stage.reset();
	}

	maple_client::~maple_client()
	{
		this->current_stage.reset();
	}
	
	bool maple_client::reset()
	{
		return this->migrate(server_type::login, config::server::ip, config::server::port);
	}

	bool maple_client::migrate(server_type type, std::string const& ip, unsigned short port)
	{
		this->current_stage.reset();
			
		switch (type)
		{
		case server_type::game:
			this->current_stage = std::make_unique<stage::game_stage>(this, this->account(), this->session());
			break;

		case server_type::login:
			this->current_stage = std::make_unique<stage::login_stage>(this, this->account());
			break;

		default:
			printf("Unknown stage requested!\n");
			break;
		}
			
		return (this->current_stage.get() && this->current_stage.get()->connect(ip, port));
	}
	
	bool maple_client::notify_close()
	{
		//printf("[maple_client] %s notify_close\n", this->account().username().c_str());
		return this->reset();
	}

	bool maple_client::notify_connect()
	{
		//printf("[maple_client] %s notify_connect\n", this->account().username().c_str());
		return true;
	}
	
	bool maple_client::notify_read()
	{
		//printf("[maple_client] %s notify_read\n", this->account().username().c_str());

		if (!this->current_stage.get()->try_read())
		{
			return this->reset();
		}

		return true;
	}

	bool maple_client::notify_write()
	{
		//printf("[maple_client] %s notify_write\n", this->account().username().c_str());
		return true;
	}

	SOCKET maple_client::get_desc() const
	{
		return (this->current_stage.get() ? this->current_stage.get()->get_desc() : INVALID_SOCKET);
	}

	account& maple_client::account()
	{
		return this->account_;
	}

	session& maple_client::session()
	{
		return this->session_;
	}
}