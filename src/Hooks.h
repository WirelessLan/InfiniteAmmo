#pragma once

#include "InfiniteAmmo.h"

namespace Hooks {
	using _UseAmmo = std::uint32_t(*)(RE::Actor* actor, const RE::BGSObjectInstanceT<RE::TESObjectWEAP>& a_weapon, RE::BGSEquipIndex a_equipIndex, std::uint32_t a_shotCount);
	REL::Relocation<uintptr_t> UseAmmo_Target(REL::ID(1400465), 0x780);
	_UseAmmo UseAmmo_Original;

	struct BSAnimationGraphEvent {
		RE::TESObjectREFR* refr;
		RE::BSFixedString name;
		RE::BSFixedString args;
	};

	using _PlayerAnimGraphEvent_ReceiveEvent = RE::BSEventNotifyControl(*)(void* arg1, BSAnimationGraphEvent* evn, void* dispatcher);
	REL::Relocation<uintptr_t> PlayerAnimGraphEvent_ReceiveEvent_Target(REL::ID(1542933), 0x8);
	_PlayerAnimGraphEvent_ReceiveEvent PlayerAnimGraphEvent_ReceiveEvent_Original;

	std::uint32_t UseAmmo_Hook(RE::Actor* actor, const RE::BGSObjectInstanceT<RE::TESObjectWEAP>& a_weapon, RE::BGSEquipIndex a_equipIndex, std::uint32_t a_shotCount) {
		RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
		if (actor == player) {
			if (InfiniteAmmo::IsThrowableWeapon(a_equipIndex)) {
				if (InfiniteAmmo::IsInfiniteThrowable(a_weapon.object))
					return InfiniteAmmo::GetInventoryItemCount(actor, a_weapon.object);
			}
			else {
				RE::TESObjectWEAP::InstanceData* weapInst = InfiniteAmmo::GetWeaponInstanceData(a_weapon.object, a_weapon.instanceData.get());
				if (InfiniteAmmo::IsNeverEndingWeapon(a_weapon.object, weapInst))
					return InfiniteAmmo::GetInventoryItemCount(actor, weapInst->ammo);
			}
		}
		return UseAmmo_Original(actor, a_weapon, a_equipIndex, a_shotCount);
	}

	RE::BSEventNotifyControl PlayerAnimGraphEvent_ReceiveEvent_Hook(void* arg1, BSAnimationGraphEvent* evn, void* dispatcher) {
		if (evn->name == "ReloadComplete") {
			RE::EquippedItem* equippedItem = InfiniteAmmo::GetEquippedItemByEquipIndex(InfiniteAmmo::EquipIndex::kPrimary);
			if (equippedItem)
				InfiniteAmmo::AddAmmo(equippedItem->item.object, InfiniteAmmo::GetWeaponInstanceData(equippedItem->item.object, equippedItem->item.instanceData.get()));
		}

		return PlayerAnimGraphEvent_ReceiveEvent_Original(arg1, evn, dispatcher);
	}

	void Install() {
		UseAmmo_Original = *(_UseAmmo*)(UseAmmo_Target.get());
		REL::safe_write(UseAmmo_Target.address(), (uintptr_t)UseAmmo_Hook);

		PlayerAnimGraphEvent_ReceiveEvent_Original = *(_PlayerAnimGraphEvent_ReceiveEvent*)(PlayerAnimGraphEvent_ReceiveEvent_Target.get());
		REL::safe_write(PlayerAnimGraphEvent_ReceiveEvent_Target.address(), (uintptr_t)PlayerAnimGraphEvent_ReceiveEvent_Hook);
	}
}
