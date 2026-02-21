#include "Utility.h"
#include "Classes.h"

#define SMOOTHCAM_API_COMMONLIB
#include "SmoothCamAPI.h"
#include "editorID.hpp"

#include <Windows.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;
constexpr auto MATH_PI = 3.14159265358979323846f;

static std::vector<FactionDef> g_factionDefs;

int menuHotkey;
int enableBlur;
bool g_isUltraWide = false;

bool forced3rdPerson;
bool rotatedPlayer = false;
bool fixCameraZoom;
bool shouldDisableAnimCam;
float playerAngleX;
float playerRotation;
float targetZoomOffset;
RE::NiPoint2 freeRotation;
RE::NiPoint2 g_freeRotation;
RE::TESCameraState* g_prevState = nullptr;
RE::NiPoint3 posOffsetExpected;
RE::NiPointer<RE::NiFloatInterpolator> radialBlurStrength;
float blurRadius;
RE::Setting* overShoulderCombatPosX;
float fOverShoulderCombatPosX;
RE::Setting* overShoulderCombatAddY;
float fOverShoulderCombatAddY;
RE::Setting* overShoulderCombatPosZ;
float fOverShoulderCombatPosZ;
RE::Setting* autoVanityModeDelay;
float fAutoVanityModeDelay;
RE::Setting* overShoulderPosX;
float fOverShoulderPosX;
RE::Setting* overShoulderPosZ;
float fOverShoulderPosZ;
RE::Setting* vanityModeMinDist;
float fVanityModeMinDist;
RE::Setting* vanityModeMaxDist;
float fVanityModeMaxDist;
RE::Setting* mouseWheelZoomSpeed;
float fMouseWheelZoomSpeed;
RE::Setting* togglePOVDelay;
float fTogglePOVDelay;
float worldFOV;
bool playerHeadtrackingEnabled;
float fNewOverShoulderCombatPosX;
float fNewOverShoulderCombatAddY;
float fNewOverShoulderCombatPosZ;
float timescale;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

float GetPlayerXPProgression() {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerXP = player->GetInfoRuntimeData().skills->data->xp;
    auto playerLevelThreshold = player->GetInfoRuntimeData().skills->data->levelThreshold;
    float levelProgression = round((playerXP / playerLevelThreshold) * 140); // LevelProgresssBar movieclip is made of 140 frames and not 100 for some reason...
    return levelProgression;
}

std::string GetInGameDate() {
    auto calendar = RE::Calendar::GetSingleton();
    char datetime[504];
    calendar->GetTimeDateString(datetime, 0x200u, 1);
    std::string dt = datetime;
    return dt;
}

int GetPlayerGold() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        logger::warn("Player not found.");
        return 0;
    }

    auto* goldForm = RE::TESForm::LookupByEditorID("Gold001");
    if (!goldForm) {
        logger::warn("Gold form not found.");
        return 0;
    }

    auto* goldObject = goldForm->As<RE::TESBoundObject>();
    if (!goldObject) {
        logger::warn("Gold form is not a bound object.");
        return 0;
    }

    const auto& invCounts = player->GetInventoryCounts();
    auto it = invCounts.find(goldObject);
    int totalGold = (it != invCounts.end()) ? it->second : 0;

    logger::trace("Player has {} gold.", totalGold);
    return totalGold;
}

bool HasPlayerSpellByEDID(RE::PlayerCharacter* player, const char* edid) {
    if (!player || !edid || !*edid) {
        return false;
    }
    auto* spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(edid);
    return spell && player->HasSpell(spell);
}

bool IsPlayerVampire(RE::PlayerCharacter* player) {
    if (!player) {
        return false;
    }

    auto* vampKW = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("Vampire");
    return vampKW && player->HasKeyword(vampKW);
}

const char* GetPlayerCondition() {
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return "";
    }

    if (HasPlayerSpellByEDID(player, "BOS_WerebearDisease")) {
        return "werebear";
    }
    if (HasPlayerSpellByEDID(player, "WerewolfChange")) {
        return "werewolf";
    }
    if (IsPlayerVampire(player)) {
        return "vampire";
    }

    return "";
}

bool IsPluginLoaded(const std::string& pluginName) {
    std::string dllName = pluginName + ".dll";

    HMODULE module = GetModuleHandleA(dllName.c_str());
    if (module) {
        return true;
    } else {
        return false;
    }
}

void TogglePlayerControls(bool enable) {
    auto playerControls = RE::PlayerControls::GetSingleton();
    if (playerControls) {
        playerControls->lookHandler->inputEventHandlingEnabled = enable;
        playerControls->attackBlockHandler->inputEventHandlingEnabled = enable;
        playerControls->autoMoveHandler->inputEventHandlingEnabled = enable;
        playerControls->shoutHandler->inputEventHandlingEnabled = enable;
        playerControls->readyWeaponHandler->inputEventHandlingEnabled = enable;
        playerControls->activateHandler->inputEventHandlingEnabled = enable;
        playerControls->sneakHandler->inputEventHandlingEnabled = enable;
        playerControls->movementHandler->inputEventHandlingEnabled = enable;
        playerControls->jumpHandler->inputEventHandlingEnabled = enable;
        playerControls->toggleRunHandler->inputEventHandlingEnabled = enable;
        playerControls->togglePOVHandler->inputEventHandlingEnabled = enable;
    }
}

bool GetSurvivalModeEnabled() {
    static const std::string survivalEnabled = "Survival_ModeEnabled";
    if (auto form = RE::TESForm::LookupByEditorID(survivalEnabled)) {
        auto formID = form->GetFormID();
        auto survivalModeGlobal = RE::TESForm::LookupByID<RE::TESGlobal>(formID);
        auto value = survivalModeGlobal->value;
        if (value == 1.0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool IsPlayersMount(const RE::Actor* actor) {
    if (!actor) {
        return false;
    }

    RE::NiPointer<RE::Actor> playerMount;
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (player->GetMount(playerMount)) {
        return playerMount.get() == actor;
    }

    return false;
}

void LoadDataFromINI() {
    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error rc = ini.LoadFile(INI_FILE_PATH.c_str());
    if (rc < 0) {
        logger::error("Failed to load INI file: {}", INI_FILE_PATH);
        return;
    }

    const char* keycodeStr = ini.GetValue("General", "iKeycode", "49"); // N
    const char* enable_blur = ini.GetValue("General", "iEnableBlur", "1");
    menuHotkey = std::stoi(keycodeStr);
    enableBlur = std::stoi(enable_blur);
    logger::debug("Loaded keycode: {}", keycodeStr);
    logger::debug("Loaded blur enabled: {}", enable_blur);
}

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
 // All credit goes to derickso/myztikrice for the following functions (https://github.com/derickso/ShowPlayerInMenus)
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void RotateCamera() {
    auto camera = RE::PlayerCamera::GetSingleton();
    auto ini = RE::INISettingCollection::GetSingleton();
    auto player = RE::PlayerCharacter::GetSingleton();
    bool isBeastForm = false;
    bool isVampireLord = false;

    auto thirdState = (RE::ThirdPersonState*)camera->cameraStates[RE::CameraState::kThirdPerson].get();

    if (player->GetRace() == RE::TESForm::LookupByEditorID<RE::TESRace>("WerewolfBeastRace") ||
        player->GetRace() == RE::TESForm::LookupByEditorID<RE::TESRace>("DLC2WerebearBeastRace")) {
        isBeastForm = true;
    } else if (player->GetRace() == RE::TESForm::LookupByEditorID<RE::TESRace>("DLC1VampireBeastRace")) {
        isVampireLord = true;
    }

    // collect original values for later
    playerAngleX = player->data.angle.x;
    playerRotation = player->data.angle.z;
    freeRotation = thirdState->freeRotation;
    g_prevState = camera->currentState.get();
    g_freeRotation = freeRotation;
    timescale = RE::TESForm::LookupByID<RE::TESGlobal>(0x3A)->value;
    posOffsetExpected = thirdState->posOffsetExpected;
    if (camera->IsInFirstPerson()) {
        forced3rdPerson = true;
    }
    camera->SetState(thirdState);
    // camera->UpdateThirdPerson(player->AsActorState()->IsWeaponDrawn());

    // set over the shoulder camera values for when player has weapon drawn and unpaused menu(s) in order to prevent
    // camera from snapping
    overShoulderCombatPosX = ini->GetSetting("fOverShoulderCombatPosX:Camera");
    overShoulderCombatAddY = ini->GetSetting("fOverShoulderCombatAddY:Camera");
    overShoulderCombatPosZ = ini->GetSetting("fOverShoulderCombatPosZ:Camera");
    autoVanityModeDelay = ini->GetSetting("fAutoVanityModeDelay:Camera");
    overShoulderPosX = ini->GetSetting("fOverShoulderPosX:Camera");
    overShoulderPosZ = ini->GetSetting("fOverShoulderPosZ:Camera");
    vanityModeMinDist = ini->GetSetting("fVanityModeMinDist:Camera");
    vanityModeMaxDist = ini->GetSetting("fVanityModeMaxDist:Camera");
    mouseWheelZoomSpeed = ini->GetSetting("fMouseWheelZoomSpeed:Camera");
    togglePOVDelay = ini->GetSetting("fTogglePOVDelay:Controls");
    fOverShoulderCombatPosX = overShoulderCombatPosX->GetFloat();
    fOverShoulderCombatAddY = overShoulderCombatAddY->GetFloat();
    fOverShoulderCombatPosZ = overShoulderCombatPosZ->GetFloat();
    fAutoVanityModeDelay = autoVanityModeDelay->GetFloat();
    fOverShoulderPosX = overShoulderPosX->GetFloat();
    fOverShoulderPosZ = overShoulderPosZ->GetFloat();
    fVanityModeMinDist = vanityModeMinDist->GetFloat();
    fVanityModeMaxDist = vanityModeMaxDist->GetFloat();
    fMouseWheelZoomSpeed = mouseWheelZoomSpeed->GetFloat();
    fTogglePOVDelay = togglePOVDelay->GetFloat();

    worldFOV = camera->worldFOV;
    player->GetGraphVariableBool("IsNPC", playerHeadtrackingEnabled);

    // temporarily disable headtracking if enabled
    player->SetGraphVariableBool("IsNPC", false);

    // toggle anim cam which unshackles camera and lets it move in front of player with their weapon drawn, necessary if
    // not using TDM
    thirdState->toggleAnimCam = true;
    thirdState->freeRotationEnabled = true;

    autoVanityModeDelay->data.f = 10800.0f;  // 3 hours
    togglePOVDelay->data.f = 10800.0f;

    if (g_isUltraWide) {
        fNewOverShoulderCombatPosX = -28.0f;
    } else {
        fNewOverShoulderCombatPosX = -35.0f;
    }

    if (isBeastForm) {
        fNewOverShoulderCombatPosX -= 27.0f;
    } else if (player->AsActorState()->IsWeaponDrawn()) {
        fNewOverShoulderCombatPosX -= 5.0f;
    }

    fNewOverShoulderCombatAddY = 0.f;
    auto playerSitState = player->AsActorState()->GetSitSleepState();
    if (player->IsOnMount()) {
        fNewOverShoulderCombatPosZ = 35.0f + (player->GetHeight() - 130);
        vanityModeMinDist->data.f = 190.0f;
        vanityModeMaxDist->data.f = 190.0f;
    }
    else if (playerSitState >= RE::SIT_SLEEP_STATE::kWantToSit && playerSitState <= RE::SIT_SLEEP_STATE::kWantToStand) {
        fNewOverShoulderCombatPosZ = -53.0f + (player->GetHeight() - 130);
        vanityModeMinDist->data.f = 155.0f;
        vanityModeMaxDist->data.f = 155.0f;
    } 
    else if (isBeastForm) {
        fNewOverShoulderCombatPosZ = -32.0f + (player->GetHeight() - 130);
        vanityModeMinDist->data.f = 200.0f;
        vanityModeMaxDist->data.f = 200.0f;
    } else if (isVampireLord) {
        fNewOverShoulderCombatPosZ = -37.0f + (player->GetHeight() - 128);
        vanityModeMinDist->data.f = 165.0f;
        vanityModeMaxDist->data.f = 165.0f;
    } else if (player->AsActorState()->IsWeaponDrawn()) {
        fNewOverShoulderCombatPosZ = -32.0f + (player->GetHeight() - 128);
        vanityModeMinDist->data.f = 165.0f;
        vanityModeMaxDist->data.f = 165.0f;
    }
    else {
        fNewOverShoulderCombatPosZ = -22.0f + (player->GetHeight() - 128);
        vanityModeMinDist->data.f = 155.0f;
        vanityModeMaxDist->data.f = 155.0f;
    }

    thirdState->freeRotation.x = MATH_PI - 0.5f;

    if (player->IsOnMount()) {
        RE::NiPointer<RE::Actor> playerMount;
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (player->GetMount(playerMount)) {
            auto* node = playerMount->Get3D();

            RE::NiMatrix3 rot = node->world.rotate;
            RE::NiPoint3 forward = rot * RE::NiPoint3{0, 1, 0};
            forward.Unitize();

            float slope = forward.z;
            thirdState->freeRotation.y = playerMount.get()->data.angle.x - 0.1f;
            logger::info("forward.z: {}", forward.z);
            
            if (forward.z < 0) { //going downhill
                fNewOverShoulderCombatPosX += 70.0f * forward.z;
                fNewOverShoulderCombatPosZ += 60.0f * forward.z;
            } else { //going uphill
                fNewOverShoulderCombatPosX += 40.0f * forward.z;
                fNewOverShoulderCombatPosZ += 50.0f * forward.z;
            }
            vanityModeMinDist->data.f -= 110.0f * forward.z;
            vanityModeMaxDist->data.f -= 110.0f * forward.z;
        }
    } else {
        thirdState->freeRotation.y = 0.0f;
    }

    // account for camera freeRotation settings getting pushed into player's pitch (x) values when weapon drawn
    if (!player->AsActorState()->IsWeaponDrawn())
        player->data.angle.x = 0.1f;
    else {
        player->data.angle.x -= player->data.angle.x - 0.1f;
    }
    
    overShoulderCombatPosX->data.f = fNewOverShoulderCombatPosX;
    overShoulderCombatAddY->data.f = fNewOverShoulderCombatAddY;
    overShoulderCombatPosZ->data.f = fNewOverShoulderCombatPosZ;

    overShoulderPosX->data.f = fNewOverShoulderCombatPosX;
    overShoulderPosZ->data.f = fNewOverShoulderCombatPosZ;

    // Skyrim Souls RE has the potential to mess with camera distance in menus after exiting, so when we restore it
    // later on, make the transition suitably instant
    mouseWheelZoomSpeed->data.f = 10000.0f;

    // make final camera distance not depend on camera pitch (looking down at player)
    thirdState->pitchZoomOffset = 0.1f;

    thirdState->posOffsetExpected = thirdState->posOffsetActual =
        RE::NiPoint3(fNewOverShoulderCombatPosX, fNewOverShoulderCombatAddY, fNewOverShoulderCombatPosZ);

    camera->worldFOV = 50.f;

    camera->Update();

    //timescale to 0
    RE::TESForm::LookupByID<RE::TESGlobal>(0x3A)->value = 0.f;

    //disables AI
    auto processLists = RE::ProcessLists::GetSingleton();
    if (processLists) {
        for (auto handle : processLists->highActorHandles) {
            auto actor = handle.get().get();
            if (!actor || actor->IsPlayerRef() || IsPlayersMount(actor)) continue;
            FreezeNPC(actor);
        }
    }
}

void ResetCamera() {
    auto player = RE::PlayerCharacter::GetSingleton();

    auto camera = RE::PlayerCamera::GetSingleton();
    auto thirdState = (RE::ThirdPersonState*)camera->cameraStates[RE::CameraState::kThirdPerson].get();

    if (forced3rdPerson) {
        // to cameraState = (RE::TESCameraState*)camera->cameraStates[m_camStateId].get();
        const auto firstPersonState =
            static_cast<RE::FirstPersonState*>(camera->cameraStates[RE::CameraState::kFirstPerson].get());
        camera->SetState(firstPersonState);
    }
    if (g_prevState) {
        camera->SetState(g_prevState);
    }

    // restore original values
    player->data.angle.x = playerAngleX;
    player->data.angle.z = playerRotation;
    autoVanityModeDelay->data.f = fAutoVanityModeDelay;
    togglePOVDelay->data.f = fTogglePOVDelay;
    thirdState->toggleAnimCam = false;
    thirdState->targetZoomOffset = targetZoomOffset;
    thirdState->freeRotation = freeRotation;
    vanityModeMinDist->data.f = fVanityModeMinDist;
    vanityModeMaxDist->data.f = fVanityModeMaxDist;
    camera->worldFOV = worldFOV;
    thirdState->posOffsetExpected = thirdState->posOffsetActual = posOffsetExpected;
    overShoulderCombatPosX->data.f = fOverShoulderCombatPosX;
    overShoulderCombatAddY->data.f = fOverShoulderCombatAddY;
    overShoulderCombatPosZ->data.f = fOverShoulderCombatPosZ;
    overShoulderPosX->data.f = fOverShoulderPosX;
    overShoulderPosZ->data.f = fOverShoulderPosZ;
    player->SetGraphVariableBool("IsNPC", playerHeadtrackingEnabled);

    forced3rdPerson = false;

    camera->Update();

    // camera->Update() function uses this value, so restore it after we've updated the camera
    mouseWheelZoomSpeed->data.f = fMouseWheelZoomSpeed;

    rotatedPlayer = false;

    //setting timescale back to its former value
    RE::TESForm::LookupByID<RE::TESGlobal>(0x3A)->value = timescale;

    //re-enables AI
    auto processLists = RE::ProcessLists::GetSingleton();
    if (processLists) {
        for (auto handle : processLists->highActorHandles) {
            auto actor = handle.get().get();
            if (!actor || actor->IsPlayerRef() || IsPlayersMount(actor)) continue;
            UnfreezeNPC(actor);
        }
    }
}

//credit goes to powerofthree for the freeze and unfreeze functions (https://github.com/powerof3/ClassicParalysis)
void FreezeNPC(RE::Actor* a_actor) {
    a_actor->PauseCurrentDialogue();
    //a_actor->InterruptCast(false);
    //a_actor->StopInteractingQuick(true);
    
    if (const auto currentProcess = a_actor->GetActorRuntimeData().currentProcess) {
        currentProcess->ClearMuzzleFlashes();
    }

    a_actor->GetActorRuntimeData().boolFlags.reset(RE::Actor::BOOL_FLAGS::kShouldAnimGraphUpdate);

    if (const auto charController = a_actor->GetCharController(); charController) {
        charController->flags.set(RE::CHARACTER_FLAGS::kNotPushable);

        charController->flags.reset(RE::CHARACTER_FLAGS::kRecordHits);
        charController->flags.reset(RE::CHARACTER_FLAGS::kHitFlags);
    }

    a_actor->EnableAI(false);
    a_actor->StopMoving(1.0f);
}

void UnfreezeNPC(RE::Actor* a_actor) {
    a_actor->GetActorRuntimeData().boolFlags.set(RE::Actor::BOOL_FLAGS::kShouldAnimGraphUpdate);

    if (const auto charController = a_actor->GetCharController()) {
        charController->flags.reset(RE::CHARACTER_FLAGS::kNotPushable);

        charController->flags.set(RE::CHARACTER_FLAGS::kRecordHits);
        charController->flags.set(RE::CHARACTER_FLAGS::kHitFlags);
    }

    a_actor->EnableAI(true);
}

const TESClass* GetBestMatchingClass(const std::vector<TESClass>& classes,
                                     const std::unordered_map<std::string, float>& skillLevels) {
    const TESClass* bestMatch = nullptr;
    float bestScore = -1.0f;

    for (const auto& tesClass : classes) {
        float score = 0.0f;
        //logger::info("CLASS: {}", tesClass.name);
        for (auto skill : tesClass.majorSkills) {
            //logger::info("-------{}", skill);
            auto it = skillLevels.find(skill);
            if (it != skillLevels.end()) {
                //logger::info("-------lvl{}", it->second);
                score += it->second;
            }
        }

        if (score > bestScore) {
            bestScore = score;
            bestMatch = &tesClass;
        }
    }
    logger::info("Selected class for Player: {}", bestMatch->name);
    return bestMatch;
}

static bool TryParseStage(const nlohmann::json& jStage, std::int32_t& outStage) {
    try {
        if (jStage.is_number_integer()) {
            outStage = jStage.get<std::int32_t>();
            return true;
        }
        if (jStage.is_string()) {
            outStage = std::stoi(jStage.get<std::string>());
            return true;
        }
    } catch (...) {
    }
    return false;
}

static bool LoadFactionFile(const fs::path& path, FactionDef& out) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger::warn("Failed to open faction JSON: {}", path.string());
        return false;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        logger::error("Error parsing faction JSON {}: {}", path.string(), e.what());
        return false;
    }

    if (!j.is_object()) {
        logger::warn("Faction JSON not an object: {}", path.string());
        return false;
    }

    if (!j.contains("id") || !j["id"].is_string() || !j.contains("name") || !j["name"].is_string() ||
        !j.contains("ranks") || !j["ranks"].is_array()) {
        logger::warn("Skipping invalid faction JSON {} (missing id/name/ranks)", path.string());
        return false;
    }

    out.id = j["id"].get<std::string>();
    out.name = j["name"].get<std::string>();
    out.gameFactionCheck.clear();
    if (j.contains("gameFactionCheck") && j["gameFactionCheck"].is_string()) {
        out.gameFactionCheck = j["gameFactionCheck"].get<std::string>();
    }
    out.ranks.clear();

    for (const auto& r : j["ranks"]) {
        if (!r.is_object()) {
            logger::warn("Skipping invalid rank in {} (rank not an object)", path.filename().string());
            continue;
        }

        if (!r.contains("maleTitle") || !r["maleTitle"].is_string() || !r.contains("femaleTitle") ||
            !r["femaleTitle"].is_string() || !r.contains("requirements") || !r["requirements"].is_array()) {
            logger::warn("Skipping invalid rank in {} (missing maleTitle/femaleTitle/requirements)",
                         path.filename().string());
            continue;
        }

        FactionRankDef rank;
        rank.requireAll = r.value("requireAll", true);
        rank.maleTitle = r["maleTitle"].get<std::string>();
        rank.femaleTitle = r["femaleTitle"].get<std::string>();
        rank.rankDisplayOnly = r.value("rankDisplayOnly", false);
        rank.requirements.clear();

        for (const auto& jr : r["requirements"]) {
            if (!jr.is_object()) {
                logger::warn("Skipping invalid requirement in {} (not an object)", path.filename().string());
                continue;
            }

            if (!jr.contains("quest") || !jr["quest"].is_string() || !jr.contains("stage")) {
                logger::warn("Skipping invalid requirement in {} (missing quest/stage)", path.filename().string());
                continue;
            }

            QuestRequirement req;
            req.quest = jr["quest"].get<std::string>();

            if (!TryParseStage(jr["stage"], req.stage)) {
                logger::warn("Skipping requirement with invalid stage in {} (quest={})", path.filename().string(),
                             req.quest);
                continue;
            }

            rank.requirements.emplace_back(std::move(req));
        }

        if (rank.requirements.empty()) {
            logger::warn("Skipping rank in {} (no valid requirements)", path.filename().string());
            continue;
        }

        out.ranks.emplace_back(std::move(rank));
    }


    if (out.ranks.empty()) {
        logger::warn("Faction '{}' has no valid ranks: {}", out.id, path.string());
    }

    return true;
}

static bool AreRankRequirementsMet(const FactionRankDef& rank) {
    if (rank.requirements.empty()) {
        return false;
    }

    if (rank.requireAll) {
        // AND
        for (const auto& req : rank.requirements) {
            if (!IsQuestStageAtLeast(req.quest, req.stage)) {
                return false;
            }
        }
        return true;
    } else {
        // OR
        for (const auto& req : rank.requirements) {
            if (IsQuestStageAtLeast(req.quest, req.stage)) {
                return true;
            }
        }
        return false;
    }
}


void LoadFactionDefinitions() {
    g_factionDefs.clear();

    try {
        if (!fs::exists(FACTIONS_DIRECTORY) || !fs::is_directory(FACTIONS_DIRECTORY)) {
            logger::warn("Faction directory missing: {}", FACTIONS_DIRECTORY);
            return;
        }

        std::size_t loaded = 0;
        for (const auto& entry : fs::directory_iterator(FACTIONS_DIRECTORY)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto& p = entry.path();
            if (p.extension() != ".json") {
                continue;
            }

            FactionDef def;
            if (LoadFactionFile(p, def)) {
                logger::trace("Loaded faction '{}' ({} ranks) from {}", def.id, def.ranks.size(), p.filename().string());
                g_factionDefs.emplace_back(std::move(def));
                ++loaded;
            }
        }

        logger::info("Faction JSON scan complete: {} loaded", loaded);
    } catch (const std::exception& e) {
        logger::error("Error iterating faction directory {}: {}", FACTIONS_DIRECTORY, e.what());
    }
}

const std::vector<FactionDef>& GetFactionDefinitions() { return g_factionDefs; }

static bool IsQuestStageAtLeast(const std::string& questEdid, std::int32_t requiredStage) {
    if (questEdid.empty()) {
        return false;
    }

    auto* quest = RE::TESForm::LookupByEditorID<RE::TESQuest>(questEdid);
    if (!quest) {
        logger::warn("FactionRanks: quest not found by EDID: {}", questEdid);
        return false;
    }

    const auto curStage = quest->GetCurrentStageID();
    return curStage >= requiredStage;
}

bool TryGetBestRankForFaction(const FactionDef& def, RE::SEXES::SEX gender, std::string& outRankTitle,
                              bool& outRankDisplayOnly) {
    outRankTitle.clear();
    outRankDisplayOnly = false;

    if (!IsPlayerInFactionWithRank(def.gameFactionCheck)) {
        return false;
    }

    bool found = false;
    for (const auto& r : def.ranks) {
        if (AreRankRequirementsMet(r)) {
            outRankTitle = (gender == RE::SEXES::SEX::kFemale) ? r.femaleTitle : r.maleTitle;
            outRankDisplayOnly = r.rankDisplayOnly;
            found = true;
        }
    }

    return found;
}

static inline bool IsWS(unsigned char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

static std::string TrimRightCopy(std::string s) {
    while (!s.empty() && IsWS(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

std::string RemoveLastDotSentence(std::string text) {
    text = TrimRightCopy(std::move(text));
    if (text.empty()) {
        return text;
    }

    std::vector<size_t> sentenceEnds;
    sentenceEnds.reserve(8);

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '.') {
            continue;
        }

        const bool atEnd = (i + 1 >= text.size());
        const bool nextIsWS = (!atEnd && IsWS(static_cast<unsigned char>(text[i + 1])));

        if (atEnd || nextIsWS) {
            size_t j = i + 1;
            while (j < text.size() && IsWS(static_cast<unsigned char>(text[j]))) {
                ++j;
            }
            sentenceEnds.push_back(j);
        }
    }

    if (sentenceEnds.size() < 2) {
        return text;
    }

    text.erase(sentenceEnds[sentenceEnds.size() - 2]);
    return TrimRightCopy(std::move(text));
}

std::string CollapsePercent(std::string s) {
    std::string out;
    out.reserve(s.size());

    bool lastWasPercent = false;
    for (char c : s) {
        if (c == '%') {
            if (!lastWasPercent) {
                out.push_back('%');
                lastWasPercent = true;
            }
        } else {
            out.push_back(c);
            lastWasPercent = false;
        }
    }
    return out;
}

static bool IsPlayerInFactionWithRank(const std::string& factionEdid) {
    if (factionEdid.empty()) {
        return true;
    }

    auto* player = RE::PlayerCharacter::GetSingleton();
    auto* base = player ? player->GetActorBase() : nullptr;
    if (!base) {
        return false;
    }

    bool found = false;

    player->VisitFactions([&](const RE::TESFaction* fac, int8_t rank) {
        if (!fac) return false;

        if (rank > -1) {
            auto edid = clib_util::editorID::get_editorID(fac);
            if (edid == factionEdid) {
                found = true;
                return true;  // stop iteration
            }
        }
        return false;
    });

    return found;
}
