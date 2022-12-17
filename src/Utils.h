#pragma once

namespace Utils {
	struct ADDITEM_DATA {
		struct Data {
			RE::TESForm* item;		// 00
			uint64_t	unk08;		// 08
			uint32_t	count;		// 10
			uint32_t	unk14;		// 14
			uint64_t	unk18;		// 18
			uint64_t	unk20;		// 20
			float		unk28;		// 28
			uint32_t	unk2C;		// 2C
			uint64_t	unk30[(0x580 - 0x30) >> 3];	// 30
		};

		RE::TESObjectREFR* ref;	// 00
		uint64_t unk08;			// 08
		bool silent;			// 10
	};

	void AddItem(RE::TESObjectREFR* refr, RE::TESForm* item, uint32_t count, bool isSilent) {
		if (!refr || !item || !count)
			return;

		using func_t = void (*)(ADDITEM_DATA*, ADDITEM_DATA::Data*);
		REL::Relocation<func_t> func(REL::ID(363239));

		ADDITEM_DATA addItemData = { refr, 0, isSilent };
		ADDITEM_DATA::Data itemData = { 0 };
		itemData.item = item;
		itemData.count = count;
		itemData.unk28 = 1.0f;

		func(&addItemData, &itemData);
	}

	void trim(std::string& s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	const RE::TESFile* LookupModByName(RE::TESDataHandler* dataHandler, const std::string& pluginName) {
		for (RE::TESFile* file : dataHandler->files) {
			if (pluginName.compare(file->filename) == 0)
				return file;
		}
		return nullptr;
	}

	enum RecordFlag	{
		kNone = 0,
		kESM = 1 << 0,
		kActive = 1 << 3,
		kLocalized = 1 << 7,
		kESL = 1 << 9
	};

	bool IsLight(const RE::TESFile* mod) { 
		return (mod->flags & RecordFlag::kESL) == RecordFlag::kESL;
	}

	uint32_t GetPartialIndex(const RE::TESFile* mod) {
		return !IsLight(mod) ? mod->compileIndex : (0xFE000 | mod->smallFileCompileIndex);
	}

	RE::TESForm* GetFormFromIdentifier(const std::string& pluginName, const uint32_t formId) {
		RE::TESDataHandler* g_dataHandler = RE::TESDataHandler::GetSingleton();
		if (!g_dataHandler)
			return nullptr;

		const RE::TESFile* mod = LookupModByName(g_dataHandler, pluginName);
		if (!mod || mod->compileIndex == -1)
			return nullptr;

		uint32_t actualFormId = formId;
		uint32_t pluginIndex = GetPartialIndex(mod);
		if (!IsLight(mod))
			actualFormId |= pluginIndex << 24;
		else
			actualFormId |= pluginIndex << 12;

		return RE::TESForm::GetFormByID(actualFormId);
	}

	RE::TESForm* GetFormFromIdentifier(const std::string& pluginName, const std::string& formIdStr) {
		uint32_t formID = std::stoul(formIdStr, nullptr, 16) & 0xFFFFFF;
		return GetFormFromIdentifier(pluginName, formID);
	}
}
