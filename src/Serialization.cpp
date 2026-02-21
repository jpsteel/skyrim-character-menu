#include "Serialization.h"

#include <cstdint>
#include <type_traits>

#include "RE/Skyrim.h"
#include "Utility.h"

namespace {
    template <class T>
    bool Write(SKSE::SerializationInterface* intfc, const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);
        return intfc->WriteRecordData(std::addressof(value), sizeof(T));
    }

    template <class T>
    bool Read(SKSE::SerializationInterface* intfc, T& value) {
        static_assert(std::is_trivially_copyable_v<T>);
        return intfc->ReadRecordData(std::addressof(value), sizeof(T));
    }
}

void RevertCallback(SKSE::SerializationInterface*) {
    g_hasSavedTitle = false;
    g_savedTitleRank.clear();
    g_savedTitleFaction.clear();
}

void SaveCallback(SKSE::SerializationInterface* intfc) {
    intfc->OpenRecord('CTTL', 1);

    intfc->WriteRecordData(&g_hasSavedTitle, sizeof(g_hasSavedTitle));

    auto writeString = [&](const std::string& s) {
        uint32_t len = static_cast<uint32_t>(s.size());
        intfc->WriteRecordData(&len, sizeof(len));
        if (len > 0) {
            intfc->WriteRecordData(s.data(), len);
        }
    };

    writeString(g_savedTitleRank);
    writeString(g_savedTitleFaction);
}


static void SkipRecordData(SKSE::SerializationInterface* intfc, std::uint32_t length) {
    std::uint8_t buf[256];
    while (length > 0) {
        const auto chunk = (std::min)(length, static_cast<std::uint32_t>(sizeof(buf)));
        if (!intfc->ReadRecordData(buf, chunk)) {
            return;
        }
        length -= chunk;
    }
}

void LoadCallback(SKSE::SerializationInterface* intfc) {
    uint32_t type, version, length;

    while (intfc->GetNextRecordInfo(type, version, length)) {
        if (type == 'CTTL') {
            if (version != 1) {
                SkipRecordData(intfc, length);
                continue;
            }

            auto readString = [&](std::string& out) -> bool {
                uint32_t len = 0;
                if (!intfc->ReadRecordData(&len, sizeof(len))) {
                    return false;
                }

                if (len > 4096) {
                    return false;
                }

                out.resize(len);
                if (len > 0 && !intfc->ReadRecordData(out.data(), len)) {
                    return false;
                }
                return true;
            };

            if (!intfc->ReadRecordData(&g_hasSavedTitle, sizeof(g_hasSavedTitle))) {
                g_hasSavedTitle = false;
                continue;
            }

            std::string rank, fac;
            if (!readString(rank) || !readString(fac)) {
                g_hasSavedTitle = false;
                g_savedTitleRank.clear();
                g_savedTitleFaction.clear();
                continue;
            }

            g_savedTitleRank = std::move(rank);
            g_savedTitleFaction = std::move(fac);
        } else {
            SkipRecordData(intfc, length);
        }
    }
}
