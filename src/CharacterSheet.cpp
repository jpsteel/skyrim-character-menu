#include "CharacterSheet.h"
#include "Scaleform.h"
#include "Utility.h"

namespace Scaleform {
    CharacterSheet::CharacterSheet() {
        auto scaleformManager = RE::BSScaleformManager::GetSingleton();
        scaleformManager->LoadMovieEx(this, MENU_PATH, [this](RE::GFxMovieDef* a_def) {
            using StateType = RE::GFxState::StateType;

            fxDelegate.reset(new RE::FxDelegate());
            fxDelegate->RegisterHandler(this);
            a_def->SetState(StateType::kExternalInterface, fxDelegate.get());
            fxDelegate->Release();

            auto logger = new Logger<CharacterSheet>();
            a_def->SetState(StateType::kLog, logger);
            logger->Release();
        });

        inputContext = Context::kMenuMode;
        depthPriority = 3;
        menuFlags.set(RE::UI_MENU_FLAGS::kDisablePauseMenu,
                      RE::UI_MENU_FLAGS::kUsesBlurredBackground, RE::UI_MENU_FLAGS::kModal,
                      RE::UI_MENU_FLAGS::kTopmostRenderedMenu, RE::UI_MENU_FLAGS::kUsesMenuContext,
                      RE::UI_MENU_FLAGS::kRequiresUpdate, RE::UI_MENU_FLAGS::kUpdateUsesCursor);

        if (!RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled()) {
            menuFlags |= RE::UI_MENU_FLAGS::kUsesCursor;
        }
    }

    void CharacterSheet::Register() {
        auto ui = RE::UI::GetSingleton();
        if (ui) {
            ui->Register(CharacterSheet::MENU_NAME, Creator);
            logger::debug("Registered {}", CharacterSheet::MENU_NAME);
        }
    }

    void CharacterSheet::Show() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(CharacterSheet::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            //RE::UIBlurManager::GetSingleton()->IncrementBlurCount();
        }
    }
    
    void CharacterSheet::Hide() {
        auto uiMessageQueue = RE::UIMessageQueue::GetSingleton();
        if (uiMessageQueue) {
            uiMessageQueue->AddMessage(CharacterSheet::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            RE::PlaySound("UIJournalClose");
            //RE::UIBlurManager::GetSingleton()->DecrementBlurCount();
        }
    }

    void CharacterSheet::Accept(RE::FxDelegateHandler::CallbackProcessor* a_cbReg) {
        a_cbReg->Process("PlaySound", PlaySound);
        a_cbReg->Process("CloseMenu", CloseMenu);
        a_cbReg->Process("OpenSkillsMenu", OpenSkillsMenu);
        a_cbReg->Process("SaveFactionTitle", SaveFactionTitle);
    }

    void CharacterSheet::PlaySound(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 1);
        assert(a_params[0].IsString());

        RE::PlaySound(a_params[0].GetString());
    }

    void CharacterSheet::CloseMenu(const RE::FxDelegateArgs& a_params) {
        assert(a_params.GetArgCount() == 0);

        Hide();
    }

    void CharacterSheet::OpenSkillsMenu(const RE::FxDelegateArgs&) {
        auto* q = RE::UIMessageQueue::GetSingleton();
        if (!q) {
            return;
        }

        auto* fadeOut = RE::TESForm::LookupByEditorID<RE::TESImageSpaceModifier>("FadeToBlackImod");
        auto* fadeIn = RE::TESForm::LookupByEditorID<RE::TESImageSpaceModifier>("FadeToBlackBackImod");

        if (fadeIn) {
            RE::ImageSpaceModifierInstanceForm::Trigger(fadeIn, 1.0f, nullptr);
        }

        q->AddMessage(RE::StatsMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        q->AddMessage(CharacterSheet::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
        q->AddMessage(RE::HUDMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }

    void CharacterSheet::SaveFactionTitle(const RE::FxDelegateArgs& a_params) { 
        assert(a_params.GetArgCount() == 2);

        auto* rankC = a_params[0].GetString();
        auto* facC = a_params[1].GetString();

        g_savedTitleRank = rankC ? rankC : "";
        g_savedTitleFaction = facC ? facC : "";
        g_hasSavedTitle = true;
    }
}