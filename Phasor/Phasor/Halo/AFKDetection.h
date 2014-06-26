#pragma once

#include "Player.h"
#include "../../Common/Timers.h"
#include "../../Common/vect3d.h"
#include "Server/Server.h"

namespace halo {
    namespace afk_detection
    {
        class CAFKDetection
        {
        private:
            vect3d camera;
            DWORD move_count, afk_duration, timer_id;
            Timers& timers;
            s_player& player;

        public:
            CAFKDetection(s_player& player, Timers& timers);
            ~CAFKDetection();
            void CheckPlayerActivity();
            void MarkPlayerActive();

            // Called when the timer expires (check if they're afk)
            void CheckInactivity();

            static DWORD max_duration; // max number of minutes inactive
            static const DWORD kMoveCountThreshold;
            static bool bDisable;
        };

        void Enable();
        void Disable();
        e_command_result sv_kickafk(void*,
                                    commands::CArgParser& args, COutStream& out);
    }
}