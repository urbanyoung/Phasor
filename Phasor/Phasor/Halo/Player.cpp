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
		stream.reset(new CPlayerStream(*this));	
		server::GetPlayerIP(*this, &ip, &port);
		server::GetPlayerHash(*this, hash);
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

	// todo: store a bool indicating if admin
	bool s_player::IsAdmin() const
	{
		return Admin::IsAdmin(hash);
	}

	void s_player::Message(const wchar_t* fmt, ...) const
	{
		va_list ArgList;
		va_start(ArgList, fmt);
		std::wstring str = FormatVarArgsW(fmt, ArgList);
		va_end(ArgList);

		server::MessagePlayer(*this, str);
	}

	void s_player::ConsoleMessage(const std::wstring& str) const
	{
		g_PrintStream << "todo: console message player" << endl << str << endl;
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