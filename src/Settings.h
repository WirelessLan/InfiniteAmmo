#pragma once

#include "InfiniteAmmo.h"

namespace Settings {
	bool GetConfigValue(const char* a_section, const char* a_key, std::uint32_t* a_dataOut);
	bool GetConfigValue(const char* a_section, const char* a_key, std::uint16_t* a_dataOut);
	bool GetConfigValue(const char* a_section, const char* a_key, bool* a_dataOut);
	void UpdateWeaponsConfigFile();
	void LoadSettings();
	std::string_view AddCurrentWeaponToList(bool a_isExcluded, InfiniteAmmo::EquipIndex a_equipIndex);
	std::string_view RemoveCurrentWeaponFromList(bool a_isExcluded, InfiniteAmmo::EquipIndex a_equipIndex);
}
