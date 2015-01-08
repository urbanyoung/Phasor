#include "AFKDetection.h"
#include "../../Common/MyString.h"
#include "Server/Server.h"
#include "../Globals.h"

namespace halo { namespace afk_detection
{
	DWORD CAFKDetection::max_duration = 0;
	bool CAFKDetection::bDisable = false;
	const DWORD CAFKDetection::kMoveCountThreshold = 100;

	// timer used by
	class CAFKDetectionEvent : public TimerEvent
	{
	private:
		DWORD dwDelay;
		CAFKDetection& parent;

	public:
		CAFKDetectionEvent(DWORD dwDelay, CAFKDetection& parent)
			: dwDelay(dwDelay), parent(parent), TimerEvent(dwDelay)
		{
		}

		virtual ~CAFKDetectionEvent() {}

		virtual bool OnExpiration(Timers& timers)
		{
			parent.CheckInactivity();
			return true;
		}
	};

	CAFKDetection::CAFKDetection(s_player& player, Timers& timers)
		: player(player), timers(timers), move_count(0), afk_duration(0)
	{
		timer_ptr timer(new CAFKDetectionEvent(60000,*this));
		timer_id = timers.AddTimer(std::move(timer));
	}

	CAFKDetection::~CAFKDetection()
	{
		timers.RemoveTimer(timer_id);
	}

	void CAFKDetection::CheckPlayerActivity()
	{
		// make sure the object is available
		objects::s_halo_biped* object = player.get_object();
		if (object)	{
			vect3d new_camera = object->cameraView;

			// check if the camera has moved
			if (new_camera != camera) move_count++;

			camera = new_camera;

			// Check if the player is shooting/throwing nades etc, if so they are not afk
			if (object->actionFlags.melee || 
				object->actionFlags.primaryWeaponFire ||
				object->actionFlags.secondaryWeaponFire)
				MarkPlayerActive();
		}
	}

	void CAFKDetection::MarkPlayerActive()
	{
		afk_duration = 0;
		move_count = kMoveCountThreshold + 1;
	}

	void CAFKDetection::CheckInactivity()
	{
		if (max_duration == 0 || bDisable || player.is_admin) return;
		if (move_count <= kMoveCountThreshold) {
			afk_duration++;
			if (afk_duration >= max_duration) {
				player.Kick();
				TempForwarder f(server::say_stream, TempForwarder::end_point(*g_PrintStream));
                g_GameLog->WriteLog(kPlayerLeave, "Kicked due to inactivity");
				f << player.mem->playerName << " has been kicked due to inactivity." << endl;

			} else {
				player.chat_stream->wprint(L"You don't appear to be playing. You will be kicked in %i minute(s), if you remain inactive.", 
					max_duration - afk_duration);
			}
		} else afk_duration = 0;
	
		move_count = 0;
	}

	void Disable() { CAFKDetection::bDisable = true; }
	void Enable()  { CAFKDetection::bDisable = false; }

	e_command_result sv_kickafk(void*, 
		commands::CArgParser& args, COutStream& out)
	{
		CAFKDetection::max_duration = args.ReadUInt();

		if (CAFKDetection::max_duration)
			out << "Inactive players will be kicked after " << 
			CAFKDetection::max_duration << " minutes(s)" << endl;
		else
			out << "Inactive players will no longer be kicked." << endl;

		return e_command_result::kProcessed;
	}

}}