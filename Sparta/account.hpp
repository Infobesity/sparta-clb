#pragma once

#include "generic.hpp"

namespace maplestory
{
	class account
	{
	public:
		account()
			: username_(""), password_(""), pic_(""), world_id_(0), channel_id_(0), character_id_(0)
		{
		
		}

		account(std::string const& username, std::string const& password, std::string const& pic, unsigned int world_id, unsigned int channel_id, unsigned int character_id)
			: username_(username), password_(password), pic_(pic), world_id_(world_id), channel_id_(channel_id), character_id_(character_id)
		{
		
		}

		~account()
		{
		
		}
		
		std::string& username()
		{
			return this->username_;
		}
		
		std::string& password()
		{
			return this->password_;
		}

		std::string& pic()
		{
			return this->pic_;
		}
		
		unsigned int& world_id()
		{
			return this->world_id_;
		}

		unsigned int& channel_id()
		{
			return this->channel_id_;
		}

		unsigned int& character_id()
		{
			return this->character_id_;
		}

	private:
		std::string username_;
		std::string password_;
		std::string pic_;

		unsigned int world_id_;
		unsigned int channel_id_;
		unsigned int character_id_;
	};
}