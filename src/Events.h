#pragma once

#include "InfiniteAmmo.h"

namespace Events {
	struct TESEquipEvent {
		RE::TESObjectREFR* ref;	// 00
		uint32_t formId;		// 08
		uint32_t unk0C;			// 0C
		uint16_t unk10;			// 10
		bool isEquipping;		// 12
	};

	class ItemEquipEvent : public RE::BSTEventSink<TESEquipEvent> {
	public:
		RE::BSEventNotifyControl ProcessEvent(const TESEquipEvent& evn, RE::BSTEventSource<TESEquipEvent>*) {
			RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
			if (evn.ref == player && evn.isEquipping) {
				RE::EquippedItem* equippedItem = InfiniteAmmo::GetEquippedItemByFormID(evn.formId);
				if (equippedItem) {
					if (!InfiniteAmmo::IsThrowableWeapon(equippedItem->equipIndex)) {
						RE::TESObjectWEAP::InstanceData* weapInst = InfiniteAmmo::GetWeaponInstanceData(equippedItem->item.object, equippedItem->item.instanceData.get());
						CurrentAmmoCapacity = InfiniteAmmo::GetCurrentAmmoCapacity(weapInst);
						InfiniteAmmo::AddAmmo(equippedItem->item.object, weapInst);
					}
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void Install() {
		REL::Relocation<RE::BSTEventSource<TESEquipEvent>*> eventSource(REL::ID(485633));
		if (eventSource.get())
			eventSource->RegisterSink(new ItemEquipEvent());
	}
}
