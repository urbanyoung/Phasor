#include "Packet.h"
#include "../Addresses.h"

namespace halo { namespace server {

	DWORD BuildPacket(LPBYTE output, DWORD arg1, DWORD packettype, DWORD arg3, LPBYTE dataPtr, DWORD arg4, DWORD arg5, DWORD arg6)
	{
		DWORD dwBuildPacket = FUNC_BUILDPACKET;
		DWORD retval = 0;

		__asm
		{
			pushad

				PUSH arg6
				PUSH arg5
				PUSH arg4
				PUSH dataPtr
				PUSH arg3
				PUSH packettype
				PUSH arg1
				MOV EDX,0x7FF8
				MOV EAX,output
				call dword ptr ds:[dwBuildPacket]
			mov retval, eax
				add esp, 0x1C

				popad
		}

		return retval;
	}

	// This function adds a packet to the queue
	void AddPacketToGlobalQueue(LPBYTE packet, DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5)
	{
		DWORD dwAddToQueue = FUNC_ADDPACKETTOQUEUE;

		__asm
		{
			pushad

				MOV ECX,DWORD PTR DS:[ADDR_SOCKETREADY]
			mov ECX, [ECX]
			cmp ecx, 0
				je NOT_READY_TO_SEND

				PUSH arg5
				PUSH arg4
				PUSH arg3
				PUSH arg2
				PUSH packet
				PUSH arg1
				MOV EAX,packetCode
				call dword ptr ds:[dwAddToQueue]
			add esp, 0x18

	NOT_READY_TO_SEND:

			popad
		}
	}

	// This function adds a packet to a specific player's queue
	void AddPacketToPlayerQueue(DWORD player, LPBYTE packet, DWORD packetCode, DWORD arg1, DWORD arg2, DWORD arg3, DWORD arg4, DWORD arg5)
	{
		DWORD dwAddToPlayerQueue = FUNC_ADDPACKETTOPQUEUE;

		__asm
		{
			pushad

				MOV ESI,DWORD PTR DS:[ADDR_SOCKETREADY]
			mov esi, [esi]

			cmp esi, 0
				je NOT_READY_TO_SEND

				PUSH arg5
				PUSH arg4
				PUSH arg3
				PUSH arg2
				PUSH packetCode
				PUSH packet
				PUSH arg1
				mov eax, player
				call dword ptr ds:[dwAddToPlayerQueue]
			add esp, 0x1C

	NOT_READY_TO_SEND:

			popad
		}
	}
}}