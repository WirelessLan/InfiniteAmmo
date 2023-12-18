#include "InfiniteAmmo.h"

#include "Utils.h"

namespace InfiniteAmmo {
	std::uint16_t g_currentAmmoCapacity;
	std::uint16_t g_neverEndingCapacity = 1;
	std::uint32_t g_minAmmoCapacityMult = 2;
	bool g_useInfiniteAmmo = true;
	bool g_useInfiniteThrowableWeapon = true;
	bool g_useWhiteListMode = false;
	std::unordered_set<std::uint32_t> g_excludedWeapons;
	std::unordered_set<std::uint32_t> g_includedWeapons;

	bool IsExcludedWeapon(std::uint32_t a_weapFormId) {
		return g_excludedWeapons.find(a_weapFormId) != g_excludedWeapons.end();
	}

	bool IsIncludedWeapon(std::uint32_t a_weapFormId) {
		return g_includedWeapons.find(a_weapFormId) != g_includedWeapons.end();
	}

	bool IsThrowableWeapon(RE::BGSEquipIndex& a_equipIndex) {
		return a_equipIndex.index == EquipIndex::kThrowable;
	}

	RE::TESObjectWEAP::InstanceData* GetWeaponInstanceData(RE::TESForm* a_weapForm, RE::TBO_InstanceData* a_weapInst) {
		if (a_weapInst) {
			RE::TESObjectWEAP::InstanceData* weapInstData = RE::fallout_cast<RE::TESObjectWEAP::InstanceData*, RE::TBO_InstanceData>(a_weapInst);
			if (weapInstData)
				return weapInstData;
		}

		if (a_weapForm) {
			RE::TESObjectWEAP* objWeap = a_weapForm->As<RE::TESObjectWEAP>();
			if (objWeap)
				return &objWeap->weaponData;
		}

		return nullptr;
	}

	RE::EquippedItem* GetEquippedItemByFormID(std::uint32_t a_formId) {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh)
			return nullptr;

		if (player->currentProcess->middleHigh->equippedItems.empty())
			return nullptr;

		for (RE::EquippedItem& item : player->currentProcess->middleHigh->equippedItems) {
			if (item.item.object->formID == a_formId)
				return &item;
		}

		return nullptr;
	}

	RE::EquippedItem* GetEquippedItemByEquipIndex(EquipIndex a_equipIndex) {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh)
			return nullptr;

		if (player->currentProcess->middleHigh->equippedItems.empty())
			return nullptr;

		for (RE::EquippedItem& item : player->currentProcess->middleHigh->equippedItems) {
			if (item.equipIndex.index == a_equipIndex)
				return &item;
		}

		return nullptr;
	}

	std::uint32_t GetInventoryItemCount(RE::Actor* a_actor, RE::TESForm* a_item) {
		if (!a_actor || !a_item)
			return 0;

		RE::BGSInventoryList* inventory = a_actor->inventoryList;
		if (!inventory)
			return 0;

		std::uint32_t totalItemCount = 0;

		inventory->rwLock.lock_read();

		for (RE::BGSInventoryItem& item : inventory->data) {
			if (item.object != a_item)
				continue;

			RE::BGSInventoryItem::Stack* sp = item.stackData.get();
			while (sp) {
				totalItemCount += sp->count;
				sp = sp->nextStack.get();
			}

			break;
		}

		inventory->rwLock.unlock_read();

		return totalItemCount;
	}

	std::uint16_t GetAmmoType(RE::TESObjectWEAP::InstanceData* a_weapInst, std::uint16_t a_ammoCapacity) {
		std::uint16_t ammoType = AmmoType::kDefault;

		if (!a_weapInst || !a_weapInst->ammo)
			return ammoType;

		// 현재 장비한 무기의 최대 장전 가능 탄약량이 0일때: 끝없는 
		if (a_ammoCapacity == 0)
			ammoType |= AmmoType::kNeverEnding;

		// 현재 장비한 무기의 탄약의 Health가 0이 아닐때: 퓨전코어
		if (a_weapInst->ammo->data.health != 0)
			ammoType |= AmmoType::kFusionCore;

		// 현재 장비한 무기의 플래그에 ChargingReload가 있을때: 충전식 장전
		if (a_weapInst->flags & WeaponFlags::kChargingReload)
			ammoType |= AmmoType::kCharging;

		return ammoType;
	}

	std::uint16_t GetAmmoCapacity(RE::TESObjectWEAP::InstanceData* a_weapInst, std::uint16_t a_weapAmmoCap) {
		if (!a_weapInst || !a_weapInst->ammo)
			return 0;

		// 탄약의 타입을 체크
		std::uint16_t ammoType = GetAmmoType(a_weapInst, a_weapAmmoCap);

		// 현재 무기의 장전가능한 탄환 수량 조회
		std::uint16_t ammoCapacity = 0;
		// 일반 무기의 경우 기존의 장전가능한 탄환 수량을 이용하면 됨
		if (ammoType == AmmoType::kDefault || ammoType == AmmoType::kCharging)
			ammoCapacity = a_weapAmmoCap;
		// 끝없는 무기의 경우 장전가능 탄환 수량 0
		else if (ammoType & AmmoType::kNeverEnding)
			ammoCapacity = 0;
		// 퓨전코어 무기의 경우 무조건 퓨전코어 1개
		else if (ammoType & AmmoType::kFusionCore)
			ammoCapacity = 1;

		return ammoCapacity;
	}

	std::uint16_t GetCurrentAmmoCapacity(RE::TESObjectWEAP::InstanceData* a_weapInst) {
		if (!a_weapInst)
			return 0;

		return GetAmmoCapacity(a_weapInst, a_weapInst->ammoCapacity);
	}

	std::uint16_t GetCurrentAmmoCapacity() {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh)
			return 0;

		if (player->currentProcess->middleHigh->equippedItems.empty())
			return 0;

		for (RE::EquippedItem& item : player->currentProcess->middleHigh->equippedItems) {
			if (!IsThrowableWeapon(item.equipIndex))
				return GetCurrentAmmoCapacity(GetWeaponInstanceData(item.item.object, item.item.instanceData.get()));
		}

		return 0;
	}

	void AddAmmo(RE::TESForm* a_weapForm, RE::TESObjectWEAP::InstanceData* a_weapInst) {
		if (!a_weapForm || !a_weapInst || !a_weapInst->ammo)
			return;

		// 탄약 무한 옵션이 꺼져있을 때 무시
		if (!g_useInfiniteAmmo)
			return;

		// 현재 무기가 제외 무기일 경우 무시
		if (IsExcludedWeapon(a_weapForm->formID))
			return;

		// 화이트리스트 모드가 켜져있고 현재 무기가 화이트리스트에 포함되지 않는 경우 무시
		if (g_useWhiteListMode && !IsIncludedWeapon(a_weapForm->formID))
			return;

		std::uint16_t ammoCapacity = g_currentAmmoCapacity;

		// 끝없는 무기의 경우 끝없는 무기 기본 탄환 수량을 이용
		if (GetAmmoType(a_weapInst, ammoCapacity) & AmmoType::kNeverEnding)
			ammoCapacity = g_neverEndingCapacity;

		// 현재 탄환의 총 보유수량 조회
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		std::uint32_t totalAmmoCount = GetInventoryItemCount(player, a_weapInst->ammo);

		// 탄약 추가
		if (totalAmmoCount < static_cast<std::uint32_t>(ammoCapacity) * g_minAmmoCapacityMult) {
			std::uint32_t diff = static_cast<std::uint32_t>(ammoCapacity) * g_minAmmoCapacityMult - totalAmmoCount;
			Utils::AddItem(player, a_weapInst->ammo, diff, true);
		}
	}

	bool IsInfiniteThrowable(RE::TESForm* a_weapForm) {
		if (!a_weapForm)
			return false;

		// 투척무기 무한이 아닌 경우 유한
		if (!g_useInfiniteThrowableWeapon)
			return false;

		// 제외무기에 포함된 경우 유한
		if (IsExcludedWeapon(a_weapForm->formID))
			return false;

		// 화이트리스트 모드이고 화이트 리스트 무기가 아닌 경우 유한
		if (g_useWhiteListMode && !IsIncludedWeapon(a_weapForm->formID))
			return false;

		return true;
	}

	bool IsNeverEndingWeapon(RE::TESForm* a_weapForm, RE::TESObjectWEAP::InstanceData* a_weapInst) {
		if (!a_weapForm || !a_weapInst || !a_weapInst->ammo)
			return false;

		// 탄약무한이 아닌 경우 유한
		if (!g_useInfiniteAmmo)
			return false;

		// 제외무기에 포함된 경우 유한
		if (IsExcludedWeapon(a_weapForm->formID))
			return false;

		// 화이트리스트 모드이고 화이트 리스트 무기가 아닌 경우 유한
		if (g_useWhiteListMode && !IsIncludedWeapon(a_weapForm->formID))
			return false;

		return GetAmmoType(a_weapInst, g_currentAmmoCapacity) & AmmoType::kNeverEnding;
	}
}
