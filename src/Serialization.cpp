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
    g_savedTitleRankOnly = false;
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

    std::uint8_t rankOnly = g_savedTitleRankOnly ? 1 : 0;
    intfc->WriteRecordData(&rankOnly, sizeof(rankOnly));
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
        if (type != 'CTTL') {
            SkipRecordData(intfc, length);
            continue;
        }

        if (version != 1) {
            SkipRecordData(intfc, length);
            continue;
        }

        std::uint32_t bytesRead = 0;

        auto readExact = [&](void* dst, std::uint32_t sz) -> bool {
            if (bytesRead + sz > length) {
                return false;
            }
            if (!intfc->ReadRecordData(dst, sz)) {
                return false;
            }
            bytesRead += sz;
            return true;
        };

        auto readString = [&](std::string& out) -> bool {
            uint32_t len = 0;
            if (!readExact(&len, sizeof(len))) {
                return false;
            }
            if (len > 4096) {  // sanity cap
                return false;
            }
            if (bytesRead + len > length) {
                return false;
            }
            out.resize(len);
            if (len > 0 && !readExact(out.data(), len)) {
                return false;
            }
            return true;
        };

        g_hasSavedTitle = false;
        g_savedTitleRank.clear();
        g_savedTitleFaction.clear();
        g_savedTitleRankOnly = false;

        if (!readExact(&g_hasSavedTitle, sizeof(g_hasSavedTitle))) {
            SkipRecordData(intfc, length - bytesRead);
            continue;
        }

        std::string rank, fac;
        if (!readString(rank) || !readString(fac)) {
            SkipRecordData(intfc, length - bytesRead);
            continue;
        }

        g_savedTitleRank = std::move(rank);
        g_savedTitleFaction = std::move(fac);

        if (length - bytesRead >= sizeof(std::uint8_t)) {
            std::uint8_t rankOnly = 0;
            if (readExact(&rankOnly, sizeof(rankOnly))) {
                g_savedTitleRankOnly = (rankOnly != 0);
            }
        } else {
            g_savedTitleRankOnly = false;
        }

        if (length > bytesRead) {
            SkipRecordData(intfc, length - bytesRead);
        }
    }
}
