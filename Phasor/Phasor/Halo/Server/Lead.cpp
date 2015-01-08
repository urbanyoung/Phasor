#include "Lead.h"
#include "../Game/Objects.h"
#include "../Game/Game.h"
#include "../../Globals.h"

namespace halo {
namespace server {
namespace lead 
{

    void __stdcall OnProjectileMove(halo::ident obj)
    {
        char hex[9];
        sprintf_s(hex, sizeof(hex), "%08X", obj);
        *g_PrintStream << "Projectile move: " << hex << endl;

        objects::s_halo_object* proj = (objects::s_halo_object*)objects::GetObjectAddress(obj);
        if (!proj) return;
        
        sprintf_s(hex, sizeof(hex), "%08X", proj->ownerPlayer);
        *g_PrintStream << "Owner: " << hex << endl;

        s_player* player = game::getPlayer(proj->ownerPlayer.slot);
        if (!player) return;


    }

    void __stdcall OnProjectileMoveRet()
    {

    }

    void __stdcall OnRayCast(DWORD flags, vect3d* pos, vect3d* dir, halo::ident obj,
                             halo::objects::s_intersection_output* output)
    {

    }

    void __stdcall OnRayCastRet()
    {

    }

} //lead
} //server
} //halo