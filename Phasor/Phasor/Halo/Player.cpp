#include "Player.h"
#include "../Globals.h"
#include "AFKDetection.h"
#include "../Admin.h"
#include "../../Common/MyString.h"
#include "Addresses.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include "../../Scripts/script-events.h"
#include <assert.h>

namespace halo 
{
	struct s_player_table
	{
		s_table_header header;
		s_player_structure players[16];
	};

	s_player::s_player(int memory_id) : memory_id(memory_id), sv_killed(false),
        force_entered(false)
	{
		*g_PrintStream << "New player " << memory_id << endl;

		mem = GetPlayerMemory(memory_id);
		afk.reset(new afk_detection::CAFKDetection(*this, g_Timers));
		console_stream.reset(new PlayerConsoleStream(*this));	
		chat_stream.reset(new PlayerChatStream(*this));

		server::PhasorMachine* machine = server::FindMachine(*this);
		// machine will never be null
		server::GetMachineIP(*machine->machine, &ip, &port);
		server::GetMachineHash(*machine->machine, hash);
		
		if (Admin::isChallengeEnabled()) {
			if (machine->hash_validated) checkAndSetAdmin();
			else { // wait for response from gamespy
				authenticating_hash = true;
				is_admin = false;
			}
		} else checkAndSetAdmin();
	}

	s_player::~s_player()
	{
		*g_PrintStream << "Player " << memory_id << " left" << endl;
	}

	void s_player::checkAndSetAdmin() {
		is_admin = Admin::isAdmin(hash);
		authenticating_hash = false;
	}

	objects::s_halo_biped* s_player::get_object() const
	{
		return (objects::s_halo_biped*)objects::GetObjectAddress(mem->object_id);
	}

	void s_player::Kick() const
	{
		std::string cmd = m_sprintf("sv_kick %i", mem->playerNum + 1);
		server::ExecuteServerCommand(cmd, NULL);
	}

	void s_player::ChangeTeam(BYTE new_team, bool forcekill)
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
				
				scripting::events::OnTeamChange(*this, false, old_team, new_team);
				break;
			}
		}

	}

	void s_player::Kill()
	{
		sv_killed = true; // used later for detecting what killed the player
	
		if (mem->object_id.valid()) {
			DWORD playerMask = getPlayerIdent();
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

		sv_killed = false;
	}

	void s_player::ApplyCamo(float duration) const
	{
		DWORD playerMask = getPlayerIdent();
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

	bool s_player::InVehicle() const
	{
		auto biped = get_object();
		if (!biped) return false;
		return biped->base.vehicleId.valid();
	}

	ident s_player::getPlayerIdent() const
	{
		return make_ident((mem->playerJoinCount << 0x10) | memory_id);
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