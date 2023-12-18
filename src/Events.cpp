#include "Events.h"

#include "InfiniteAmmo.h"

namespace Events {
	struct TESEquipEvent {
		RE::TESObjectREFR* ref;	// 00
		std::uint32_t formId;	// 08
		std::uint32_t unk0C;	// 0C
		std::uint16_t unk10;	// 10
		bool isEquipping;		// 12
	};

	class ItemEquipEvent : public RE::BSTEventSink<TESEquipEvent> {
	public:
		RE::BSEventNotifyControl ProcessEvent(const TESEquipEvent& a_evn, RE::BSTEventSource<TESEquipEvent>*) {
			RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
			if (!player)
				return RE::BSEventNotifyControl::kContinue;

			if (a_evn.ref != player || !a_evn.isEquipping)
				return RE::BSEventNotifyControl::kContinue;

			RE::EquippedItem* equippedItem = InfiniteAmmo::GetEquippedItemByFormID(a_evn.formId);
			if (!equippedItem)
				return RE::BSEventNotifyControl::kContinue;

			if (InfiniteAmmo::IsThrowableWeapon(equippedItem->equipIndex))
				return RE::BSEventNotifyControl::kContinue;

			RE::TESObjectWEAP::InstanceData* weapInst = InfiniteAmmo::GetWeaponInstanceData(equippedItem->item.object, equippedItem->item.instanceData.get());
			InfiniteAmmo::g_currentAmmoCapacity = InfiniteAmmo::GetCurrentAmmoCapacity(weapInst);
			InfiniteAmmo::AddAmmo(equippedItem->item.object, weapInst);

			return RE::BSEventNotifyControl::kContinue;
		}
	};

	void Install() {
		REL::Relocation<RE::BSTEventSource<TESEquipEvent>*> eventSource(REL::ID(485633));
		if (eventSource.get())
			eventSource->RegisterSink(new ItemEquipEvent());
	}
}
