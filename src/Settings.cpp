#include "Settings.h"

#include <Windows.h>
#include <fstream>

#include "InfiniteAmmo.h"
#include "Utils.h"

namespace Settings {
	std::string GetINIOption(const char* a_section, const char* a_key) {
		std::string	result;
		char resultBuf[256] = { 0 };

		static const std::string& configPath = "Data\\MCM\\Settings\\" + std::string(Version::PROJECT) + ".ini";
		GetPrivateProfileStringA(a_section, a_key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());
		return resultBuf;
	}

	bool GetConfigValue(const char* a_section, const char* a_key, std::uint32_t* a_dataOut) {
		std::string	data = GetINIOption(a_section, a_key);
		if (data.empty())
			return false;

		std::uint32_t tmp;
		bool res = (sscanf_s(data.c_str(), "%u", &tmp) == 1);
		if (res)
			*a_dataOut = tmp;
		return res;
	}

	bool GetConfigValue(const char* a_section, const char* a_key, std::uint16_t* a_dataOut) {
		std::string	data = GetINIOption(a_section, a_key);
		if (data.empty())
			return false;

		std::uint16_t tmp;
		bool res = (sscanf_s(data.c_str(), "%hu", &tmp) == 1);
		if (res)
			*a_dataOut = tmp;
		return res;
	}

	bool GetConfigValue(const char* a_section, const char* a_key, bool* a_dataOut) {
		std::string	data = GetINIOption(a_section, a_key);
		if (data.empty())
			return false;

		std::int32_t tmp;
		bool res = (sscanf_s(data.c_str(), "%d", &tmp) == 1);
		if (res)
			*a_dataOut = tmp;
		return res;
	}

	std::uint8_t GetNextChar(const std::string& a_line, std::uint32_t& a_index) {
		if (a_index < a_line.length())
			return a_line[a_index++];
		return 0xFF;
	}

	std::string GetNextData(const std::string& a_line, std::uint32_t& a_index, char a_delimeter) {
		std::uint8_t ch;
		std::string retVal = "";

		while ((ch = GetNextChar(a_line, a_index)) != 0xFF) {
			if (ch == '#') {
				if (a_index > 0)
					a_index--;
				break;
			}

			if (a_delimeter != 0 && ch == a_delimeter)
				break;

			retVal += ch;
		}

		return Utils::Trim(retVal);
	}

	void ReadWeaponsConfigFile() {
		std::string settingFilePath = "Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) + "_Weapons.cfg";
		std::ifstream settingFile(settingFilePath);

		InfiniteAmmo::g_excludedWeapons.clear();
		InfiniteAmmo::g_includedWeapons.clear();

		if (!settingFile.is_open()) {
			logger::warn("Cannot open the config file: {}", settingFilePath);
			return;
		}

		std::string line;
		std::string lineType, pluginName, formId;
		while (std::getline(settingFile, line)) {
			line = Utils::Trim(line);
			if (line.empty() || line[0] == '#')
				continue;

			std::uint32_t index = 0;

			lineType = GetNextData(line, index, '|');
			if (lineType.empty()) {
				logger::warn("Cannot read a line type: {}", line);
				continue;
			}

			if (lineType == "Exclude" || lineType == "Include") {
				pluginName = GetNextData(line, index, '|');
				if (pluginName.empty()) {
					logger::warn("Cannot read a plugin name: {}", line);
					continue;
				}

				formId = GetNextData(line, index, 0);
				if (formId.empty()) {
					logger::warn("Cannot read a formId: {}", line);
					continue;
				}

				RE::TESForm* weapForm = Utils::GetFormFromIdentifier(pluginName, formId);
				if (!weapForm) {
					logger::warn("Cannot find a weapon: {}", line);
					continue;
				}

				if (lineType == "Exclude")
					InfiniteAmmo::g_excludedWeapons.insert(weapForm->formID);
				else if (lineType == "Include")
					InfiniteAmmo::g_includedWeapons.insert(weapForm->formID);

				logger::info("{} Weapon: {} | {:08X}", lineType, pluginName, weapForm->formID);
			}
			else {
				logger::warn("Cannot determine a line type: {}", line);
				continue;
			}
		}
	}

	void UpdateWeaponsConfigFile() {
		std::string settingFilePath = "Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) + "_Weapons.cfg";
		std::ofstream settingFile(settingFilePath);

		if (!settingFile.is_open()) {
			logger::warn("Cannot open the config file: {}", settingFilePath);
			return;
		}

		for (std::uint32_t element : InfiniteAmmo::g_excludedWeapons) {
			RE::TESForm* weapForm = RE::TESForm::GetFormByID(element);
			if (!weapForm)
				continue;

			RE::TESFile* modInfo = weapForm->sourceFiles.array->front();
			if (!modInfo)
				continue;

			std::uint32_t formId = modInfo->IsLight() ? element & 0xFFF : element & 0xFFFFFF;
			settingFile << "Exclude|" << modInfo->filename << "|" << std::uppercase << std::hex << formId << std::endl;
		}

		for (std::uint32_t element : InfiniteAmmo::g_includedWeapons) {
			RE::TESForm* weapForm = RE::TESForm::GetFormByID(element);
			if (!weapForm)
				continue;

			RE::TESFile* modInfo = weapForm->sourceFiles.array->front();
			if (!modInfo)
				continue;

			std::uint32_t formId = modInfo->IsLight() ? element & 0xFFF : element & 0xFFFFFF;
			settingFile << "Include|" << modInfo->filename << "|" << std::uppercase << std::hex << formId << std::endl;
		}
	}

	void LoadSettings() {
		InfiniteAmmo::g_useInfiniteAmmo = true;
		InfiniteAmmo::g_useInfiniteThrowableWeapon = true;
		InfiniteAmmo::g_neverEndingCapacity = 1;
		InfiniteAmmo::g_minAmmoCapacityMult = 2;
		InfiniteAmmo::g_useWhiteListMode = false;

		GetConfigValue("Settings", "bUseInfiniteAmmo", &InfiniteAmmo::g_useInfiniteAmmo);
		GetConfigValue("Settings", "bUseInfiniteThrowableWeapon", &InfiniteAmmo::g_useInfiniteThrowableWeapon);
		GetConfigValue("Settings", "iNeverEndingCapacity", &InfiniteAmmo::g_neverEndingCapacity);
		GetConfigValue("Settings", "iMinAmmoCapacityMult", &InfiniteAmmo::g_minAmmoCapacityMult);
		GetConfigValue("Settings", "bUseWhiteListMode", &InfiniteAmmo::g_useWhiteListMode);

		logger::info(FMT_STRING("bUseInfiniteAmmo: {}"), InfiniteAmmo::g_useInfiniteAmmo);
		logger::info(FMT_STRING("bUseInfiniteThrowableWeapon: {}"), InfiniteAmmo::g_useInfiniteThrowableWeapon);
		logger::info(FMT_STRING("iNeverEndingCapacity: {}"), InfiniteAmmo::g_neverEndingCapacity);
		logger::info(FMT_STRING("iMinAmmoCapacityMult: {}"), InfiniteAmmo::g_minAmmoCapacityMult);
		logger::info(FMT_STRING("bUseWhiteListMode: {}"), InfiniteAmmo::g_useWhiteListMode);

		ReadWeaponsConfigFile();
	}

	std::string_view AddCurrentWeaponToList(bool a_isExcluded, InfiniteAmmo::EquipIndex a_equipIndex) {
		RE::EquippedItem* equipData = InfiniteAmmo::GetEquippedItemByEquipIndex(a_equipIndex);
		if (!equipData)
			return "현재 무기를 장착중인 상태가 아닙니다"sv;

		if (InfiniteAmmo::IsExcludedWeapon(equipData->item.object->formID))
			return "제외목록에 포함되어있는 무기입니다"sv;

		if (InfiniteAmmo::IsIncludedWeapon(equipData->item.object->formID))
			return "포함목록에 포함되어있는 무기입니다"sv;

		if (a_isExcluded) {
			InfiniteAmmo::g_excludedWeapons.insert(equipData->item.object->formID);
			return "현재 무기가 제외목록에 추가되었습니다"sv;
		}
		else {
			InfiniteAmmo::g_includedWeapons.insert(equipData->item.object->formID);
			return "현재 무기가 포함목록에 추가되었습니다"sv;
		}
	}

	std::string_view RemoveCurrentWeaponFromList(bool a_isExcluded, InfiniteAmmo::EquipIndex a_equipIndex) {
		RE::EquippedItem* equipData = InfiniteAmmo::GetEquippedItemByEquipIndex(a_equipIndex);
		if (!equipData)
			return "현재 무기를 장착중인 상태가 아닙니다"sv;

		if (a_isExcluded) {
			if (!InfiniteAmmo::IsExcludedWeapon(equipData->item.object->formID))
				return "제외목록에 포함되어있지 않은 무기입니다"sv;
			InfiniteAmmo::g_excludedWeapons.erase(equipData->item.object->formID);
			return "현재 무기가 제외목록에서 제거되었습니다"sv;
		}
		else {
			if (!InfiniteAmmo::IsIncludedWeapon(equipData->item.object->formID))
				return "포함목록에 포함되어있지 않은 무기입니다"sv;
			InfiniteAmmo::g_includedWeapons.erase(equipData->item.object->formID);
			return "현재 무기가 포함목록에서 제거되었습니다"sv;
		}
	}
}
