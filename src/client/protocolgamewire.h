#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <tuple>
#include <string>

namespace ProtocolGameWire {
    // Qt 15.33 resource decoder: U32 for 30..33, 60, 80, 86 and 87.
    template<class Reader>
    uint64_t readResourceBalance(Reader& msg, uint8_t type, uint16_t version)
    {
        switch (type) {
            case 30: case 31: case 32: case 33: case 86: case 87:
                return msg.getU32();
            case 60: case 80:
                if (version >= 1533)
                    return msg.getU32();
                [[fallthrough]];
            default:
                return msg.getU64();
        }
    }

    struct PlayerModes {
        uint8_t fight;
        uint8_t chase;
        bool safe;
        uint8_t pvp;
    };

    template<class Reader>
    PlayerModes readPlayerModes(Reader& msg, uint16_t version, bool hasPvp, uint8_t currentFight)
    {
        // Qt 15.33 carries chase, secure and PvP only. Keep local fight mode.
        const auto fight = version >= 1533 ? currentFight : msg.getU8();
        const auto chase = msg.getU8();
        const bool safe = msg.getU8() != 0;
        const auto pvp = static_cast<uint8_t>(version >= 1533 || hasPvp ? msg.getU8() : 0);
        return {fight, chase, safe, pvp};
    }

    template<class Writer>
    void writePlayerModes(Writer& msg, uint16_t version, bool hasPvp, const PlayerModes& modes)
    {
        if (version < 1530)
            msg.addU8(modes.fight);
        msg.addU8(modes.chase);
        msg.addU8(modes.safe);
        if (version >= 1530 || hasPvp)
            msg.addU8(modes.pvp);
        // Preserve Rubik's custom-server 15.30 layout; official 15.33 has no tail.
        if (version >= 1530 && version < 1533)
            msg.addU8(0);
    }

    struct StaticHouseData {
        uint32_t primaryHouseId;
        std::array<uint8_t, 6> parameters;
        std::vector<uint32_t> accountHouseIds;
        std::vector<uint32_t> houseIds;
    };

    template<class Reader>
    StaticHouseData readStaticHouseData(Reader& msg)
    {
        // Qt 15.33 0xC6: U32, six U8 scalars, U8-sized list, U16-sized list.
        StaticHouseData data{};
        data.primaryHouseId = msg.getU32();
        for (auto& parameter : data.parameters)
            parameter = msg.getU8();
        const auto accountCount = msg.getU8();
        for (unsigned i = 0; i < accountCount; ++i)
            data.accountHouseIds.push_back(msg.getU32());
        const auto houseCount = msg.getU16();
        for (unsigned i = 0; i < houseCount; ++i)
            data.houseIds.push_back(msg.getU32());
        return data;
    }


    using SpellDataModifications = std::vector<std::tuple<uint16_t, uint8_t, int32_t>>;

    template<class Reader>
    SpellDataModifications readSpellDataModifications(Reader& msg)
    {
        // GameserverMessageSpellDataModification: count, then spell/type/value.
        // Linux translator 0xb78800 reads U16, U8, signed I32 per entry.
        const auto count = msg.getU16();
        SpellDataModifications result;
        result.reserve(count);
        for (unsigned i = 0; i < count; ++i) {
            const auto spell = msg.getU16();
            const auto type = msg.getU8();
            const auto value = static_cast<int32_t>(msg.getU32());
            result.emplace_back(spell, type, value);
        }
        return result;
    }


    struct StoreOpenParameters {
        uint8_t service = 0;
        uint8_t subType = 0;
        std::string category;
        std::string filter;
        uint32_t offerId = 0;
        std::string search;
        uint8_t parameter7 = 0;
        uint8_t parameter8 = 0;
    };

    template<class Reader>
    StoreOpenParameters readStoreOpenParameters(Reader& msg)
    {
        // Translator helper 0xb5f3e0: service selects one payload; two U8 tails always follow.
        StoreOpenParameters data;
        data.service = msg.getU8();
        switch (data.service) {
            case 1: case 3:
                data.subType = msg.getU8();
                break;
            case 2:
                data.category = msg.getString();
                data.filter = msg.getString();
                break;
            case 4:
                data.offerId = msg.getU32();
                break;
            case 5:
                data.search = msg.getString();
                break;
            default:
                break;
        }
        data.parameter7 = msg.getU8();
        data.parameter8 = msg.getU8();
        return data;
    }

    struct CreatureTypeUpdate {
        uint32_t id;
        uint8_t type;
        uint32_t masterId = 0;
    };

    template<class Reader>
    CreatureTypeUpdate readCreatureTypeUpdate(Reader& msg)
    {
        CreatureTypeUpdate data{};
        data.id = msg.getU32();
        data.type = msg.getU8();
        if (data.type == 3)
            data.masterId = msg.getU32();
        return data;
    }

    struct HouseActionResult {
        uint32_t identifier;
        uint8_t action;
        uint8_t result;
        bool hasExtra = false;
        bool extra = false;
    };

    template<class Reader>
    HouseActionResult readHouseActionResult(Reader& msg)
    {
        HouseActionResult data{};
        data.identifier = msg.getU32();
        data.action = msg.getU8();
        data.result = msg.getU8();
        data.hasExtra = data.action == 1 && data.result == 0;
        if (data.hasExtra)
            data.extra = msg.getU8() != 0;
        return data;
    }

    using DepotSearchResults = std::vector<std::tuple<uint16_t, uint8_t, uint16_t>>;

    template<class Reader, class IsUpgradeable>
    DepotSearchResults readDepotSearchResults(Reader& msg, IsUpgradeable isUpgradeable)
    {
        const auto count = msg.getU16();
        DepotSearchResults results;
        results.reserve(count);
        for (unsigned i = 0; i < count; ++i) {
            const auto itemId = msg.getU16();
            const auto tier = static_cast<uint8_t>(isUpgradeable(itemId) ? msg.getU8() : 0);
            const auto amount = msg.getU16();
            results.emplace_back(itemId, tier, amount);
        }
        return results;
    }


    template<class Reader>
    std::vector<uint16_t> readUnlockedSpells(Reader& msg)
    {
        const auto count = msg.getU16();
        std::vector<uint16_t> spells;
        spells.reserve(count);
        for (unsigned i = 0; i < count; ++i)
            spells.push_back(msg.getU16());
        return spells;
    }

}
