#include "Player.h"
#include "../Globals.h"
#include "AFKDetection.h"
#include "../Admin.h"
#include "../../Common/MyString.h"
#include <assert.h>

namespace halo 
{
	s_player::s_player(int memory_id) : memory_id(memory_id)
	{
		afk = new CAFKDetection(*this, g_Timers);
	}
	s_player::~s_player()
	{
		delete afk;
	}

	objects::s_halo_object* s_player::get_object()
	{
		objects::s_halo_object* object = objects::GetObjectAddress(mem->object_id);
		assert(object == m_object);
		m_object = object;
		return object;
	}

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
}