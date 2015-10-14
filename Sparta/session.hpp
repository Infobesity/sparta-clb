#pragma once

#include "generic.hpp"

namespace maplestory
{
	class session
	{
	public:
		session()
			: session_id_(0), account_id_(0), character_id_(0), character_name_("")
		{
		
		}
		
		session(unsigned __int64 session_id, unsigned int account_id, unsigned int character_id, std::string const& character_name)
			: session_id_(session_id), character_id_(character_id), account_id_(account_id), character_name_(character_name)
		{
		
		}

		~session()
		{
		
		}
		
		unsigned __int64& session_id()
		{
			return this->session_id_;
		}
		
		unsigned int& account_id()
		{
			return this->account_id_;
		}
		
		unsigned int& character_id()
		{
			return this->character_id_;
		}

		std::string& character_name()
		{
			return this->character_name_;
		}
		
	private:
		unsigned __int64 session_id_;
		unsigned int account_id_;

		unsigned int character_id_;
		std::string character_name_;
	};
}