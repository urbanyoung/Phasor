#include "Lead.h"
#include "../Game/Objects.h"
#include "../Game/Game.h"
#include "../../Globals.h"
#include <unordered_map>

namespace halo {
namespace server {
namespace lead 
{
    size_t bytesCopied = 0;
    template <class T, int N>
    class CircBuffer
    {
    private:
        T data[N];
        int index, size;

        inline bool full() {
            return size == N;
        }

        const T* get_first() {
            if (full())
                return data[index];
            else
                return data[0];
        }

        const T* get_last() {
            if (index == 0)
                return data[N-1];
            else
                return data[index-1];
        }

    public:
        CircBuffer()
            : index(0), size(0)
        {}

        void add(const T* x) { 
            bytesCopied += sizeof(T);
            memcpy(&data[index], x, sizeof(T));
            index = (index == N - 1) ? 0 : index + 1;
            if (!full()) ++size;
        }

        // Get data element starting from 0 as the most recent
        const T* get(int i) {
            if (size == 0)
                return nullptr; // no data yet
            else if (i >= size)
                return get_first(); // get least recent data
            else if (i <= 0)
                return get_last(); // get most recent data
            else {
                int abs_index = index - 1 - i;
                if (abs_index < 0) { // wrap to the end
                    abs_index = N + abs_index;

                    if (abs_index >= size)
                        return get_first();
                }

                return data[abs_index];
            }            
        }

        void clear() {
            size = 0;
            index = 0;
        }
    };

    CircBuffer<halo::objects::s_halo_biped, 30> playerHistory[16];

    typedef CircBuffer<halo::objects::s_halo_vehicle, 30> VehicleCircBuffer;
    std::unordered_map<unsigned long, VehicleCircBuffer> vehicleHistory;

    bool ignoreUntilNewGame = true;

    // ---------------------------------------------
    //

    void OnObjectDestroy(ident objid)
    {
        auto itr = vehicleHistory.find(objid);
        if (itr != vehicleHistory.end()) {
            vehicleHistory.erase(itr);
        }
    }

    void OnObjectCreation(ident objid) 
    {
        objects::s_halo_object* obj = (objects::s_halo_object*)objects::GetObjectAddress(objid);
        if (obj && obj->objectType == objects::e_object_type::vehicle) {
            vehicleHistory.emplace(std::make_pair(objid, VehicleCircBuffer()));
        }
    }

    void OnGameEnd()
    {
        vehicleHistory.clear();
        for (int i = 0; i < 16; i++)
            playerHistory[i].clear();

        ignoreUntilNewGame = true;
    }

    void OnGameStart()
    {
        ignoreUntilNewGame = false;
    }

    void __stdcall OnTick()
    {
        if (ignoreUntilNewGame) return;

        bytesCopied = 0;
        for (int i = 0; i < 16; i++) {
            s_player* player = game::getPlayer(i);
            if (player) {
                auto& history = playerHistory[player->memory_id];

                objects::s_halo_biped* obj = (objects::s_halo_biped*)
                    objects::GetObjectAddress(player->mem->object_id);
                if (obj) {
                    history.add(obj);
                } else {
                    // object is no longer valid (i.e. player died)
                    history.clear();
                }
            }
        }

        for (auto itr = vehicleHistory.begin(); itr != vehicleHistory.end(); ++itr) {
            objects::s_halo_vehicle* obj = (objects::s_halo_vehicle*)
                objects::GetObjectAddress(make_ident(itr->first));
            if (obj) {
                itr->second.add(obj);
            } else {
                itr->second.clear();
            }
        }
    }

    void RollbackObjects(unsigned int ticks)
    {

    }

    void RestoreObjects()
    {

    }

    void __stdcall OnProjectileMove(halo::ident obj)
    {
        objects::s_halo_object* proj = (objects::s_halo_object*)objects::GetObjectAddress(obj);
        if (!proj) return;
        
        s_player* player = game::getPlayer(proj->ownerPlayer.slot);
        if (!player) return;

        // todo: better pings, interpolation between ticks
        auto ticks = player->mem->ping / 33;
        RollbackObjects(ticks);
    }

    void __stdcall OnProjectileMoveRet()
    {
        RestoreObjects();
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