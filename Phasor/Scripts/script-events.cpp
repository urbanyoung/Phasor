#include "script-events.h"

namespace scripting {
	namespace events {

		const std::vector<std::string> eventList
		{
			"OnScriptUnload",
			"OnTeamChange",
			"OnServerCommand",
			"OnServerCommandAttempt",
			"OnNewGame",
			"OnGameEnd",
			"OnBanCheck",
			"OnClientUpdate",
			"OnPlayerJoin",
			"OnPlayerLeave",
			"OnTeamDecision",
			"OnPlayerSpawn",
			"OnPlayerSpawnEnd",
			"OnObjectCreation",
			"OnObjectCreationAttempt",
			"OnWeaponAssignment",
			"OnObjectInteraction",
			"OnDamageLookup",
			"OnDamageApplication",
			"OnServerChat",
			"OnVehicleEntry",
			"OnVehicleEject",
			"OnPlayerKill",
			"OnKillMultiplier",
			"OnWeaponReload",
			"OnNameRequest"
		};
	}
}