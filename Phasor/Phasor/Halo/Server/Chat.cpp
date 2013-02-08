#include "Chat.h"
#include "../Player.h"
#include "../Game/Game.h"
#include "Packet.h"
#include <assert.h>

namespace halo { namespace server { namespace chat {

#pragma pack(push, 1)
	struct s_chat_data
	{
		DWORD type;
		DWORD player;
		const wchar_t* msg;
	};

#pragma pack(pop)

	void DispatchChat(e_chat_types type, const std::wstring& msg, 
		const s_player* from_const, const s_player* to_const)
	{
		/*! \todo
		 * Remove non-const casting after get_object has been checked.
		 */
		s_player* from = (s_player*)from_const;
		s_player* to = (s_player*)to_const;

		s_chat_data chat_data;
		chat_data.type = (DWORD)type;
		chat_data.msg = msg.c_str();

		switch (type)
		{
		case kChatServer:
			{
				chat_data.player = -1;
			} break;
		case kChatAll:
		case kChatTeam:
			{
				assert(from != NULL);
				chat_data.player = from->memory_id;
			} break;
		default:
			{
				assert(0);
			} break;
		}
		
		// Build the packet
		BYTE buffer[8192];		
		DWORD d_ptr = (DWORD)&chat_data; // Gotta pass a pointer to the struct
		DWORD retval = BuildPacket(buffer, 0, 0x0F, 0, (LPBYTE)&d_ptr, 0, 1, 0);
		
		// Add the packet to the appropriate queue
#define PACKET_QUEUE_PARAMS buffer, retval, 1, 1, 0, 1, 3

		if (to) // send to specific player
			AddPacketToPlayerQueue(to->mem->playerNum, PACKET_QUEUE_PARAMS);
		else { // apply normal processing if a dest player isn't specified
			switch (type)
			{
			case kChatServer:
			case kChatAll:
				{
					AddPacketToGlobalQueue(PACKET_QUEUE_PARAMS);
				} break;
			case kChatTeam:
				{
					for (int i = 0; i < 16; i++) {
						s_player* player = game::GetPlayer(i);
						if (player && player->mem->team) 
							AddPacketToPlayerQueue(player->mem->playerNum, PACKET_QUEUE_PARAMS);
					}
				} break;
			case kChatVehicle:
				{
					// Check if the sender is in a vehicle
					halo::objects::s_halo_object* from_obj = from->get_object();
					if (from_obj && from_obj->base.vehicleId != -1) {
						// send to players in this vehicle
						for (int i = 0; i < 16; i++) {
							s_player* player = game::GetPlayer(i);
							if (!player) continue;
							halo::objects::s_halo_object* obj = player->get_object();
							if (obj && obj->base.vehicleId == from_obj->base.vehicleId)
								AddPacketToPlayerQueue(player->mem->playerNum, PACKET_QUEUE_PARAMS);		
						}
					}
				} break;
			}
		}
	}
}}}