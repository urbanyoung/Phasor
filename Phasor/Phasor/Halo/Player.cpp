#include "Player.h"
#include "../Globals.h"
#include "AFKDetection.h"
#include "../Admin.h"
#include "../../Common/MyString.h"
#include "Addresses.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include "../../ScriptingEvents.h"
#include <assert.h>

namespace halo 
{
	struct s_player_table
	{
		s_table_header header;
		s_player_structure players[16];

	};
	s_player::s_player(int memory_id) : memory_id(memory_id)
	{
		*g_PrintStream << "New player " << memory_id << endl;
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
		*g_PrintStream << "Player " << memory_id << " left" << endl;
	}

	/*! \todo 
	 * check if object can invalidate itself.. ie does loopobjects ever defrag? */
	objects::s_halo_biped* s_player::get_object()
	{
		objects::s_halo_biped* object = (objects::s_halo_biped*)objects::GetObjectAddress(mem->object_id);
		m_object = object;
		return object;
	}

	void s_player::Kick() const
	{
		*g_PrintStream << "todo: kick player" << endl;
	}

	void s_player::ChangeTeam(BYTE new_team, bool forcekill) const
	{
		using namespace server;
		s_server_info* server_info = GetServerStruct();

		for (int i = 0; i < 16; i++) {
			s_presence_item* player_entry = &server_info->player_data[i];
			if (player_entry->playerId == memory_id) {
				BYTE old_team = mem->team;

				// update teams in memory
				mem->team = new_team;
				mem->team_Again = new_team;
				player_entry->team = new_team;

				if (forcekill) Kill();
				NotifyServerOfTeamChange(*this);
				scripting::events::OnTeamChange(*this, false, old_team);
				break;
			}
		}

	}

	/*!
	 * \todo set sv_killed */
	void s_player::Kill() const
	{
		//sv_killed = true; // used later for detecting what killed the player
	
		if (mem->object_id.valid()) {
			DWORD playerMask = (mem->playerJoinCount << 0x10) | memory_id;
			DWORD playerObj = mem->object_id;
			__asm
			{
				pushad

				PUSH 0
				PUSH -1
				PUSH -1
				PUSH -1
				MOV EAX,playerObj
				call DWORD PTR ds:[FUNC_ONPLAYERDEATH]
				add esp, 0x10
				mov eax, playerObj
				call DWORD PTR ds:[FUNC_ACTIONDEATH_1]
				mov eax, playerMask
				call DWORD PTR ds:[FUNC_ACTIONDEATH_2]
				push playerMask
				call DWORD PTR ds:[FUNC_ACTIONDEATH_3]
				add esp, 4

				popad
			}
		}

		//sv_killed = false;
	}

	void s_player::ApplyCamo(float duration) const
	{
		DWORD playerMask = (mem->playerJoinCount << 0x10) | memory_id;
		DWORD count = 0x8000; // count >= 0x8000 == infinite

		if (duration != 0)
			count = (DWORD)(duration * 30); // 30 ticks per second

		// Make the player invisible
		__asm
		{
			mov eax, memory_id
			mov ebx, playerMask
			push count
			push 0
			call dword ptr ds:[FUNC_DOINVIS]
			add esp, 8
		}
	}

	void s_player::SetSpeed(float speed) const
	{
		mem->speed = speed;
	}

	s_player_structure* GetPlayerMemory(int index)
	{
		if (index >= 0 && index < 16) {
			s_player_table* table = *(s_player_table**)ADDR_PLAYERBASE;
			if (table) return &table->players[index];
		}
		return 0;
	}	
}