#pragma once

namespace Utils {
	void AddItem(RE::TESObjectREFR* a_refr, RE::TESForm* a_item, std::uint32_t a_count, bool a_isSilent);
	std::string Trim(const std::string& a_str);
	RE::TESForm* GetFormFromIdentifier(const std::string& a_pluginName, const std::uint32_t a_formId);
	RE::TESForm* GetFormFromIdentifier(const std::string& a_pluginName, const std::string& a_formIdStr);
}
