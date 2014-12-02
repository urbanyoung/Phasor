#pragma once

#include "../../../Common/Types.h"

namespace halo { namespace server{

	DWORD BuildPacket(LPBYTE output, DWORD arg1, DWORD packettype, DWORD arg3, LPBYTE dataPtr, DWORD arg4, DWORD arg5, DWORD arg6);
	
	// This function adds a packet to the queue
	void AddPacketToGlobalQueue(LPBYTE packet, DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5);

	// This function adds a packet to a specific player's queue
	void AddPacketToPlayerQueue(DWORD player, LPBYTE packet, DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5);
}}

