#pragma once

#include <vector>
#include <Windows.h>

namespace Core
{
	// Codecaves
	namespace Events
	{
		void OnConsoleProcessing_CC();
		void ConsoleHandler_CC();
		void OnServerCommand();
		void OnMapLoading_CC();
		void OnGameEnd_CC();
		void OnNewGame_CC();
		void OnPlayerWelcome_CC();
		void OnPlayerQuit_CC();
		void OnTeamSelection_CC();
		void OnTeamChange_CC();
		void OnPlayerSpawn_CC();
		void OnPlayerSpawEnd_CC();
		void OnObjectCreation_CC();
		void OnWeaponAssignment_CC();
		void OnObjectInteration_CC();
		void OnClientUpdate_CC();
		void OnDamageLookup_CC();
		void OnChat_CC();
		void OnVehicleEntry_CC();
		void OnWeaponReload_CC();
		void OnDeath_CC();
		void OnKillMultiplier_CC();
		void OnObjectRespawn_CC();
		void OnEquipmentDestroy_CC();
		void OnVehicleForceEject_CC();
		void OnVehicleUserEject_CC();
		void OnHaloPrint_CC();
		void OnHaloBanCheck_CC();
	}

	
}