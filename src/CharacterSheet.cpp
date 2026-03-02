#include "CharacterSheet.h"
#include "Scaleform.h"
#include "Utility.h"
#include "editorID.hpp"
#include "CustomSkills.h"

struct StandingStoneInfo {
    std::string name;
    std::string descriptionEditorId;
};

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
        assert(a_params.GetArgCount() == 3);

        auto* rankC = a_params[0].GetString();
        auto* facC = a_params[1].GetString();
        bool rankOnlyC = a_params[2].GetBool();

        g_savedTitleRank = rankC ? rankC : "";
        g_savedTitleFaction = facC ? facC : "";
        g_savedTitleRankOnly = rankOnlyC;
        g_hasSavedTitle = true;
    }

    void CharacterSheet::SetGenericData(RE::Actor* target, RE::GPtr<RE::IMenu> menu) {
        // GENERIC DATA
        int level = target->GetLevel();
        auto name = target->GetName();
        RE::BSString raceDesc;
        const char* race;
        if (target->IsPlayerRef()) {
            // Have to use GetRaceData instead of GetRace, because GetRace will return
            // "Werewolf" in beast form while GetRaceData will always return the player's
            // original race
            auto player = RE::PlayerCharacter::GetSingleton();
            race = player->GetRaceData().charGenRace->GetName();
            player->GetRaceData().charGenRace->GetDescription(raceDesc, player->GetRaceData().charGenRace);
        } else {
            race = target->GetRace()->GetName();
            target->GetRace()->GetDescription(raceDesc, target->GetRace());
        }
        auto height = target->GetHeight();
        logger::trace("Character info:\n---Name: {}\n---Race: {}\n{}\n---Level: {}\n---Height: {}", name, race,
                      raceDesc.c_str(), level, height);

        // BIRTHSIGN
        auto activeEffects = target->AsMagicTarget()->GetActiveEffectList();
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
                return spell && target->HasSpell(spell);
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
    }

    void CharacterSheet::SetAttributes(RE::Actor* target, RE::GPtr<RE::IMenu> menu) {
        // ATTRIBUTES
        std::array<RE::GFxValue, 9> attributesData;
        // Health
        float permanentHealth = target->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kHealth);
        float baseHealth = target->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kHealth);
        float temporaryHealth =
            target->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kHealth);
        float maxHealth = permanentHealth + temporaryHealth;
        float health = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
        // Magicka
        float permanentMagicka = target->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kMagicka);
        float baseMagicka = target->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kMagicka);
        float temporaryMagicka =
            target->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kMagicka);
        float maxMagicka = permanentMagicka + temporaryMagicka;
        float magicka = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka);
        // Stamina
        float permanentStamina = target->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina);
        float baseStamina = target->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kStamina);
        float temporaryStamina =
            target->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kStamina);
        float maxStamina = permanentStamina + temporaryStamina;
        float stamina = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);

        logger::trace("Attributes:\n---Health: {}/{} ({})\n---Magicka: {}/{} ({})\n---Stamina: {}/{} ({})", health,
                     maxHealth, maxHealth - baseHealth, magicka, maxMagicka, maxMagicka - baseMagicka, stamina,
                     maxStamina, maxStamina - baseStamina);

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

        // STATS
        std::array<RE::GFxValue, 12> statsData;
        float healRate = (target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealRate) / 100) * maxHealth;
        float magickaRate =
            (target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagickaRate) / 100) * maxMagicka;
        float staminaRate =
            (target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStaminaRate) / 100) * maxStamina;
        float speedMult = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult);
        // UESP regarding WeaponSpeedMutl AV: "This is an odd modifier because the default is 0 and
        // yet it is a multiplier, meaning 1 = 100%, 0.5 = 50%, 2 = 200% but 0 = also 100%"
        float weaponSpeedMultRaw = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kWeaponSpeedMult);
        float weaponSpeedMult = weaponSpeedMultRaw == 0 ? 100 : weaponSpeedMultRaw * 100;

        float critChance = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCriticalChance);
        float poisonResist = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kPoisonResist);
        float magicResist = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistMagic);
        float fireResist = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistFire);
        float frostResist = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistFrost);
        float shockResist = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistShock);
        float diseaseResist = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kResistDisease);
        logger::trace(
            "Heal rate: {}\nMagicka rate: {}\nStamina rate: {}\nSpeed mult: {}\nWeapon speed mult: {}\nCritical hit "
            "chance: "
            "{}\nPoison resist: {}\nMagic resist: {}\nFire resist: {}\nFrost resist: {}\nShock resist: {}\nDisease "
            "resist:{}",
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
    }

    void CharacterSheet::SetFactions(RE::Actor* target, RE::GPtr<RE::IMenu> menu) {
        RE::GFxValue factionsArray;
        menu->uiMovie->CreateArray(&factionsArray);

        if (!target->IsPlayerRef()) {
            std::array<RE::GFxValue, 1> factionsData;
            factionsData[0] = factionsArray;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetFactions", nullptr, factionsData.data(),
                                  factionsData.size());
            return;
        }

        auto playerGender = target->GetActorBase()->GetSex();

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

        if (g_hasSavedTitle) {
            std::array<RE::GFxValue, 3> args;
            args[0] = g_savedTitleRank.c_str();
            args[1] = g_savedTitleFaction.c_str();
            args[2] = g_savedTitleRankOnly;
            menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetPlayerTitle", nullptr, args.data(), args.size());
        }
    }

    void CharacterSheet::SetSkills(RE::Actor* target, RE::GPtr<RE::IMenu> menu) {
        // SKILLS
        RE::GFxValue skillsArray;
        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* playerSkills = player->GetInfoRuntimeData().skills;
        auto* playerSkillsData = (playerSkills && playerSkills->data) ? playerSkills->data : nullptr;

        auto avList = RE::ActorValueList::GetSingleton();
        if (!avList) {
            return;
        }
        std::unordered_map<std::string, float> skillLevels;  // used for class extrapolation

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
                float value = target->AsActorValueOwner()->GetActorValue(av);
                // XP
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
                auto key = GetActorValueKey(av);  // used for class extrapolation
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

        if (IsPluginLoaded("Constellations") && target->IsPlayerRef()) {
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

        if (IsPluginLoaded("Firmament") && target->IsPlayerRef()) {
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
            skillLevels["horseman"] = horsemanValue;        // used for class extrapolation
            skillLevels["exploration"] = explorationValue;  // used for class extrapolation
            skillLevels["philosophy"] = philosophyValue;    // used for class extrapolation
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
    }

    void CharacterSheet::SetMiscValues(RE::Actor* target, RE::GPtr<RE::IMenu> menu) {
        std::array<RE::GFxValue, 5> miscData;

        int playerGold = GetPlayerGold();
        int armorRating = target->CalcArmorRating();
        float playerWeight = target->GetWeightInContainer();
        int carryWeight = target->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCarryWeight);
        int warmth = -1;
        if (GetSurvivalModeEnabled()) {
            warmth = target->GetWarmthRating();
        }
        logger::trace("Gold: {}\nArmor rating: {}\nCarry weight: {}/{}\nWarmth: {}", playerGold, armorRating,
                      playerWeight, carryWeight, warmth);
        miscData[0] = playerGold;
        miscData[1] = armorRating;
        miscData[2] = playerWeight;
        miscData[3] = carryWeight;
        miscData[4] = warmth;
        menu->uiMovie->Invoke("_root.CharacterSheet_mc.SetMiscData", nullptr, miscData.data(), miscData.size());
    }
}