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
		kChatServer, // phasor only
		kChatPrivate, // phasor only
	};

#pragma pack(push, 1)
	struct s_chat_data
	{
		e_chat_types type;
		unsigned long player;
		const wchar_t* msg;
	};

#pragma pack(pop)

	void DispatchChat(e_chat_types type, const wchar_t* msg, 
		const s_player* from=NULL, const s_player* to=NULL);
}}}

