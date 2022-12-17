#pragma once

#include "Settings.h"

namespace Scripts {
	void SetBool(std::monostate, RE::BSFixedString varName, bool value) {
		if (varName == "bUseInfiniteAmmo")
			bUseInfiniteAmmo = value;
		else if (varName == "bUseInfiniteThrowableWeapon")
			bUseInfiniteThrowableWeapon = value;
		else if (varName == "bUseWhiteListMode")
			bUseWhiteListMode = value;
	}

	void SetInt(std::monostate, RE::BSFixedString varName, uint32_t value) {
		if (varName == "iNeverEndingCapacity")
			iNeverEndingCapacity = static_cast<uint16_t>(value);
		else if (varName == "iMinAmmoCapacityMult")
			iMinAmmoCapacityMult = value;
	}

	RE::BSFixedString RunCommand_Native(std::monostate, RE::BSFixedString cmd) {
		F4SE::stl::zstring result = ""sv;
		if (cmd == "AddWeapon_Excluded")
			result = Settings::AddCurrentWeaponToList(true, InfiniteAmmo::EquipIndex::kPrimary);
		else if (cmd == "AddThrowable_Excluded")
			result = Settings::AddCurrentWeaponToList(true, InfiniteAmmo::EquipIndex::kThrowable);
		else if (cmd == "RemoveWeapon_Excluded")
			result = Settings::RemoveCurrentWeaponFromList(true, InfiniteAmmo::EquipIndex::kPrimary);
		else if (cmd == "RemoveThrowable_Excluded")
			result = Settings::RemoveCurrentWeaponFromList(true, InfiniteAmmo::EquipIndex::kThrowable);
		else if (cmd == "ClearWeapons_Excluded") {
			result = "제외목록이 초기화되었습니다"sv;
			excludedWeapons.clear();
		}

		else if (cmd == "AddWeapon_Included")
			result = Settings::AddCurrentWeaponToList(false, InfiniteAmmo::EquipIndex::kPrimary);
		else if (cmd == "AddThrowable_Included")
			result = Settings::AddCurrentWeaponToList(false, InfiniteAmmo::EquipIndex::kThrowable);
		else if (cmd == "RemoveWeapon_Included")
			result = Settings::RemoveCurrentWeaponFromList(false, InfiniteAmmo::EquipIndex::kPrimary);
		else if (cmd == "RemoveThrowable_Included")
			result = Settings::RemoveCurrentWeaponFromList(false, InfiniteAmmo::EquipIndex::kThrowable);
		else if (cmd == "ClearWeapons_Included") {
			result = "포함목록이 초기화되었습니다"sv;
			includedWeapons.clear();
		}

		Settings::UpdateWeaponsConfigFile();
		return result;
	}

	bool Install(RE::BSScript::IVirtualMachine* a_vm) {
		a_vm->BindNativeMethod("InfiniteAmmo"sv, "SetBool"sv, SetBool);
		a_vm->BindNativeMethod("InfiniteAmmo"sv, "SetInt"sv, SetInt);
		a_vm->BindNativeMethod("InfiniteAmmo"sv, "RunCommand_Native"sv, RunCommand_Native);
		return true;
	}
}
