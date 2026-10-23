#include "client/protocolgamewire.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

namespace {
    unsigned checks = 0;

    void require(const bool condition, const char* message)
    {
        ++checks;
        if (!condition)
            throw std::runtime_error(message);
    }

    struct Reader {
        std::vector<uint8_t> bytes;
        size_t pos = 0;

        uint8_t getU8()
        {
            if (pos == bytes.size())
                throw std::out_of_range("truncated packet");
            return bytes[pos++];
        }

        uint16_t getU16()
        {
            const auto low = getU8();
            return low | (uint16_t(getU8()) << 8);
        }

        uint32_t getU32()
        {
            uint32_t value = 0;
            for (unsigned shift = 0; shift < 32; shift += 8)
                value |= uint32_t(getU8()) << shift;
            return value;
        }

        std::string getString()
        {
            const auto length = getU16();
            std::string value;
            for (unsigned i = 0; i < length; ++i)
                value.push_back(static_cast<char>(getU8()));
            return value;
        }

        uint64_t getU64()
        {
            uint64_t value = 0;
            for (unsigned shift = 0; shift < 64; shift += 8)
                value |= uint64_t(getU8()) << shift;
            return value;
        }
    };


    struct Writer {
        std::vector<uint8_t> bytes;
        void addU8(uint8_t value) { bytes.push_back(value); }
    };

    void resourceWidths()
    {
        for (uint16_t version : {1530, 1533}) {
            for (unsigned type = 0; type < 256; ++type) {
                const bool narrow = (type >= 30 && type <= 33) || type == 86 || type == 87 ||
                    (version >= 1533 && (type == 60 || type == 80));
                Reader r{{1, 0, 0, 0}};
                if (!narrow) r.bytes.insert(r.bytes.end(), {1, 0, 0, 0});
                r.bytes.push_back(0xEE);
                require(ProtocolGameWire::readResourceBalance(r, type, version) ==
                    (narrow ? 1ULL : 0x100000001ULL), "resource value/width");
                require(r.getU8() == 0xEE && r.pos == r.bytes.size(), "next resource opcode survives");
            }
        }
    }

    void capturedModesBoundary()
    {
        // Actual failing log: A7 01 01 00 D5 15 00 08 00 [Amphibic...].
        Reader r{{0xA7, 1, 1, 0, 0xD5, 0x15, 0, 8, 0}};
        require(r.getU8() == 0xA7, "modes opcode");
        auto modes = ProtocolGameWire::readPlayerModes(r, 1533, true, 2);
        require(modes.fight == 2 && modes.chase == 1 && modes.safe && modes.pvp == 0,
            "new modes retain local fight setting");
        require(r.getU8() == 0xD5 && r.getU16() == 21 && r.getU16() == 8,
            "following bestiary packet remains aligned");
        Writer w;
        ProtocolGameWire::writePlayerModes(w, 1533, true, modes);
        require(w.bytes == std::vector<uint8_t>({1, 1, 0}), "15.33 sends three bytes without padding");
        Reader old{{2, 1, 1, 3, 0xD5}};
        modes = ProtocolGameWire::readPlayerModes(old, 1530, true, 1);
        require(modes.fight == 2 && modes.pvp == 3 && old.getU8() == 0xD5, "legacy receive layout");
        Writer custom;
        ProtocolGameWire::writePlayerModes(custom, 1530, true, modes);
        require(custom.bytes == std::vector<uint8_t>({1, 1, 3, 0}), "custom 15.30 transmit preserved");
        Reader ancient{{3, 0, 1, 0xD5}};
        modes = ProtocolGameWire::readPlayerModes(ancient, 860, false, 1);
        require(modes.fight == 3 && modes.pvp == 0 && ancient.getU8() == 0xD5, "legacy without PvP");
        Writer legacy;
        ProtocolGameWire::writePlayerModes(legacy, 860, false, modes);
        require(legacy.bytes == std::vector<uint8_t>({3, 0, 1}), "legacy transmit without PvP");
    }

    void chainedResourcesAndBlessings()
    {
        Reader r{{0xEE, 60, 7, 0, 0, 0, 0x9C, 0, 0, 1,
                  0xEE, 80, 8, 0, 0, 0, 0xA7, 1, 1, 0, 0xD5}};
        require(r.getU8() == 0xEE, "resource prefix");
        auto type = r.getU8();
        require(ProtocolGameWire::readResourceBalance(r, type, 1533) == 7, "NPC balance");
        require(r.getU8() == 0x9C && r.getU16() == 0 && r.getU8() == 1, "blessings stay aligned");
        require(r.getU8() == 0xEE, "next resource prefix");
        type = r.getU8();
        require(ProtocolGameWire::readResourceBalance(r, type, 1533) == 8, "wheel balance");
        require(r.getU8() == 0xA7, "modes prefix after resource");
        ProtocolGameWire::readPlayerModes(r, 1533, true, 1);
        require(r.getU8() == 0xD5 && r.pos == r.bytes.size(), "packet chain fully consumed");
        Reader truncated{{1, 2, 3}};
        bool rejected = false;
        try { ProtocolGameWire::readResourceBalance(truncated, 60, 1533); }
        catch (const std::out_of_range&) { rejected = true; }
        require(rejected, "truncated resource must fail");
    }
    void staticHouseLists()
    {
        Reader captured{{0xC6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x01,0x00,0x03,0x00,0xC5,0xB3,0x00,0x00,0x87,0xC4,0x00,0x00,0xA7,0xE6,0x00,0x00}};
        require(captured.getU8() == 0xC6, "captured house opcode");
        auto data = ProtocolGameWire::readStaticHouseData(captured);
        require(data.primaryHouseId == 0 && data.accountHouseIds.empty(), "captured empty account list");
        require(data.houseIds == std::vector<uint32_t>({46021, 50311, 59047}), "captured house IDs");
        require(captured.pos == captured.bytes.size(), "captured house packet ends before 0x0B");
        Reader zero{{0,0,0,0, 0,0,0,3,3,1, 0, 0,0, 0x0B}};
        data = ProtocolGameWire::readStaticHouseData(zero);
        require(data.houseIds.empty() && zero.getU8() == 0x0B, "both empty lists preserve next opcode");
        Reader multiple{{7,0,0,0, 0,2,0,3,3,1, 2, 7,0,0,0, 9,0,0,0, 1,0, 10,0,0,0, 0xEE}};
        data = ProtocolGameWire::readStaticHouseData(multiple);
        require(data.accountHouseIds == std::vector<uint32_t>({7,9}), "multiple account houses");
        require(data.houseIds == std::vector<uint32_t>({10}) && multiple.getU8() == 0xEE, "second list boundary");
        Reader one{{7,0,0,0, 0,1,0,3,3,1, 1, 7,0,0,0, 0,0, 0xEE}};
        data = ProtocolGameWire::readStaticHouseData(one);
        require(data.accountHouseIds == std::vector<uint32_t>({7}) && one.getU8() == 0xEE, "one account house");
        Reader truncated{{0,0,0,0, 0,0,0,3,3,1, 1, 7}};
        bool rejected = false;
        try { ProtocolGameWire::readStaticHouseData(truncated); }
        catch (const std::out_of_range&) { rejected = true; }
        require(rejected, "truncated house ID must fail");
    }

    void spellModificationBoundaries()
    {
        // Captured warning: 60 00 00 EE 01 33 00 00 00 00 00 00 00 5E ...
        Reader captured{{0x60,0,0,0xEE,1,0x33,0,0,0,0,0,0,0,0x5E}};
        require(captured.getU8() == 0x60, "spell modification opcode");
        const auto empty = ProtocolGameWire::readSpellDataModifications(captured);
        require(empty.empty(), "zero modifications are a valid packet");
        require(captured.getU8() == 0xEE && captured.getU8() == 1, "following resource remains aligned");
        require(ProtocolGameWire::readResourceBalance(captured, 1, 1533) == 51,
            "captured resource value after empty modifications");
        require(captured.getU8() == 0x5E && captured.pos == captured.bytes.size(), "next passive packet survives");
        Reader list{{2,0, 0x34,0x12,2, 0xF6,0xFF,0xFF,0xFF, 0xFF,0xFF,3, 0x78,0x56,0x34,0x12, 0xEE}};
        const auto entries = ProtocolGameWire::readSpellDataModifications(list);
        require(entries.size() == 2, "both modifications decoded");
        require(entries[0] == std::make_tuple(uint16_t(0x1234), uint8_t(2), int32_t(-10)), "negative modifier stays signed");
        require(entries[1] == std::make_tuple(uint16_t(65535), uint8_t(3), int32_t(0x12345678)), "full spell and modifier values");
        require(list.getU8() == 0xEE && list.pos == list.bytes.size(), "nonempty list preserves next opcode");
        for (size_t size = 0; size < 9; ++size) {
            Reader cut{{1,0, 0x34,0x12,2, 1,0,0,0}};
            cut.bytes.resize(size);
            bool rejected = false;
            try { ProtocolGameWire::readSpellDataModifications(cut); }
            catch (const std::out_of_range&) { rejected = true; }
            require(rejected, "truncated count or modifier rejected");
        }
    }

    void storeDeepLinkVariants()
    {
        // Captured A8 payload 00 00 03, followed by A9 01 00 and a string.
        Reader empty{{0,0,3,0xA9,1,0,0,0}};
        auto data = ProtocolGameWire::readStoreOpenParameters(empty);
        require(data.service == 0 && data.parameter7 == 0 && data.parameter8 == 3,
            "deeplink has two tail bytes even without a variant payload");
        require(empty.getU8() == 0xA9 && empty.getU8() == 1 && empty.getU8() == 0 && empty.getString().empty(),
            "captured deeplink/resting-area boundary");
        for (uint8_t service : {1,3}) {
            Reader r{{service,7,2,3,0xEE}};
            data = ProtocolGameWire::readStoreOpenParameters(r);
            require(data.subType == 7 && data.parameter7 == 2 && data.parameter8 == 3 && r.getU8() == 0xEE,
                "byte deeplink variant");
        }
        Reader category{{2,3,0,'a','b','c',1,0,'x',2,3,0xEE}};
        data = ProtocolGameWire::readStoreOpenParameters(category);
        require(data.category == "abc" && data.filter == "x" && category.getU8() == 0xEE,
            "category/filter variant");
        Reader offer{{4,0x78,0x56,0x34,0x12,2,3,0xEE}};
        data = ProtocolGameWire::readStoreOpenParameters(offer);
        require(data.offerId == 0x12345678 && offer.getU8() == 0xEE, "offer variant is U32");
        Reader search{{5,3,0,'a','b','c',2,3,0xEE}};
        data = ProtocolGameWire::readStoreOpenParameters(search);
        require(data.search == "abc" && search.getU8() == 0xEE, "search variant");
        for (size_t size = 0; size < 7; ++size) {
            Reader cut{{4,1,0,0,0,2,3}};
            cut.bytes.resize(size);
            bool rejected = false;
            try { ProtocolGameWire::readStoreOpenParameters(cut); }
            catch (const std::out_of_range&) { rejected = true; }
            require(rejected, "truncated store parameters rejected");
        }
    }

    void creatureTypeAndHouseResults()
    {
        for (uint8_t type : {0,1,2,3}) {
            Reader r{{1,0,0,0,type}};
            if (type == 3) r.bytes.insert(r.bytes.end(), {0x78,0x56,0x34,0x12});
            r.bytes.push_back(0xEE);
            const auto data = ProtocolGameWire::readCreatureTypeUpdate(r);
            require(data.id == 1 && data.type == type && data.masterId == (type == 3 ? 0x12345678 : 0),
                "owner only follows summon type");
            require(r.getU8() == 0xEE, "creature update preserves next opcode even if creature absent");
        }
        for (uint8_t action : {0,1,2,7}) {
            for (uint8_t result : {0,1,3}) {
                Reader r{{1,0,0,0,action,result}};
                const bool extra = action == 1 && result == 0;
                if (extra) r.bytes.push_back(1);
                r.bytes.push_back(0xEE);
                const auto data = ProtocolGameWire::readHouseActionResult(r);
                require(data.identifier == 1 && data.action == action && data.result == result &&
                    data.hasExtra == extra && data.extra == extra, "house action/result and conditional flag");
                require(r.getU8() == 0xEE, "house result preserves next opcode");
            }
        }
    }

    void depotAndUnlockedSpells()
    {
        Reader r{{2,0, 1,0, 0x34,0x12, 2,0, 3, 0x78,0x56, 0xEE}};
        const auto data = ProtocolGameWire::readDepotSearchResults(r, [](uint16_t id) { return id == 2; });
        require(data.size() == 2 && data[0] == std::make_tuple(uint16_t(1),uint8_t(0),uint16_t(0x1234)),
            "depot item without tier");
        require(data[1] == std::make_tuple(uint16_t(2),uint8_t(3),uint16_t(0x5678)) && r.getU8() == 0xEE,
            "depot item with tier and next opcode");
        Reader zero{{0,0,0xEE}};
        require(ProtocolGameWire::readDepotSearchResults(zero, [](uint16_t) { return false; }).empty() &&
            zero.getU8() == 0xEE, "empty depot results");
        Reader spells{{2,0, 0x34,0x12, 0xFF,0xFF, 0xEE}};
        require(ProtocolGameWire::readUnlockedSpells(spells) == std::vector<uint16_t>({0x1234,65535}) &&
            spells.getU8() == 0xEE, "unlocked spells are U16 IDs");
    }

}
int main()
{
    storeDeepLinkVariants();
    creatureTypeAndHouseResults();
    depotAndUnlockedSpells();
    spellModificationBoundaries();
    staticHouseLists();
    resourceWidths();
    capturedModesBoundary();
    chainedResourcesAndBlessings();
    std::cout << checks << " protocol wire checks passed\n";
}
