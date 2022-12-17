#pragma once 

#include <unordered_set>

#include "Utils.h"

extern uint16_t CurrentAmmoCapacity;

uint16_t iNeverEndingCapacity = 1;
uint32_t iMinAmmoCapacityMult = 2;
bool bUseInfiniteAmmo = true;
bool bUseInfiniteThrowableWeapon = true;
bool bUseWhiteListMode = false;
std::unordered_set<uint32_t> excludedWeapons;
std::unordered_set<uint32_t> includedWeapons;

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

	bool IsExcludedWeapon(uint32_t weapFormId) {
		return excludedWeapons.find(weapFormId) != excludedWeapons.end();
	}

	bool IsIncludedWeapon(uint32_t weapFormId) {
		return includedWeapons.find(weapFormId) != includedWeapons.end();
	}

	bool IsThrowableWeapon(RE::BGSEquipIndex& equipIndex) {
		return equipIndex.index == EquipIndex::kThrowable;
	}

	RE::TESObjectWEAP::InstanceData* GetWeaponInstanceData(RE::TESForm* weapForm, RE::TBO_InstanceData* weapInst) {
		if (weapInst) {
			RE::TESObjectWEAP::InstanceData* weapInstData = RE::fallout_cast<RE::TESObjectWEAP::InstanceData*, RE::TBO_InstanceData>(weapInst);
			if (weapInstData)
				return weapInstData;
		}

		if (weapForm) {
			RE::TESObjectWEAP* objWeap = RE::fallout_cast<RE::TESObjectWEAP*, RE::TESForm>(weapForm);
			if (objWeap)
				return &objWeap->weaponData;
		}

		return nullptr;
	}

	RE::EquippedItem* GetEquippedItemByFormID(uint32_t formId) {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->currentProcess || !player->currentProcess->middleHigh)
			return nullptr;

		if (player->currentProcess->middleHigh->equippedItems.empty())
			return nullptr;

		for (RE::EquippedItem& item : player->currentProcess->middleHigh->equippedItems) {
			if (item.item.object->formID == formId)
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

	uint32_t GetInventoryItemCount(RE::Actor* a_actor, RE::TESForm* a_item) {
		if (!a_actor || !a_item)
			return 0;

		RE::BGSInventoryList* inventory = a_actor->inventoryList;
		if (!inventory)
			return 0;

		uint32_t totalItemCount = 0;
		inventory->rwLock.lock_read();
		for (RE::BGSInventoryItem& item : inventory->data) {
			if (item.object == a_item) {
				RE::BGSInventoryItem::Stack* sp = item.stackData.get();
				while (sp) {
					totalItemCount += sp->count;
					sp = sp->nextStack.get();
				}
				break;
			}
		}
		inventory->rwLock.unlock_read();

		return totalItemCount;
	}

	uint16_t GetAmmoType(RE::TESObjectWEAP::InstanceData* weapInst, uint16_t ammoCapacity) {
		uint16_t ammoType = AmmoType::kDefault;

		if (!weapInst || !weapInst->ammo)
			return ammoType;

		// 현재 장비한 무기의 최대 장전 가능 탄약량이 0일때: 끝없는 
		if (ammoCapacity == 0)
			ammoType |= AmmoType::kNeverEnding;

		uint32_t ammoHealth = weapInst->ammo->data.health;
		// 현재 장비한 무기의 탄약의 Health가 0이 아닐때: 퓨전코어
		if (ammoHealth != 0)
			ammoType |= AmmoType::kFusionCore;

		// 현재 장비한 무기의 플래그에 ChargingReload가 있을때: 충전식 장전
		if (weapInst->flags & WeaponFlags::kChargingReload)
			ammoType |= AmmoType::kCharging;

		return ammoType;
	}

	uint16_t GetAmmoCapacity(RE::TESObjectWEAP::InstanceData* weapInst, uint16_t weapAmmoCap) {
		if (!weapInst || !weapInst->ammo)
			return 0;

		// 탄약의 타입을 체크
		uint16_t ammoType = GetAmmoType(weapInst, weapAmmoCap);

		// 현재 무기의 장전가능한 탄환 수량 조회
		uint16_t ammoCapacity = 0;
		// 일반 무기의 경우 기존의 장전가능한 탄환 수량을 이용하면 됨
		if (ammoType == InfiniteAmmo::AmmoType::kDefault || ammoType == InfiniteAmmo::AmmoType::kCharging)
			ammoCapacity = weapAmmoCap;
		// 끝없는 무기의 경우 장전가능 탄환 수량 0
		else if (ammoType & InfiniteAmmo::AmmoType::kNeverEnding)
			ammoCapacity = 0;
		// 퓨전코어 무기의 경우 무조건 퓨전코어 1개
		else if (ammoType & InfiniteAmmo::AmmoType::kFusionCore)
			ammoCapacity = 1;

		return ammoCapacity;
	}

	uint16_t GetCurrentAmmoCapacity(RE::TESObjectWEAP::InstanceData* weapInst) {
		if (!weapInst)
			return 0;
		return GetAmmoCapacity(weapInst, weapInst->ammoCapacity);
	}

	uint16_t GetCurrentAmmoCapacity() {
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

	void AddAmmo(RE::TESForm* weapForm, RE::TESObjectWEAP::InstanceData* weapInst) {
		if (!weapForm || !weapInst || !weapInst->ammo)
			return;

		// 탄약 무한 옵션이 꺼져있을 때 무시
		if (!bUseInfiniteAmmo)
			return;

		// 현재 무기가 제외 무기일 경우 무시
		if (IsExcludedWeapon(weapForm->formID))
			return;

		// 화이트리스트 모드가 켜져있고 현재 무기가 화이트리스트에 포함되지 않는 경우 무시
		if (bUseWhiteListMode && !IsIncludedWeapon(weapForm->formID))
			return;

		uint16_t ammoCapacity = CurrentAmmoCapacity;

		// 끝없는 무기의 경우 끝없는 무기 기본 탄환 수량을 이용
		if (GetAmmoType(weapInst, ammoCapacity) & AmmoType::kNeverEnding)
			ammoCapacity = iNeverEndingCapacity;

		// 현재 탄환의 총 보유수량 조회
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		uint32_t totalAmmoCount = GetInventoryItemCount(player, weapInst->ammo);

		// 탄약 추가
		if (totalAmmoCount < static_cast<uint32_t>(ammoCapacity) * iMinAmmoCapacityMult) {
			uint32_t diff = ammoCapacity * iMinAmmoCapacityMult - totalAmmoCount;
			Utils::AddItem(player, weapInst->ammo, diff, true);
		}
	}

	bool IsInfiniteThrowable(RE::TESForm* weapForm) {
		if (!weapForm)
			return false;

		// 투척무기 무한이 아닌 경우 유한
		if (!bUseInfiniteThrowableWeapon)
			return false;

		// 제외무기에 포함된 경우 유한
		if (IsExcludedWeapon(weapForm->formID))
			return false;

		// 화이트리스트 모드이고 화이트 리스트 무기가 아닌 경우 유한
		if (bUseWhiteListMode && !IsIncludedWeapon(weapForm->formID))
			return false;

		return true;
	}

	bool IsNeverEndingWeapon(RE::TESForm* weapForm, RE::TESObjectWEAP::InstanceData* weapInst) {
		if (!weapForm || !weapInst || !weapInst->ammo)
			return false;

		// 탄약무한이 아닌 경우 유한
		if (!bUseInfiniteAmmo)
			return false;

		// 제외무기에 포함된 경우 유한
		if (IsExcludedWeapon(weapForm->formID))
			return false;

		// 화이트리스트 모드이고 화이트 리스트 무기가 아닌 경우 유한
		if (bUseWhiteListMode && !IsIncludedWeapon(weapForm->formID))
			return false;

		return GetAmmoType(weapInst, CurrentAmmoCapacity) & AmmoType::kNeverEnding;
	}
}
