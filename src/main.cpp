#include <spdlog/sinks/basic_file_sink.h>

#include "Utility.h"
#include "EventProcessor.h"
#include "Scaleform.h"
#include "CharacterSheet.h"
#define SMOOTHCAM_API_COMMONLIB
#include "SmoothCamAPI.h"
#include "APIManager.h"
#include "Serialization.h"


void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    auto eventProcessor = EventProcessor::GetSingleton();
    switch (message->type) {
        case (SKSE::MessagingInterface::kDataLoaded):
            APIs::RequestAPIs();
            RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(eventProcessor);
            Scaleform::CharacterSheet::Register();
            break;
        case (SKSE::MessagingInterface::kInputLoaded):
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent*>(eventProcessor);
            SKSE::GetModCallbackEventSource()->AddEventSink(eventProcessor);
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kPostPostLoad:
            APIs::RequestAPIs();
            break;
        case SKSE::MessagingInterface::kPostLoad:
            APIs::RequestAPIs();
            break;
        case SKSE::MessagingInterface::kNewGame:
            APIs::RequestAPIs();
            break;
        case SKSE::MessagingInterface::kSaveGame:
        default: 
            break;
    }
}

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    SetupLog();
    spdlog::set_level(spdlog::level::info);

    auto* ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID('CTTL');
    ser->SetRevertCallback(RevertCallback);
    ser->SetSaveCallback(SaveCallback);
    ser->SetLoadCallback(LoadCallback);


    SKSE::GetMessagingInterface()->RegisterListener(SKSEMessageHandler);
    pluginHandle = skse->GetPluginHandle();

    LoadDataFromINI();
    LoadFactionDefinitions();

    logger::info("Character Sheet successfully loaded.");

    return true;
}