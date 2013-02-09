#include "Player.h"
#include "../Globals.h"
#include "AFKDetection.h"
#include "../Admin.h"
#include "../../Common/MyString.h"
#include "Addresses.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include <assert.h>

namespace halo 
{
	s_player::s_player(int memory_id) : memory_id(memory_id)
	{
		g_PrintStream << "New player " << memory_id << endl;
		mem = GetPlayerMemory(memory_id);
		afk.reset(new afk_detection::CAFKDetection(*this, g_Timers));
		console_stream.reset(new PlayerConsoleStream(*this));	
		chat_stream.reset(new PlayerChatStream(*this));
		server::GetPlayerIP(*this, &ip, &port);
		server::GetPlayerHash(*this, hash);
		is_admin = Admin::IsAdmin(hash);
		m_object = 0;
	}

	s_player::~s_player()
	{
		g_PrintStream << "Player " << memory_id << " left" << endl;
	}

	objects::s_halo_object* s_player::get_object()
	{
		objects::s_halo_object* object = objects::GetObjectAddress(mem->object_id);
		assert(m_object == 0 || object == m_object);
		m_object = object;
		return object;
	}

	void s_player::Kick() const
	{
		g_PrintStream << "todo: kick player" << endl;
	}

	s_player_structure* GetPlayerMemory(int index)
	{
		s_player_structure* mem = 0;
		if (index >= 0 && index < 16) {
			LPBYTE lpPlayerList = (LPBYTE)*(DWORD*)ULongToPtr(ADDR_PLAYERBASE);

			if (lpPlayerList)
				mem = (s_player_structure*)(lpPlayerList + 0x38 + (0x200 * index));
		}

		return mem;
	}	
}