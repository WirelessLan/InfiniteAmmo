#pragma once

#include <unordered_set>

namespace InfiniteAmmo {
	enum AmmoType {
		kDefault = 0x00,
		kCharging = 0x01,
		kFusionCore = 0x02,
		kNeverEnding = 0x04
	};

	enum WeaponFlags {
		kChargingReload = 0x0000008
	};

	enum EquipIndex : uint32_t {
		kPrimary = 0,
		kThrowable = 2
	};

	extern std::uint16_t g_currentAmmoCapacity;
	extern std::uint16_t g_neverEndingCapacity;
	extern std::uint32_t g_minAmmoCapacityMult;
	extern bool g_useInfiniteAmmo;
	extern bool g_useInfiniteThrowableWeapon;
	extern bool g_useWhiteListMode;
	extern std::unordered_set<std::uint32_t> g_excludedWeapons;
	extern std::unordered_set<std::uint32_t> g_includedWeapons;

	bool IsExcludedWeapon(std::uint32_t a_weapFormId);
	bool IsIncludedWeapon(std::uint32_t a_weapFormId);
	bool IsThrowableWeapon(RE::BGSEquipIndex& a_equipIndex);
	RE::TESObjectWEAP::InstanceData* GetWeaponInstanceData(RE::TESForm* a_weapForm, RE::TBO_InstanceData* a_weapInst);
	RE::EquippedItem* GetEquippedItemByFormID(std::uint32_t a_formId);
	RE::EquippedItem* GetEquippedItemByEquipIndex(EquipIndex a_equipIndex);
	std::uint32_t GetInventoryItemCount(RE::Actor* a_actor, RE::TESForm* a_item);
	std::uint16_t GetCurrentAmmoCapacity(RE::TESObjectWEAP::InstanceData* a_weapInst);
	std::uint16_t GetCurrentAmmoCapacity();
	void AddAmmo(RE::TESForm* a_weapForm, RE::TESObjectWEAP::InstanceData* a_weapInst);
	bool IsInfiniteThrowable(RE::TESForm* a_weapForm);
	bool IsNeverEndingWeapon(RE::TESForm* a_weapForm, RE::TESObjectWEAP::InstanceData* a_weapInst);
}
