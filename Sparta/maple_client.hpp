#pragma once

#include "generic.hpp"

#include "account.hpp"
#include "session.hpp"

#include "stage_base.hpp"

namespace maplestory
{
	enum class server_type
	{
        login,
		game,
	};

	class maple_client
	{
	public:
		maple_client(maplestory::account& account);
		~maple_client();

		bool reset();

		bool migrate(server_type type, std::string const& ip, unsigned short port);
	
		bool notify_close();
		bool notify_connect();
		bool notify_read();
		bool notify_write();

		SOCKET get_desc() const;

		maplestory::account& account();
		maplestory::session& session();

	private:
		maplestory::account account_;
		maplestory::session session_;
		
		std::unique_ptr<stage::stage_base> current_stage;
	};
}