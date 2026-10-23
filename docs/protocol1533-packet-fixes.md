# Gameplay packet alignment fixes (15.33)

Compared Rubik against ../otclient and the supplied Linux client on 2026-10-23.

- 0xEE: port the existing resource-width fix. Types 60 and 80 use U32 in
  15.33, instead of U64. Types 30..33, 86 and 87 remain U32; others U64.
- 0xA7: read chase/secure/PvP (three bytes), retaining the local fight mode.
  The captured failing frame has A7 01 01 00 D5 15 00: D5 is the next
  bestiary packet, not a fourth tactics byte. Send tactics without padding
  for 15.33; preserve Rubik's older custom 15.30 send layout.
- 0x9C: both clients already read U16 blessings plus U8 visual state.
  The Linux decoder agrees. No speculative extra fields were added.

Linux evidence: translator vtable VA 0x2f14270; dispatcher 0xb66c20;
0xEE -> slot 0x4e0 -> 0xb76530; 0xA7 -> slot 0x328 -> 0xb54fd0;
0x9C -> slot 0x1e8 -> 0xb75630. These are gameplay decoders only.

Tests exercise all 256 resource types at 15.30 and 15.33, the captured
A7/D5 boundary, a chained resource/blessings/tactics frame, legacy tactics,
and truncated data. The same header functions are used by production parsers.
No live login or disconnection cause has been verified by these offline tests.
ClientCheck/BattlEye response logic is outside these changes.

## 0xC6 follow-up from captured log

The official CyclopediaStaticHouseData decoder at 0xb95dc0 reads U32,
six U8 scalar values, then an U8 count followed by that many U32 IDs,
then an U16 count followed by that many U32 IDs. This matches protobuf
fields 1..7 (scalars), 8 and 9 (repeated uint32).
Rubik assumed the first list contained exactly one ID, even when count=0.
The captured C6 payload has an empty first list and three IDs in the second:
46021, 50311, 59047. The next opcode is 0x0B. Its UUID payload is deliberately
excluded from the regression fixture. Production now reads both list counts
for 15.33 while preserving the existing Lua house callback and older parser.
Regression coverage includes empty, single and multiple first-list entries,
second-list boundaries and a truncated ID.

For the remaining unknown 0x60 after 0x9C, the supplied log has no full frame
or earlier opcode positions. Both the schema and the Linux decoder still
agree with the existing blessing parser. No speculative changes were made.
Unknown-opcode and exception diagnostics now include the last 12 opcodes and
their byte offsets in the frame, including the opcode being rejected. They
contain no payload data. Live verification is still required for that error.

## 0x60 SpellDataModification

The follow-up log places 0x9F at 3733, 0x9C at 3766 and 0x60 at 3770.
Linux dispatch maps 0x60 at 0xb67b67 to translator slot 0x540, decoder
0xb78800. Its default message instance RTTI identifies
GameserverMessageSpellDataModification. The supplied schema confirms repeated
SpellDataModification records with uint32 spell, enum type 0..3, int32 value.
The wire decoder uses U16 count and per entry U16 spell, U8 type and I32 value.
The captured empty list (60 00 00) is followed by EE 01 and resource value 51.
Thus 0x60 is a missing real opcode, not evidence that blessing bytes shifted.
Rubik now decodes the complete list and forwards it as tuple rows
{spellId, modifierType, signedValue} to g_game.onSpellDataModification.
Modifier type meanings and UI effects remain unverified; no field meaning
is guessed and the static spell definitions are not modified.
Tests cover the captured empty-list/resource boundary, nonempty lists,
negative modifiers and truncation at every byte of a one-entry packet.

## Broad dispatcher audit follow-up

See protocol1533-audit.md and protocol1533-audit.json for all 190 Linux
opcodes, the distinction between RTTI names and correlations, changes and
remaining gaps. New tests cover all six store deeplink variants and two
tails (0xb5f3e0), depot item tiers/quantities (0xb76b10/0xb99030), summon
owner data (0xb551f0), house result conditions (0xb766e0), unlocked spells
(0xb735a0), truncation, and the captured A8/A9 boundary. Extra fixed-layout
packets were checked at 0xbae220 (0x81), 0xb761b0 (0xC5), 0xb76210 (0xDB).
The empty opcodes have factory-only dispatcher stubs and no payload decoder.
A complete dispatcher inventory is not a complete port of every payload.
