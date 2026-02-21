#ifndef UTILITY_H
#define UTILITY_H

#include "Classes.h"
#define SMOOTHCAM_API_COMMONLIB
#include "SmoothCamAPI.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <SimpleIni.h>

#include "RE/Skyrim.h"

extern int menuHotkey;
extern int enableBlur;
extern bool g_isUltraWide;

inline std::string g_savedTitleRank;
inline std::string g_savedTitleFaction;
inline bool g_hasSavedTitle = false;

namespace fs = std::filesystem;

struct QuestRequirement {
    std::string quest;
    std::int32_t stage = 0;
};

struct FactionRankDef {
    bool requireAll = true;  // true = AND, false = OR
    std::vector<QuestRequirement> requirements;
    std::string maleTitle;
    std::string femaleTitle;
    bool rankDisplayOnly = false;
};

struct FactionDef {
    std::string id;
    std::string name;
    std::string gameFactionCheck;
    std::vector<FactionRankDef> ranks;
};

static const std::string FACTIONS_DIRECTORY = "Data/interface/character/factions";
static const std::string INI_FILE_PATH = "Data/Character Menu SE.ini";

namespace logger = SKSE::log;

extern bool forced3rdPerson;
extern bool rotatedPlayer;
extern bool fixCameraZoom;
extern bool sittingPlayer;
extern bool shouldDisableAnimCam;
extern float playerAngleX;
extern float playerRotation;
extern float targetZoomOffset;
extern RE::NiPoint2 freeRotation;
extern RE::NiPoint2 g_freeRotation;
extern RE::TESCameraState* g_prevState;
extern RE::NiPoint3 posOffsetExpected;
extern RE::NiPointer<RE::NiFloatInterpolator> radialBlurStrength;
extern float blurRadius;
extern RE::Setting* overShoulderCombatPosX;
extern float fOverShoulderCombatPosX;
extern RE::Setting* overShoulderCombatAddY;
extern float fOverShoulderCombatAddY;
extern RE::Setting* overShoulderCombatPosZ;
extern float fOverShoulderCombatPosZ;
extern RE::Setting* autoVanityModeDelay;
extern float fAutoVanityModeDelay;
extern RE::Setting* overShoulderPosX;
extern float fOverShoulderPosX;
extern RE::Setting* overShoulderPosZ;
extern float fOverShoulderPosZ;
extern RE::Setting* vanityModeMinDist;
extern float fVanityModeMinDist;
extern RE::Setting* vanityModeMaxDist;
extern float fVanityModeMaxDist;
extern RE::Setting* mouseWheelZoomSpeed;
extern float fMouseWheelZoomSpeed;
extern float worldFOV;
extern bool playerHeadtrackingEnabled;
extern float fNewOverShoulderCombatPosX;
extern float fNewOverShoulderCombatAddY;
extern float fNewOverShoulderCombatPosZ;
extern float timescale;

inline std::string GetActorValueKey(RE::ActorValue av) noexcept {
    switch (av) {
        case RE::ActorValue::kAlchemy:
            return "alchemy";
        case RE::ActorValue::kAlteration:
            return "alteration";
        case RE::ActorValue::kArchery:
            return "archery";
        case RE::ActorValue::kBlock:
            return "block";
        case RE::ActorValue::kConjuration:
            return "conjuration";
        case RE::ActorValue::kDestruction:
            return "destruction";
        case RE::ActorValue::kEnchanting:
            return "enchanting";
        case RE::ActorValue::kHeavyArmor:
            return "heavyArmor";
        case RE::ActorValue::kIllusion:
            return "illusion";
        case RE::ActorValue::kLightArmor:
            return "lightArmor";
        case RE::ActorValue::kLockpicking:
            return "lockpicking";
        case RE::ActorValue::kOneHanded:
            return "oneHanded";
        case RE::ActorValue::kPickpocket:
            return "pickpocket";
        case RE::ActorValue::kRestoration:
            return "restoration";
        case RE::ActorValue::kSmithing:
            return "smithing";
        case RE::ActorValue::kSneak:
            return "sneak";
        case RE::ActorValue::kSpeech:
            return "speech";
        case RE::ActorValue::kTwoHanded:
            return "twoHanded";
        default:
            return {};
    }
}

void SetupLog();
float GetPlayerXPProgression();
std::string GetInGameDate();
int GetPlayerGold();
bool HasPlayerSpellByEDID(RE::PlayerCharacter* player, const char* edid);
bool IsPlayerVampire(RE::PlayerCharacter* player);
const char* GetPlayerCondition();
bool IsPluginLoaded(const std::string& pluginName);
void TogglePlayerControls(bool enable);
bool GetSurvivalModeEnabled();
void RotateCamera();
void ResetCamera();
void FreezeNPC(RE::Actor* a_actor);
void UnfreezeNPC(RE::Actor* a_actor);
const TESClass* GetBestMatchingClass(const std::vector<TESClass>& classes,
                                     const std::unordered_map<std::string, float>& skillLevels);
void LoadFactionDefinitions();
static bool IsQuestStageAtLeast(const std::string& questEdid, std::int32_t requiredStage);
const std::vector<FactionDef>& GetFactionDefinitions();
bool TryGetBestRankForFaction(const FactionDef& def, RE::SEXES::SEX gender, std::string& outRankTitle,
                              bool& outRankDisplayOnly);
static inline bool IsWS(unsigned char c);
static std::string TrimRightCopy(std::string s);
std::string RemoveLastDotSentence(std::string text);
std::string CollapsePercent(std::string s);
bool IsPlayersMount(const RE::Actor* actor);
void LoadDataFromINI();
static bool IsPlayerInFactionWithRank(const std::string& factionEdid);

#endif  // UTILITY_H