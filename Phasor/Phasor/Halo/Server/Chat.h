#pragma once

#include <string>

namespace halo { 
	struct s_player;
	namespace server { namespace chat {

	enum e_chat_types
	{
		// important: don't reorder!
		kChatAll = 0,
		kChatTeam,
		kChatVehicle,
		kChatServer
	};

	void DispatchChat(e_chat_types type, const std::wstring& msg, 
		const s_player* from=NULL, const s_player* to=NULL);
}}}

