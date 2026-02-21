#ifndef EVENT_PROCESSOR_H
#define EVENT_PROCESSOR_H

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

struct StandingStoneInfo {
    std::string name;
    std::string descriptionEditorId;
};

class EventProcessor : public RE::BSTEventSink<RE::InputEvent*>, public RE::BSTEventSink<RE::MenuOpenCloseEvent>, public RE::BSTEventSink<SKSE::ModCallbackEvent> {
public:
    static EventProcessor* GetSingleton() {
        static EventProcessor instance;
        return &instance;
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* eventPtr,
                                          RE::BSTEventSource<RE::InputEvent*>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const SKSE::ModCallbackEvent* event,
                                          RE::BSTEventSource<SKSE::ModCallbackEvent>* eventSource) override;
};

extern SKSE::PluginHandle pluginHandle;

#endif  // EVENT_PROCESSOR_H