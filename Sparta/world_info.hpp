#pragma once

#include "generic.hpp"
#include "channel_info.hpp"

namespace maplestory
{
	/* CLogin::WORLDITEM */
	class world_info
	{
	public:
		world_info() : id_(0), name_(""), state_(0), event_description_(""), event_exp_rate_(0), event_drop_rate_(0), channel_count_(0)
		{
			this->channels_.clear();
		}
		
		unsigned char& id()
		{
			return this->id_;
		}
		
		std::string& name()
		{
			return this->name_;
		}
		
		unsigned char& state()
		{
			return this->state_;
		}

		std::string& event_description()
		{
			return this->event_description_;
		}

		unsigned short& event_exp_rate()
		{
			return this->event_exp_rate_;
		}

		unsigned short& event_drop_rate()
		{
			return this->event_drop_rate_;
		}

		unsigned char& channel_count()
		{
			return this->channel_count_;
		}

		std::vector<channel_info>& channels()
		{
			return this->channels_;
		}

	private:
		unsigned char id_;
		std::string name_;
		unsigned char state_;
		std::string event_description_;
		unsigned short event_exp_rate_;
		unsigned short event_drop_rate_;
		unsigned char channel_count_;
		std::vector<channel_info> channels_;
	};
}