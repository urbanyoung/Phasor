#include "Player.h"
#include "../Globals.h"
#include "AFKDetection.h"
#include "../Admin.h"
#include "../../Common/MyString.h"
#include "Addresses.h"
#include "Server/Server.h"
#include <assert.h>

namespace halo 
{
	s_player::s_player(int memory_id) : memory_id(memory_id)
	{
		g_PrintStream << "New player " << memory_id << endl;
		mem = GetPlayerMemory(memory_id);
		afk = new CAFKDetection(*this, g_Timers);
		stream = new CPlayerStream(*this);	
		server::GetPlayerIP(*this, &ip, &port);
		server::GetPlayerHash(*this, hash);
	}
	s_player::~s_player()
	{
		g_PrintStream << "Player " << memory_id << " left" << endl;
		delete afk;
		delete stream;
	}

	objects::s_halo_object* s_player::get_object()
	{
		objects::s_halo_object* object = objects::GetObjectAddress(mem->object_id);
		assert(object == m_object);
		m_object = object;
		return object;
	}

	// todo: store a bool indicating if admin
	bool s_player::IsAdmin()
	{
		return Admin::IsAdmin(hash);
	}

	void s_player::Message(const wchar_t* fmt, ...)
	{
		va_list ArgList;
		va_start(ArgList, fmt);
		std::wstring str = FormatVarArgsW(fmt, ArgList);
		va_end(ArgList);

		g_PrintStream << "todo: make this message player - " << str << endl;
	}

	void s_player::Kick()
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

	// -------------------------------------------------------------------
	bool CPlayerStream::Write(const std::wstring& str)
	{
		server::MessagePlayer(player, str);
		return true;
	}
}