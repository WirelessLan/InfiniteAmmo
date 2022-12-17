#pragma once 

#include <Windows.h>
#include <fstream>

namespace Settings {
	std::string GetINIOption(const char* section, const char* key) {
		std::string	result;
		char resultBuf[256] = { 0 };

		static const std::string& configPath = "Data\\MCM\\Settings\\" + std::string(Version::PROJECT) + ".ini";
		GetPrivateProfileStringA(section, key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());
		return resultBuf;
	}

	bool GetConfigValue(const char* section, const char* key, uint32_t* dataOut) {
		std::string	data = GetINIOption(section, key);
		if (data.empty())
			return false;

		uint32_t tmp;
		bool res = (sscanf_s(data.c_str(), "%u", &tmp) == 1);
		if (res)
			*dataOut = tmp;
		return res;
	}

	bool GetConfigValue(const char* section, const char* key, uint16_t* dataOut) {
		std::string	data = GetINIOption(section, key);
		if (data.empty())
			return false;

		uint16_t tmp;
		bool res = (sscanf_s(data.c_str(), "%hu", &tmp) == 1);
		if (res)
			*dataOut = tmp;
		return res;
	}

	bool GetConfigValue(const char* section, const char* key, bool* dataOut) {
		std::string	data = GetINIOption(section, key);
		if (data.empty())
			return false;

		int32_t tmp;
		bool res = (sscanf_s(data.c_str(), "%d", &tmp) == 1);
		if (res)
			*dataOut = tmp;
		return res;
	}

	char GetNextChar(const std::string& line, uint32_t& index) {
		if (index < line.length())
			return line[index++];

		return -1;
	}

	std::string GetNextData(const std::string& line, uint32_t& index, char delimeter) {
		char ch;
		std::string retVal = "";

		while ((ch = GetNextChar(line, index)) > 0) {
			if (ch == '#') {
				if (index > 0) index--;
				break;
			}

			if (delimeter != 0 && ch == delimeter)
				break;

			retVal += ch;
		}

		Utils::trim(retVal);
		return retVal;
	}

	void ReadWeaponsConfigFile() {
		std::string settingFilePath = "Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) + "_Weapons.cfg";
		std::ifstream settingFile(settingFilePath);

		excludedWeapons.clear();
		includedWeapons.clear();

		if (!settingFile.is_open()) {
			logger::warn(FMT_STRING("Cannot open the config file - {}"), settingFilePath);
			return;
		}

		std::string line;
		std::string lineType, pluginName, formId;
		while (std::getline(settingFile, line)) {
			Utils::trim(line);
			if (line.empty() || line[0] == '#')
				continue;

			uint32_t index = 0;

			lineType = GetNextData(line, index, '|');
			if (lineType.empty()) {
				logger::warn(FMT_STRING("Cannot read a line type - {}"), line);
				continue;
			}

			if (lineType == "Exclude" || lineType == "Include") {
				pluginName = GetNextData(line, index, '|');
				if (pluginName.empty()) {
					logger::warn(FMT_STRING("Cannot read a plugin name - {}"), line);
					continue;
				}

				formId = GetNextData(line, index, 0);
				if (formId.empty()) {
					logger::warn(FMT_STRING("Cannot read a formId - {}"), line);
					continue;
				}

				RE::TESForm* weapForm = Utils::GetFormFromIdentifier(pluginName, formId);
				if (!weapForm) {
					logger::warn(FMT_STRING("Cannot find a weapon - {}"), line);
					continue;
				}

				if (lineType == "Exclude")
					excludedWeapons.insert(weapForm->formID);
				else if (lineType == "Include")
					includedWeapons.insert(weapForm->formID);

				logger::info(FMT_STRING("{} Weapon: {} | {:08X}"), lineType, pluginName, weapForm->formID);
			}
			else {
				logger::warn(FMT_STRING("Cannot determine a line type - {}"), line);
				continue;
			}
		}

		settingFile.close();
	}

	void UpdateWeaponsConfigFile() {
		std::string settingFilePath = "Data\\F4SE\\Plugins\\" + std::string(Version::PROJECT) + "_Weapons.cfg";
		std::ofstream settingFile(settingFilePath);

		if (!settingFile.is_open()) {
			logger::warn(FMT_STRING("Cannot open the config file - {}"), settingFilePath);
			return;
		}

		for (uint32_t element : excludedWeapons) {
			RE::TESForm* weapForm = RE::TESForm::GetFormByID(element);
			if (!weapForm)
				continue;

			RE::TESFile* modInfo = weapForm->sourceFiles.array->front();
			if (!modInfo)
				continue;

			uint32_t formId = Utils::IsLight(modInfo) ? element & 0xFFF : element & 0xFFFFFF;
			settingFile << "Exclude|" << modInfo->filename << "|" << std::uppercase << std::hex << formId << std::endl;
		}

		for (uint32_t element : includedWeapons) {
			RE::TESForm* weapForm = RE::TESForm::GetFormByID(element);
			if (!weapForm)
				continue;

			RE::TESFile* modInfo = weapForm->sourceFiles.array->front();
			if (!modInfo)
				continue;

			uint32_t formId = Utils::IsLight(modInfo) ? element & 0xFFF : element & 0xFFFFFF;
			settingFile << "Include|" << modInfo->filename << "|" << std::uppercase << std::hex << formId << std::endl;
		}

		settingFile.close();
	}

	void LoadSettings() {
		bUseInfiniteAmmo = true;
		bUseInfiniteThrowableWeapon = true;
		iNeverEndingCapacity = 1;
		iMinAmmoCapacityMult = 2;
		bUseWhiteListMode = false;

		GetConfigValue("Settings", "bUseInfiniteAmmo", &bUseInfiniteAmmo);
		GetConfigValue("Settings", "bUseInfiniteThrowableWeapon", &bUseInfiniteThrowableWeapon);
		GetConfigValue("Settings", "iNeverEndingCapacity", &iNeverEndingCapacity);
		GetConfigValue("Settings", "iMinAmmoCapacityMult", &iMinAmmoCapacityMult);
		GetConfigValue("Settings", "bUseWhiteListMode", &bUseWhiteListMode);

		logger::info(FMT_STRING("bUseInfiniteAmmo: {}"), bUseInfiniteAmmo);
		logger::info(FMT_STRING("bUseInfiniteThrowableWeapon: {}"), bUseInfiniteThrowableWeapon);
		logger::info(FMT_STRING("iNeverEndingCapacity: {}"), iNeverEndingCapacity);
		logger::info(FMT_STRING("iMinAmmoCapacityMult: {}"), iMinAmmoCapacityMult);
		logger::info(FMT_STRING("bUseWhiteListMode: {}"), bUseWhiteListMode);

		ReadWeaponsConfigFile();
	}

	F4SE::stl::zstring AddCurrentWeaponToList(bool isExcluded, InfiniteAmmo::EquipIndex equipIndex) {
		RE::EquippedItem* equipData = InfiniteAmmo::GetEquippedItemByEquipIndex(equipIndex);
		if (!equipData)
			return "현재 무기를 장착중인 상태가 아닙니다"sv;

		if (excludedWeapons.find(equipData->item.object->formID) != excludedWeapons.end())
			return "제외목록에 포함되어있는 무기입니다"sv;

		if (includedWeapons.find(equipData->item.object->formID) != includedWeapons.end())
			return "포함목록에 포함되어있는 무기입니다"sv;

		if (isExcluded) {
			excludedWeapons.insert(equipData->item.object->formID);
			return "현재 무기가 제외목록에 추가되었습니다"sv;
		}
		else {
			includedWeapons.insert(equipData->item.object->formID);
			return "현재 무기가 포함목록에 추가되었습니다"sv;
		}
	}

	F4SE::stl::zstring RemoveCurrentWeaponFromList(bool isExcluded, InfiniteAmmo::EquipIndex equipIndex) {
		RE::EquippedItem* equipData = InfiniteAmmo::GetEquippedItemByEquipIndex(equipIndex);
		if (!equipData)
			return "현재 무기를 장착중인 상태가 아닙니다"sv;

		if (isExcluded) {
			if (excludedWeapons.find(equipData->item.object->formID) == excludedWeapons.end())
				return "제외목록에 포함되어있지 않은 무기입니다"sv;
			excludedWeapons.erase(equipData->item.object->formID);
			return "현재 무기가 제외목록에서 제거되었습니다"sv;
		}
		else {
			if (includedWeapons.find(equipData->item.object->formID) == includedWeapons.end())
				return "포함목록에 포함되어있지 않은 무기입니다"sv;
			includedWeapons.erase(equipData->item.object->formID);
			return "현재 무기가 포함목록에서 제거되었습니다"sv;
		}
	}
}
