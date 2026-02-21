#ifndef CHARACTER_SHEET_H
#define CHARACTER_SHEET_H

#include "PCH.h"
#include "RE/G/GFxMovieDef.h"
#include "RE/G/GFxValue.h"
#include "RE/G/GPtr.h"
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "Utility.h"

namespace Scaleform {

    class CharacterSheet : RE::IMenu {
    public:
        static constexpr const char* MENU_PATH = "charactersheet";
        static constexpr const char* MENU_NAME = "CharacterSheet";

        CharacterSheet();

        static void Register();
        static void Show();
        static void Hide();

        static constexpr std::string_view Name();

        virtual void Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) override;

        static RE::stl::owner<RE::IMenu*> Creator() { return new CharacterSheet(); }

    private:
        static void PlaySound(const RE::FxDelegateArgs& a_params);
        static void CloseMenu(const RE::FxDelegateArgs& a_params);
        static void OpenSkillsMenu(const RE::FxDelegateArgs& a_params);
        static void SaveFactionTitle(const RE::FxDelegateArgs& a_params);
    };

    constexpr std::string_view CharacterSheet::Name() { return CharacterSheet::MENU_NAME; }

}

#endif  // CHARACTER_SHEET_H
