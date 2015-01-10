#include "../Game/Objects.h"
#include "../../../Common/vect3d.h"
#include "../../../Common/Types.h"

namespace halo { namespace server {
namespace lead 
{
    void __stdcall OnTick();

    void __stdcall OnProjectileMove(halo::ident obj);
    void __stdcall OnProjectileMoveRet();

    void __stdcall OnRayCast(DWORD flags, vect3d* pos, vect3d* dir, halo::ident obj, 
                             halo::objects::s_intersection_output* output);
    void __stdcall OnRayCastRet();

    void OnObjectDestroy(ident objid);
    void OnObjectCreation(ident objid);

    void OnGameEnd();
    void OnGameStart();
}}}