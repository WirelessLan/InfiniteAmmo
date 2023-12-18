#include "Scripts.h"

#include "InfiniteAmmo.h"
#include "Settings.h"

namespace Scripts {
	void SetBool(std::monostate, std::string_view a_varName, bool a_value) {
		if (a_varName == "bUseInfiniteAmmo")
			InfiniteAmmo::g_useInfiniteAmmo = a_value;
		else if (a_varName == "bUseInfiniteThrowableWeapon")
			InfiniteAmmo::g_useInfiniteThrowableWeapon = a_value;
		else if (a_varName == "bUseWhiteListMode")
			InfiniteAmmo::g_useWhiteListMode = a_value;
	}

	void SetInt(std::monostate, std::string_view a_varName, std::uint32_t a_value) {
		if (a_varName == "iNeverEndingCapacity")
			InfiniteAmmo::g_neverEndingCapacity = static_cast<std::uint16_t>(a_value);
		else if (a_varName == "iMinAmmoCapacityMult")
			InfiniteAmmo::g_minAmmoCapacityMult = a_value;
	}

	std::string_view RunCommand_Native(std::monostate, std::string_view a_cmd) {
		std::string_view result;

		if (a_cmd == "AddWeapon_Excluded")
			result = Settings::AddCurrentWeaponToList(true, InfiniteAmmo::EquipIndex::kPrimary);
		else if (a_cmd == "AddThrowable_Excluded")
			result = Settings::AddCurrentWeaponToList(true, InfiniteAmmo::EquipIndex::kThrowable);
		else if (a_cmd == "RemoveWeapon_Excluded")
			result = Settings::RemoveCurrentWeaponFromList(true, InfiniteAmmo::EquipIndex::kPrimary);
		else if (a_cmd == "RemoveThrowable_Excluded")
			result = Settings::RemoveCurrentWeaponFromList(true, InfiniteAmmo::EquipIndex::kThrowable);
		else if (a_cmd == "ClearWeapons_Excluded") {
			result = "제외목록이 초기화되었습니다"sv;
			InfiniteAmmo::g_excludedWeapons.clear();
		}

		else if (a_cmd == "AddWeapon_Included")
			result = Settings::AddCurrentWeaponToList(false, InfiniteAmmo::EquipIndex::kPrimary);
		else if (a_cmd == "AddThrowable_Included")
			result = Settings::AddCurrentWeaponToList(false, InfiniteAmmo::EquipIndex::kThrowable);
		else if (a_cmd == "RemoveWeapon_Included")
			result = Settings::RemoveCurrentWeaponFromList(false, InfiniteAmmo::EquipIndex::kPrimary);
		else if (a_cmd == "RemoveThrowable_Included")
			result = Settings::RemoveCurrentWeaponFromList(false, InfiniteAmmo::EquipIndex::kThrowable);
		else if (a_cmd == "ClearWeapons_Included") {
			result = "포함목록이 초기화되었습니다"sv;
			InfiniteAmmo::g_includedWeapons.clear();
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
