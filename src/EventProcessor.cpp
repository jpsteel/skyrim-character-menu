#include "EventProcessor.h"
#include "Utility.h"
#include "Scaleform.h"
#include "CharacterSheet.h"
#include "Classes.h"
#include "editorID.hpp"
#include "APIManager.h"
#include "CustomSkills.h"

SKSE::PluginHandle pluginHandle = SKSE::kInvalidPluginHandle;

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    if (event->opening && event->menuName == Scaleform::CharacterSheet::MENU_NAME) {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto ui = RE::UI::GetSingleton();
        if (auto menu = ui->GetMenu(Scaleform::CharacterSheet::MENU_NAME)) {
            /*RE::Actor* target;
            auto* processLists = RE::ProcessLists::GetSingleton();
            if (!processLists) return RE::BSEventNotifyControl::kContinue;

            auto* followerFaction = RE::TESForm::LookupByEditorID<RE::TESFaction>("CurrentFollowerFaction");

            if (!followerFaction) return RE::BSEventNotifyControl::kContinue;

            for (auto handle : processLists->highActorHandles) {
                auto actor = handle.get().get();
                if (!actor || actor->IsPlayerRef()) continue;

                if (actor->IsInFaction(followerFaction)) {
                    logger::info("Follower found: {}", actor->GetName());
                    target = actor;
                    break;
                }
            }*/
            auto* target = RE::PlayerCharacter::GetSingleton();

            Scaleform::CharacterSheet::SetGenericData(target, menu);
            Scaleform::CharacterSheet::SetAttributes(target, menu);
            Scaleform::CharacterSheet::SetFactions(target, menu);
            Scaleform::CharacterSheet::SetSkills(target, menu);
            Scaleform::CharacterSheet::SetMiscValues(target, menu);

            RE::GViewport vp{};
            menu->uiMovie->GetViewport(&vp);

            if (vp.height > 0) {
                const float aspect = static_cast<float>(vp.width) / static_cast<float>(vp.height);
                g_isUltraWide = aspect > 1.95f;
            } else {
                g_isUltraWide = false;
            }

            RE::GFxValue arg;
            arg.SetBoolean(g_isUltraWide);

            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetWidescreen", nullptr, &arg, 1);

            logger::trace("Setting gamepad.");
            std::array<RE::GFxValue, 9> gamepad;
            gamepad[0] = RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled();
            gamepad[1] = detailsKey;
            gamepad[2] = actionKey;
            gamepad[3] = navLeftKey;
            gamepad[4] = navRightKey;
            gamepad[5] = detailsGamepadKey;
            gamepad[6] = actionGamepadKey;
            gamepad[7] = navLeftGamepadKey;
            gamepad[8] = navRightGamepadKey;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetGamepad", nullptr, gamepad.data(), gamepad.size());
            
            //Blur effect
            if (enableBlur == 1) {
                logger::info("Setting imagespace.");
                auto* imageSpace = RE::TESForm::LookupByEditorID<RE::TESImageSpaceModifier>("CharSheetImod");
                if (imageSpace) {
                    RE::ImageSpaceModifierInstanceForm::Trigger(imageSpace, 1.0f, nullptr);
                }
            }

            //HideWorld();

            //Camera work
            logger::trace("Setting camera.");
            auto camera = RE::PlayerCamera::GetSingleton();
            if (!camera) {
                return RE::BSEventNotifyControl::kContinue;
            }
            // SmoothCam compatibility
            logger::trace("Setting SmoothCam compatibility.");
            if (g_SmoothCam && g_SmoothCam->IsCameraEnabled()) {
                const auto result = g_SmoothCam->RequestCameraControl(pluginHandle);
                if (result == SmoothCamAPI::APIResult::OK || result == SmoothCamAPI::APIResult::AlreadyGiven) {
                    g_SmoothCam->RequestInterpolatorUpdates(pluginHandle, true);
                }
            }
            fixCameraZoom = true;

            logger::trace("Rotating camera.");
            RotateCamera(target);
            logger::trace("Toggling player controls.");
            TogglePlayerControls(false);
        }
    } 
    else if (!event->opening && event->menuName == Scaleform::CharacterSheet::MENU_NAME) {
        // Blur effect
        if (enableBlur == 1) {
            auto* imageSpace = RE::TESForm::LookupByEditorID<RE::TESImageSpaceModifier>("CharSheetImod");
            if (imageSpace) {
                RE::ImageSpaceModifierInstanceForm::Stop(imageSpace);
            }
        }

        //ShowWorld();

        // SmoothCam compatibility
        if (g_SmoothCam && g_SmoothCam->IsCameraEnabled()) {
            g_SmoothCam->ReleaseCameraControl(pluginHandle);
        }
        ResetCamera();
        TogglePlayerControls(true);
    }
    else if (!event->opening && event->menuName == RE::StatsMenu::MENU_NAME) {
        auto* q = RE::UIMessageQueue::GetSingleton();
        if (!q) {
            return RE::BSEventNotifyControl::kContinue;
        }

        q->AddMessage(RE::HUDMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }
    
    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(RE::InputEvent* const* eventPtr,
                                                      RE::BSTEventSource<RE::InputEvent*>*) {
    if (!eventPtr || !*eventPtr || !RE::Main::GetSingleton()->gameActive) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* event = *eventPtr;
    if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
        auto* buttonEvent = event->AsButtonEvent();
        auto dxScanCode = buttonEvent->GetIDCode();
        auto userEvent = RE::UserEvents::GetSingleton();
        if (buttonEvent->IsDown()) {
            auto ui = RE::UI::GetSingleton();
            if (dxScanCode == menuHotkey && buttonEvent->IsDown()) {
                if (!ui->IsMenuOpen(Scaleform::CharacterSheet::MENU_NAME) && !ui->GameIsPaused() &&
                    !ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
                    Scaleform::CharacterSheet::Show();
                    return RE::BSEventNotifyControl::kContinue;
                } else if (ui->IsMenuOpen(Scaleform::CharacterSheet::MENU_NAME)) {
                    if (auto menu = ui->GetMenu(Scaleform::CharacterSheet::MENU_NAME)) {
                        menu->uiMovie->Invoke("_root.CharacterSheet_mc.CloseMenu", nullptr, nullptr, 0);
                        return RE::BSEventNotifyControl::kContinue;
                    }
                }
            }
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const SKSE::ModCallbackEvent* event,
                                                      RE::BSTEventSource<SKSE::ModCallbackEvent>* eventSource) {
    if (!event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    return RE::BSEventNotifyControl::kContinue;
}