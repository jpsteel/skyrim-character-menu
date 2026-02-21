#pragma once

namespace Data {
    namespace Horseman {
        [[nodiscard]] inline float GetLevel() {
            auto level = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillHorsemanLevel");
            return level ? level->value : 0.0f;
        }
        [[nodiscard]] inline int GetRatio() {
            auto xpProgress = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillHorsemanRatio");
            return xpProgress ? 200 - (xpProgress->value * 200) : 200;
        }
    }

    namespace Exploration {
        [[nodiscard]] inline float GetLevel() {
            auto level = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillExplorationLevel");
            return level ? level->value : 0.0f;
        }
        [[nodiscard]] inline int GetRatio() {
            auto xpProgress = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillExplorationRatio");
            return xpProgress ? 200 - (xpProgress->value * 200) : 200;
        }
    }

    namespace Philosophy {
        [[nodiscard]] inline float GetLevel() {
            auto level = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillPhilosophyLevel");
            return level ? level->value : 0.0f;
        }
        [[nodiscard]] inline int GetRatio() {
            auto xpProgress = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillPhilosophyRatio");
            return xpProgress ? 200 - (xpProgress->value * 200) : 200;
        }
    }

    namespace HandToHand {
        [[nodiscard]] inline float GetLevel() {
            auto level = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillHandToHandLevel");
            return level ? level->value : 0.0f;
        }
        [[nodiscard]] inline int GetRatio() {
            auto xpProgress = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillHandToHandRatio");
            return xpProgress ? 200 - (xpProgress->value * 200) : 200;
        }
    }

    namespace Athletics {
        [[nodiscard]] inline float GetLevel() {
            auto level = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillAthleticsLevel");
            return level ? level->value : 0.0f;
        }
        [[nodiscard]] inline int GetRatio() {
            auto xpProgress = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillAthleticsRatio");
            return xpProgress ? 200 - (xpProgress->value * 200) : 200;
        }
    }

    namespace Sorcery {
        [[nodiscard]] inline float GetLevel() {
            auto level = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillSorceryLevel");
            return level ? level->value : 0.0f;
        }
        [[nodiscard]] inline int GetRatio() {
            auto xpProgress = RE::TESForm::LookupByEditorID<RE::TESGlobal>("SkillSorceryRatio");
            return xpProgress ? 200 - (xpProgress->value * 200) : 200;
        }
    }
}