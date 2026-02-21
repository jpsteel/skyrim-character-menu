#include "EventProcessor.h"
#include "Utility.h"
#include "Scaleform.h"
#include "CharacterSheet.h"
#include "Classes.h"
#include "editorID.hpp"
#include "APIManager.h"
#include "CustomSkills.h"

SKSE::PluginHandle pluginHandle = SKSE::kInvalidPluginHandle;

std::vector<RE::ActorValue> skills = {
    RE::ActorValue::kAlchemy,    RE::ActorValue::kAlteration,  RE::ActorValue::kArchery,
    RE::ActorValue::kBlock,      RE::ActorValue::kConjuration, RE::ActorValue::kDestruction,
    RE::ActorValue::kEnchanting, RE::ActorValue::kHeavyArmor,  RE::ActorValue::kIllusion,
    RE::ActorValue::kLightArmor, RE::ActorValue::kLockpicking, RE::ActorValue::kOneHanded,
    RE::ActorValue::kPickpocket, RE::ActorValue::kRestoration, RE::ActorValue::kSmithing,
    RE::ActorValue::kSneak,      RE::ActorValue::kSpeech,      RE::ActorValue::kTwoHanded};

std::unordered_map<std::string, StandingStoneInfo> standingStoneEffects = {
    {"doomWarriorAbility", {"$Doomstone_Warrior", "doomWarriorMSG"}},
    {"doomMageAbility", {"$Doomstone_Mage", "doomMageMSG"}},
    {"doomThiefAbility", {"$Doomstone_Thief", "doomThiefMSG"}},
    {"doomLoverAbility", {"$Doomstone_Lover", "doomLoverMSG"}},
    {"doomLordAbility", {"$Doomstone_Lord", "doomLordMSG"}},
    {"doomLadyAbility", {"$Doomstone_Lady", "doomLadyMSG"}},
    {"doomSteedAbility", {"$Doomstone_Steed", "doomSteedMSG"}},
    {"doomApprenticeAbility", {"$Doomstone_Apprentice", "doomApprenticeMSG"}},
    {"doomAtronachAbility", {"$Doomstone_Atronach", "doomAtronachMSG"}},
    {"doomRitualAbility", {"$Doomstone_Ritual", "doomRitualMSG"}},
    {"doomShadowAbility", {"$Doomstone_Shadow", "doomShadowMSG"}},
    {"doomTowerAbility", {"$Doomstone_Tower", "doomTowerMSG"}},
    {"doomSerpentAbility", {"$Doomstone_Serpent", "doomSerpentMSG"}},
};

static bool TryMapAVToPlayerSkill(RE::ActorValue av, RE::PlayerCharacter::PlayerSkills::Data::Skill& out) {
    using Skill = RE::PlayerCharacter::PlayerSkills::Data::Skill;

    switch (av) {
        case RE::ActorValue::kOneHanded:
            out = Skill::kOneHanded;
            return true;
        case RE::ActorValue::kTwoHanded:
            out = Skill::kTwoHanded;
            return true;
        case RE::ActorValue::kArchery:
            out = Skill::kArchery;
            return true;
        case RE::ActorValue::kBlock:
            out = Skill::kBlock;
            return true;
        case RE::ActorValue::kSmithing:
            out = Skill::kSmithing;
            return true;
        case RE::ActorValue::kHeavyArmor:
            out = Skill::kHeavyArmor;
            return true;
        case RE::ActorValue::kLightArmor:
            out = Skill::kLightArmor;
            return true;
        case RE::ActorValue::kPickpocket:
            out = Skill::kPickpocket;
            return true;
        case RE::ActorValue::kLockpicking:
            out = Skill::kLockpicking;
            return true;
        case RE::ActorValue::kSneak:
            out = Skill::kSneak;
            return true;
        case RE::ActorValue::kAlchemy:
            out = Skill::kAlchemy;
            return true;
        case RE::ActorValue::kSpeech:
            out = Skill::kSpeech;
            return true;
        case RE::ActorValue::kAlteration:
            out = Skill::kAlteration;
            return true;
        case RE::ActorValue::kConjuration:
            out = Skill::kConjuration;
            return true;
        case RE::ActorValue::kDestruction:
            out = Skill::kDestruction;
            return true;
        case RE::ActorValue::kIllusion:
            out = Skill::kIllusion;
            return true;
        case RE::ActorValue::kRestoration:
            out = Skill::kRestoration;
            return true;
        case RE::ActorValue::kEnchanting:
            out = Skill::kEnchanting;
            return true;
        default:
            return false;
    }
}

RE::BSEventNotifyControl EventProcessor::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                      RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    if (event->opening && event->menuName == Scaleform::CharacterSheet::MENU_NAME) {
        auto player = RE::PlayerCharacter::GetSingleton();
        auto ui = RE::UI::GetSingleton();
        if (auto menu = ui->GetMenu(Scaleform::CharacterSheet::MENU_NAME)) {
            // GENERIC DATA
            int level = player->GetLevel();
            auto name = player->GetName();
            // Have to use GetRaceData instead of GetRace, because GetRace will return
            // "Werewolf" in beast form while GetRaceData will always return the player's
            // original race
            auto race = player->GetRaceData().charGenRace->GetName();
            RE::BSString raceDesc;
            player->GetRaceData().charGenRace->GetDescription(raceDesc, player->GetRaceData().charGenRace);
            auto height = player->GetHeight();
            logger::trace("Character info:\n---Name: {}\n---Race: {}\n{}\n---Level: {}\n---Height: {}", name, race,
                          raceDesc.c_str(), level, height);

            // BIRTHSIGN
            auto activeEffects = player->AsMagicTarget()->GetActiveEffectList();
            std::string constellation;
            RE::BGSMessage* constellationMessage;
            RE::BSString constellationDesc;

            if (activeEffects) {
                for (auto* effect : *activeEffects) {
                    if (!effect || !effect->spell) {
                        continue;
                    }

                    auto* spell = effect->spell;
                    std::string edid = clib_util::editorID::get_editorID(spell);
                    if (edid.empty()) {
                        continue;
                    }

                    auto it = standingStoneEffects.find(edid);
                    if (it != standingStoneEffects.end()) {
                        const auto& info = it->second;

                        logger::trace("Constellation (effect): {}", info.name);
                        constellation = info.name;
                        constellationMessage = RE::TESForm::LookupByEditorID<RE::BGSMessage>(info.descriptionEditorId);
                        constellationMessage->GetDescription(constellationDesc, constellationMessage);
                        std::string desc = constellationDesc.c_str();
                        desc = RemoveLastDotSentence(std::move(desc));
                        desc = CollapsePercent(std::move(desc));
                        constellationDesc = desc.c_str();
                        logger::trace("{}", constellationDesc.c_str());
                        break;
                    }
                }
            }

            // Fallback if not found: powers (Ritual/Shadow/Tower/Serpent)
            if (constellation.empty()) {
                auto hasStoneSpell = [&](const std::string& spellEdid) {
                    auto* spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(spellEdid);
                    return spell && player->HasSpell(spell);
                };

                for (const auto& [spellEdid, info] : standingStoneEffects) {
                    if (hasStoneSpell(spellEdid)) {
                        logger::trace("Constellation (spell): {}", info.name);
                        constellation = info.name;
                        constellationMessage = RE::TESForm::LookupByEditorID<RE::BGSMessage>(info.descriptionEditorId);
                        constellationMessage->GetDescription(constellationDesc, constellationMessage);
                        std::string desc = constellationDesc.c_str();
                        desc = RemoveLastDotSentence(std::move(desc));
                        desc = CollapsePercent(std::move(desc));
                        constellationDesc = desc.c_str();
                        logger::trace("{}", constellationDesc.c_str());
                        break;
                    }
                }
            }

            if (constellation.empty()) {
                constellation = "-";
                constellationDesc = "";
            }

            std::array<RE::GFxValue, 9> genericData;
            genericData[0] = name;
            genericData[1] = race;
            genericData[2] = level;
            genericData[3] = GetPlayerXPProgression();
            genericData[4] = GetInGameDate();
            genericData[5] = constellation;
            genericData[6] = raceDesc.c_str();
            genericData[7] = constellationDesc.c_str();
            genericData[8] = GetPlayerCondition();
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetGenericData", nullptr, genericData.data(),
                                  genericData.size());

            // ATTRIBUTES
            std::array<RE::GFxValue, 9> attributesData;
            // Health
            float permanentHealth = player->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kHealth);
            float baseHealth = player->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kHealth);
            float temporaryHealth =
                player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
            float maxHealth = permanentHealth + temporaryHealth;
            float health = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
            // Magicka
            float permanentMagicka = player->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kMagicka);
            float baseMagicka = player->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kMagicka);
            float temporaryMagicka =
                player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kMagicka);
            float maxMagicka = permanentMagicka + temporaryMagicka;
            float magicka = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka);
            // Stamina
            float permanentStamina = player->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina);
            float baseStamina = player->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kStamina);
            float temporaryStamina =
                player->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kStamina);
            float maxStamina = permanentStamina + temporaryStamina;
            float stamina = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
        
            logger::trace("Attributes:\n---Health: {}/{} ({})\n---Magicka: {}/{} ({})\n---Stamina: {}/{} ({})", health,
                         maxHealth, maxHealth - baseHealth, magicka, maxMagicka, maxMagicka - baseMagicka, stamina, maxStamina, maxStamina - baseStamina);

            attributesData[0] = health;
            attributesData[1] = maxHealth;
            attributesData[2] = maxHealth - baseHealth;
            attributesData[3] = magicka;
            attributesData[4] = maxMagicka;
            attributesData[5] = maxMagicka - baseMagicka;
            attributesData[6] = stamina;
            attributesData[7] = maxStamina;
            attributesData[8] = maxStamina - baseStamina;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetAttributesMeters", nullptr, attributesData.data(),
                                  attributesData.size());
            
            // FACTIONS
            auto playerGender = player->GetActorBase()->GetSex();

            RE::GFxValue factionsArray;
            menu->uiMovie->CreateArray(&factionsArray);

            for (const auto& def : GetFactionDefinitions()) {
                std::string rankTitle;
                bool rankOnly = false;
                if (!TryGetBestRankForFaction(def, playerGender, rankTitle, rankOnly)) {
                    continue;
                }

                RE::GFxValue factionObj;
                menu->uiMovie->CreateObject(&factionObj);

                factionObj.SetMember("factionName", def.name.c_str());
                factionObj.SetMember("id", def.id.c_str());
                factionObj.SetMember("rank", rankTitle.c_str());
                factionObj.SetMember("rankDisplayOnly", rankOnly);

                logger::info("Faction(JSON): {} ({}) rank={} (rankOnly={})", def.name, def.id, rankTitle, rankOnly);

                factionsArray.PushBack(factionObj);
            }

            std::array<RE::GFxValue, 1> factionsData;
            factionsData[0] = factionsArray;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetFactions", nullptr, factionsData.data(), factionsData.size());

            // SKILLS
            auto* playerSkills = player->GetInfoRuntimeData().skills;
            auto* playerSkillsData = (playerSkills && playerSkills->data) ? playerSkills->data : nullptr;

            auto avList = RE::ActorValueList::GetSingleton();
            if (!avList) {
                return RE::BSEventNotifyControl::kContinue;
            }
            std::unordered_map<std::string, float> skillLevels; //used for class extrapolation
            RE::GFxValue skillsArray;
            menu->uiMovie->CreateArray(&skillsArray);
            for (auto av : skills) {
                auto avInfo = avList->GetActorValue(av);
                if (avInfo) {
                    RE::GFxValue skill;
                    menu->uiMovie->CreateObject(&skill);
                    auto name = avInfo->GetFullName();
                    RE::BSString description;
                    avInfo->GetDescription(description, avInfo);
                    logger::trace("Description: {}", description.c_str());
                    float value = player->AsActorValueOwner()->GetActorValue(av);
                    //XP
                    int xpFrame = 200;

                    if (playerSkillsData) {
                        RE::PlayerCharacter::PlayerSkills::Data::Skill s;
                        if (TryMapAVToPlayerSkill(av, s)) {
                            const auto& sd = playerSkillsData->skills[s];
                            if (sd.levelThreshold > 0.0f) {
                                float progress = std::clamp(sd.xp / sd.levelThreshold, 0.0f, 1.0f);
                                xpFrame = 200 - static_cast<int>(std::round(progress * 200.0f));
                            }
                        }
                    }
                    auto key = GetActorValueKey(av); //used for class extrapolation
                    skillLevels[key] = value;         // used for class extrapolation
                    logger::trace("{} ({}): {}\nFrame: {}", name, key, value, xpFrame);
                    skill.SetMember("skillName", name);
                    skill.SetMember("level", value);
                    skill.SetMember("description", description.c_str());
                    skill.SetMember("key", key.c_str());
                    skill.SetMember("xpFrame", xpFrame);
                    skillsArray.PushBack(skill);
                }
            }

            if (IsPluginLoaded("Constellations")) {
                logger::info("Constellations plugin detected, loading additional classes.");

                // Hand to Hand
                RE::GFxValue handtohand;
                menu->uiMovie->CreateObject(&handtohand);
                float handtohandValue = Data::HandToHand::GetLevel();
                int handtohandRatio = Data::HandToHand::GetRatio();
                handtohand.SetMember("skillName", "$HandToHand_Name");
                handtohand.SetMember("level", handtohandValue);
                handtohand.SetMember("description", "$HandToHand_Description");
                handtohand.SetMember("key", "handToHand");
                handtohand.SetMember("xpFrame", handtohandRatio);
                logger::trace("Hand-to-hand: {}", handtohandValue);

                // Athletics
                RE::GFxValue athletics;
                menu->uiMovie->CreateObject(&athletics);
                float athleticsValue = Data::Athletics::GetLevel();
                int athleticsRatio = Data::Athletics::GetRatio();
                athletics.SetMember("skillName", "$Athletics_Name");
                athletics.SetMember("level", athleticsValue);
                athletics.SetMember("description", "$Athletics_Description");
                athletics.SetMember("key", "athletics");
                athletics.SetMember("xpFrame", athleticsRatio);
                logger::trace("Athletics: {}", athleticsValue);

                // Sorcery
                RE::GFxValue sorcery;
                menu->uiMovie->CreateObject(&sorcery);
                float sorceryValue = Data::Sorcery::GetLevel();
                int sorceryRatio = Data::Sorcery::GetRatio();
                sorcery.SetMember("skillName", "$Sorcery_Name");
                sorcery.SetMember("level", sorceryValue);
                sorcery.SetMember("description", "$Sorcery_Description");
                sorcery.SetMember("key", "sorcery");
                sorcery.SetMember("xpFrame", sorceryRatio);
                logger::trace("Sorcery: {}", sorceryValue);

                
                skillsArray.PushBack(handtohand);
                skillsArray.PushBack(athletics);
                skillsArray.PushBack(sorcery);
                skillLevels["handToHand"] = handtohandValue;  // used for class extrapolation
                skillLevels["athletics"] = athleticsValue;    // used for class extrapolation
                skillLevels["sorcery"] = sorceryValue;        // used for class extrapolation
            }
            
            if (IsPluginLoaded("Firmament")) {
                logger::info("Firmament plugin detected, loading additional classes.");

                // Horseman
                RE::GFxValue horseman;
                menu->uiMovie->CreateObject(&horseman);
                float horsemanValue = Data::Horseman::GetLevel();
                int horsemanRatio = Data::Horseman::GetRatio();
                horseman.SetMember("skillName", "$Horseman_Name");
                horseman.SetMember("level", horsemanValue);
                horseman.SetMember("description", "$Horseman_Description");
                horseman.SetMember("key", "horseman");
                horseman.SetMember("xpFrame", horsemanRatio);
                logger::trace("Horseman: {}", horsemanValue);

                // Exploration
                RE::GFxValue exploration;
                menu->uiMovie->CreateObject(&exploration);
                float explorationValue = Data::Exploration::GetLevel();
                int explorationRatio = Data::Exploration::GetRatio();
                exploration.SetMember("skillName", "$Exploration_Name");
                exploration.SetMember("level", explorationValue);
                exploration.SetMember("description", "$Exploration_Description");
                exploration.SetMember("key", "exploration");
                exploration.SetMember("xpFrame", explorationRatio);
                logger::trace("Exploration: {}", explorationValue);

                // Philosophy
                RE::GFxValue philosophy;
                menu->uiMovie->CreateObject(&philosophy);
                float philosophyValue = Data::Philosophy::GetLevel();
                int philosophyRatio = Data::Philosophy::GetRatio();
                philosophy.SetMember("skillName", "$Philosophy_Name");
                philosophy.SetMember("level", philosophyValue);
                philosophy.SetMember("description", "$Philosophy_Description");
                philosophy.SetMember("key", "philosophy");
                philosophy.SetMember("xpFrame", philosophyRatio);
                logger::trace("Philosophy: {}", philosophyValue);

                skillsArray.PushBack(horseman);
                skillsArray.PushBack(exploration);
                skillsArray.PushBack(philosophy);
                skillLevels["horseman"] = horsemanValue;    // used for class extrapolation
                skillLevels["exploration"] = explorationValue;  // used for class extrapolation
                skillLevels["philosophy"] = philosophyValue;     // used for class extrapolation
            }

            
            const TESClass* match;
            if (IsPluginLoaded("Firmament")) {
                match = GetBestMatchingClass(classicClassesFirmament, skillLevels);
            } else if (IsPluginLoaded("Constellations")) {
                match = GetBestMatchingClass(classicClassesConstellations, skillLevels);
            } else {
                match = GetBestMatchingClass(classicClasses, skillLevels);
            }

            std::array<RE::GFxValue, 4> skillsData;
            skillsData[0] = skillsArray;
            skillsData[1] = match->name;
            skillsData[2] = match->specialization;
            skillsData[3] = match->description;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetSkills", nullptr, skillsData.data(), skillsData.size());

            // MISC VALUES
            std::array<RE::GFxValue, 5> miscData;

            int playerGold = GetPlayerGold();
            int armorRating = player->CalcArmorRating();
            float playerWeight = player->GetWeightInContainer();
            int carryWeight = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCarryWeight);
            int warmth = -1;
            if (GetSurvivalModeEnabled()) {
                warmth = player->GetWarmthRating();
            }
            logger::trace("Gold: {}\nArmor rating: {}\nCarry weight: {}/{}\nWarmth: {}", playerGold, armorRating,
                         playerWeight, carryWeight, warmth);
            miscData[0] = playerGold;
            miscData[1] = armorRating;
            miscData[2] = playerWeight;
            miscData[3] = carryWeight;
            miscData[4] = warmth;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetMiscData", nullptr, miscData.data(), miscData.size());

            //STATS
            std::array<RE::GFxValue, 12> statsData;
            float healRate = (player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealRate) / 100) * maxHealth;
            float magickaRate = (player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagickaRate) / 100) * maxMagicka;
            float staminaRate = (player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStaminaRate) / 100) * maxStamina;
            float speedMult = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult);
            // UESP regarding WeaponSpeedMutl AV: "This is an odd modifier because the default is 0 and 
            // yet it is a multiplier, meaning 1 = 100%, 0.5 = 50%, 2 = 200% but 0 = also 100%"
            float weaponSpeedMultRaw = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kWeaponSpeedMult);
            float weaponSpeedMult = weaponSpeedMultRaw == 0 ? 100 : weaponSpeedMultRaw * 100;

            float critChance = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCriticalChance);
            float poisonResist = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kPoisonResist);
            float magicResist = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistMagic);
            float fireResist = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistFire);
            float frostResist = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistFrost);
            float shockResist = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistShock);
            float diseaseResist = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistDisease);
            logger::trace(
                "Heal rate: {}\nMagicka rate: {}\nStamina rate: {}\nSpeed mult: {}\nWeapon speed mult: {}\nCritical hit chance: "
                "{}\nPoison resist: {}\nMagic resist: {}\nFire resist: {}\nFrost resist: {}\nShock resist: {}\nDisease resist:{}",
                healRate, magickaRate, staminaRate, speedMult, weaponSpeedMult, critChance, poisonResist, magicResist, 
                fireResist, frostResist, shockResist, diseaseResist);
            
            statsData[0] = healRate;
            statsData[1] = magickaRate;
            statsData[2] = staminaRate;
            statsData[3] = speedMult;
            statsData[4] = weaponSpeedMult;
            statsData[5] = critChance;
            statsData[6] = poisonResist;
            statsData[7] = magicResist;
            statsData[8] = fireResist;
            statsData[9] = frostResist;
            statsData[10] = shockResist;
            statsData[11] = diseaseResist;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetStats", nullptr, statsData.data(), statsData.size());

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
            std::array<RE::GFxValue, 1> gamepad;
            gamepad[0] = RE::BSInputDeviceManager::GetSingleton()->IsGamepadEnabled();
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetGamepad", nullptr, gamepad.data(), gamepad.size());
            
            //Blur effect
            if (enableBlur == 1) {
                logger::trace("Setting imagespace.");
                auto* imageSpace = RE::TESForm::LookupByEditorID<RE::TESImageSpaceModifier>("CharSheetImod");
                if (imageSpace) {
                    RE::ImageSpaceModifierInstanceForm::Trigger(imageSpace, 1.0f, nullptr);
                }
            }

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
            RotateCamera();
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