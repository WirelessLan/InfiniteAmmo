#include "Utils.h"

namespace Utils {
	struct CALCED_OBJECT {
		RE::TESForm* item;		// 00
		std::uint64_t	unk08;		// 08
		std::uint32_t	count;		// 10
		std::uint32_t	unk14;		// 14
		std::uint64_t	unk18;		// 18
		std::uint64_t	unk20;		// 20
		float			unk28;		// 28
		std::uint32_t	unk2C;		// 2C
		std::uint64_t	unk30[(0x580 - 0x30) >> 3];	// 30
	};

	void AddCalcedObjectsToInventory(RE::TESObjectREFR* a_refr, RE::BSScrapArray<CALCED_OBJECT>& a_calcedObjs, bool a_isSilent, RE::TESForm* a_arg4) {
		using func_t = void (*)(RE::TESObjectREFR*, RE::BSScrapArray<CALCED_OBJECT>&, bool, RE::TESForm*);
		REL::Relocation<func_t> func(REL::ID(137743));
		func(a_refr, a_calcedObjs, a_isSilent, a_arg4);
	}

	void AddItem(RE::TESObjectREFR* a_refr, RE::TESForm* a_item, std::uint32_t a_count, bool a_isSilent) {
		if (!a_refr || !a_item || !a_count)
			return;

		RE::BSScrapArray<CALCED_OBJECT> calcedObjs;

		CALCED_OBJECT calced_obj{ 0 };
		calced_obj.item = a_item;
		calced_obj.count = a_count;
		calced_obj.unk28 = 1.0f;

		calcedObjs.push_back(calced_obj);

		AddCalcedObjectsToInventory(a_refr, calcedObjs, a_isSilent, nullptr);
	}

	std::string Trim(const std::string& a_str) {
		std::string trimmed_str = a_str;

		trimmed_str.erase(trimmed_str.begin(), std::find_if(trimmed_str.begin(), trimmed_str.end(), [](int ch) {
			return !std::isspace(ch);
		}));

		trimmed_str.erase(std::find_if(trimmed_str.rbegin(), trimmed_str.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), trimmed_str.end());

		return trimmed_str;
	}

	RE::TESForm* GetFormFromIdentifier(const std::string& a_pluginName, const std::uint32_t a_formId) {
		RE::TESDataHandler* g_dataHandler = RE::TESDataHandler::GetSingleton();
		if (!g_dataHandler)
			return nullptr;

		return g_dataHandler->LookupForm(a_formId, a_pluginName);
	}

	RE::TESForm* GetFormFromIdentifier(const std::string& a_pluginName, const std::string& a_formIdStr) {
		std::uint32_t formID = 0;
		try {
			formID = std::stoul(a_formIdStr, nullptr, 16) & 0xFFFFFF;
		}
		catch (...) {
			return nullptr;
		}

		return GetFormFromIdentifier(a_pluginName, formID);
	}
}
