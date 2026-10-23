# Linux 15.33 protocol audit

Complete inventory of all **190 non-default dispatcher entries** in the supplied Linux ELF. 189 message names occur in the supplied protocol.proto; SessionDumpMarker does not. 179 names came from default-instance RTTI and 11 are correlated with source/schema (explicitly marked in JSON). ClientCheck response emulation is excluded.

**This is not a claim that every field of all 190 messages has been ported or verified.** Existing dispatch coverage does not prove nested payload correctness. Full gameplay decoder disassembly is retained under build/login-check/protocol-audit (175 files); 14 additional messages have no separate decoder. ClientCheck decoder contents are excluded.

Binary SHA256: `84646adbacde3aac9d2e04855cff1c8bde7cd9a3b9b6c4c0c67b2f7f84cc1aa8`. Schema SHA256: `3d382d983c5ec20bd1072249b019b02688f71dc3994921a5cd77714428673ddd`.

## Changes in this pass

- 0xA8: StoreOpenParameters, all six service variants and two trailing bytes.
- 0x94: DepotSearchResult, U16 count, item/tier and U16 quantity, instead of legacy creature helpers.
- 0x95: CreatureType, optional U32 master ID for type 3, consumed even for an unknown creature.
- 0xAE: EditGuildMessage is a string, instead of the old rule-violation U16 channel.
- 0xC3: HouseActionResult result byte precedes an optional boolean; present only for action 1/result 0.
- 0x81: TextForReport (U32 and two strings); 0xC5: GemAtelierGemRevealed (U16); 0xDB: HirelingNameChange (two U32).
- 0x2E: UnlockedSpells (U16 count and U16 IDs).
- Empty 0x9A, 0xBC, 0xC9, 0xE3, 0xF7 now dispatch callbacks; market/reward callbacks already have UI consumers.

Optional Lua callbacks expose recovered data. These parsers do not automatically implement missing feature UIs. Older wire variants remain on existing branches for the five modified existing handlers. New opcodes are relevant to 15.33.

## Remaining work

There are still **10 dispatcher entries without a Rubik handler**:

- 0x03 GameserverMessageCreatureData (decoder 0xb59360).
- 0x04 GameserverMessageSessionDumpStart (decoder 0xb55a10).
- 0x05 GameserverMessageSessionDumpMarker (decoder 0xb56c00).
- 0x2C GameserverMessageTeamFinderTeamLeader (decoder 0xb88340).
- 0x2D GameserverMessageTeamFinderTeamMember (decoder 0xb88800).
- 0x74 GameserverMessageFriendSystemData (decoder 0xb82f00).
- 0x80 GameserverMessageCharacterTradeConfiguration (decoder 0xb88c10).
- 0x99 GameserverMessageDepotSearchDetailList (decoder 0xb77050).
- 0xC2 GameserverMessageConfigureCreaturePodium (decoder 0xb62900).
- 0xCB GameserverMessageTransactionDetails (decoder 0xb6cbe0).

0x03..0x05 include creature/session-dump records, not evidence that normal live login sends them. The other missing features require their full nested formats and callbacks to be ported; no arbitrary byte skipping or fabricated empty responses were added. Nested layouts behind existing handlers also remain subject to further verification.

## Inventory

| Opcode | Linux message | Rubik handlers | Review |
|---|---|---|---|
| 0x03 | GameserverMessageCreatureData | missing | Not implemented; see remaining work |
| 0x04 | GameserverMessageSessionDumpStart | missing | Not implemented; see remaining work |
| 0x05 | GameserverMessageSessionDumpMarker | missing | Not implemented; see remaining work |
| 0x0A | GameserverMessagePendingStateEntered | parsePendingGame, parseLogin | No-payload dispatch checked |
| 0x0B | GameserverMessageReadyForSecondaryConnection | parseGMActions | Read widths compared; not a live end-to-end certification |
| 0x0F | GameserverMessageWorldEntered | parseEnterGame | No-payload dispatch checked |
| 0x14 | GameserverMessageLoginError | parseLoginError | Dispatch exists; full payload/branches not verified |
| 0x15 | GameserverMessageLoginAdvice | parseLoginAdvice | Dispatch exists; full payload/branches not verified |
| 0x16 | GameserverMessageLoginWait | parseLoginWait | Dispatch exists; full payload/branches not verified |
| 0x17 | GameserverMessageLoginSuccess | parseLogin | Dispatch exists; full payload/branches not verified |
| 0x18 | GameserverMessageSessionEndInformation | parseSessionEnd | Dispatch exists; full payload/branches not verified |
| 0x19 | GameserverMessageStoreButtonIndicators | parseStoreButtonIndicators | Dispatch exists; full payload/branches not verified |
| 0x1A | GameserverMessageBugReportsAllowed | parseBugReport | Dispatch exists; full payload/branches not verified |
| 0x1B | GameserverMessageShowMultiOfflineTrainingDialog | parseMultiOfflineTrainingDialog | No-payload dispatch checked |
| 0x1C | GameserverMessageNpcTalkParters | parseNpcChatWindow | Dispatch exists; full payload/branches not verified |
| 0x1D | GameserverMessagePing | inline | No-payload dispatch checked |
| 0x1E | GameserverMessagePingBack | parsePingBack, parsePing | No-payload dispatch checked |
| 0x1F | GameserverMessageLoginChallenge | parseLoginChallenge | Dispatch exists; full payload/branches not verified |
| 0x28 | GameserverMessageDead | parseDeath | Dispatch exists; full payload/branches not verified |
| 0x29 | GameserverMessageStash | parseSupplyStash | Dispatch exists; full payload/branches not verified |
| 0x2A | GameserverMessageSpecialContainersAvailable | parseSpecialContainer | Dispatch exists; full payload/branches not verified |
| 0x2B | GameserverMessagePartyHuntAnalyser | parsePartyAnalyzer | Dispatch exists; full payload/branches not verified |
| 0x2C | GameserverMessageTeamFinderTeamLeader | missing | Not implemented; see remaining work |
| 0x2D | GameserverMessageTeamFinderTeamMember | missing | Not implemented; see remaining work |
| 0x2E | GameserverMessageUnlockedSpells | inline | Corrected/added; wire layout checked against Linux and schema |
| 0x2F | GameserverMessageBossDifficultySelection | parseBossDifficultySelection | Dispatch exists; full payload/branches not verified |
| 0x5B | GameserverMessageTaskboard | parseTaskBoardData | Dispatch exists; full payload/branches not verified |
| 0x5C | GameserverMessageWeaponProficiencyNotification | inline | Dispatch exists; full payload/branches not verified |
| 0x5D | GameserverMessageImbuementDurations | parseImbuementDurations | Dispatch exists; full payload/branches not verified |
| 0x5E | GameserverMessagePassiveAbilityData | parsePassiveCooldown | Dispatch exists; full payload/branches not verified |
| 0x5F | GameserverMessageSkillWheel | parseOpenWheelWindow | Dispatch exists; full payload/branches not verified |
| 0x60 | GameserverMessageSpellDataModification | parseSpellDataModification | Previously corrected; regression covered |
| 0x61 | GameserverMessageBosstiaryStaticData | parseBosstiaryData | Dispatch exists; full payload/branches not verified |
| 0x62 | GameserverMessageBossSlots | parseBosstiarySlots | Dispatch exists; full payload/branches not verified |
| 0x63 | GameserverMessageClientCheck | parseClientCheck | Excluded from response emulation; existing receiver unchanged |
| 0x64 | GameserverMessageFullMap | parseMapDescription | Dispatch exists; full payload/branches not verified |
| 0x65 | GameserverMessageTopRow | parseMapMoveNorth | Dispatch exists; full payload/branches not verified |
| 0x66 | GameserverMessageRightColumn | parseMapMoveEast | Dispatch exists; full payload/branches not verified |
| 0x67 | GameserverMessageBottomRow | parseMapMoveSouth | Dispatch exists; full payload/branches not verified |
| 0x68 | GameserverMessageLeftColumn | parseMapMoveWest | Dispatch exists; full payload/branches not verified |
| 0x69 | GameserverMessageFieldData | parseUpdateTile | Dispatch exists; full payload/branches not verified |
| 0x6A | GameserverMessageCreateOnMap | parseTileAddThing | Dispatch exists; full payload/branches not verified |
| 0x6B | GameserverMessageChangeOnMap | parseTileTransformThing | Dispatch exists; full payload/branches not verified |
| 0x6C | GameserverMessageDeleteOnMap | parseTileRemoveThing | Dispatch exists; full payload/branches not verified |
| 0x6D | GameserverMessageMoveCreature | parseCreatureMove | Dispatch exists; full payload/branches not verified |
| 0x6E | GameserverMessageContainer | parseOpenContainer | Dispatch exists; full payload/branches not verified |
| 0x6F | GameserverMessageCloseContainer | parseCloseContainer | Dispatch exists; full payload/branches not verified |
| 0x70 | GameserverMessageCreateInContainer | parseContainerAddItem | Dispatch exists; full payload/branches not verified |
| 0x71 | GameserverMessageChangeInContainer | parseContainerUpdateItem | Dispatch exists; full payload/branches not verified |
| 0x72 | GameserverMessageDeleteInContainer | parseContainerRemoveItem | Dispatch exists; full payload/branches not verified |
| 0x73 | GameserverMessageBosstiary | parseBosstiaryInfo | Dispatch exists; full payload/branches not verified |
| 0x74 | GameserverMessageFriendSystemData | missing | Not implemented; see remaining work |
| 0x75 | GameserverMessageGameEvent | parseClientEvent | Dispatch exists; full payload/branches not verified |
| 0x76 | GameserverMessageInspectionList | parseCyclopediaItemDetail | Dispatch exists; full payload/branches not verified |
| 0x77 | GameserverMessageInspectionState | parseInspectionState | Read widths compared; not a live end-to-end certification |
| 0x78 | GameserverMessageSetInventory | parseAddInventoryItem | Dispatch exists; full payload/branches not verified |
| 0x79 | GameserverMessageDeleteInventory | parseRemoveInventoryItem | Dispatch exists; full payload/branches not verified |
| 0x7A | GameserverMessageNPCOffer | parseOpenNpcTrade | Dispatch exists; full payload/branches not verified |
| 0x7B | GameserverMessagePlayerGoods | parsePlayerGoods | Dispatch exists; full payload/branches not verified |
| 0x7C | GameserverMessageCloseNPCTrade | parseCloseNpcTrade | No-payload dispatch checked |
| 0x7D | GameserverMessageOwnOffer | parseOwnTrade | Dispatch exists; full payload/branches not verified |
| 0x7E | GameserverMessageCounterOffer | parseCounterTrade | Dispatch exists; full payload/branches not verified |
| 0x7F | GameserverMessageCloseTrade | parseCloseTrade | No-payload dispatch checked |
| 0x80 | GameserverMessageCharacterTradeConfiguration | missing | Not implemented; see remaining work |
| 0x81 | GameserverMessageTextForReport | inline | Corrected/added; wire layout checked against Linux and schema |
| 0x82 | GameserverMessageAmbientLight | parseWorldLight | Read widths compared; not a live end-to-end certification |
| 0x83 | GameserverMessageGraphicalEffects | parseMagicEffect | Dispatch exists; full payload/branches not verified |
| 0x84 | GameserverMessageRemoveGraphicalEffect | parseRemoveMagicEffect, parseAnimatedText | Dispatch exists; full payload/branches not verified |
| 0x85 | GameserverMessageSoundTrigger | parseAnthem, parseDistanceMissile | Dispatch exists; full payload/branches not verified |
| 0x86 | GameserverMessageExaltationBaseData | parseItemClasses, parseCreatureMark | Dispatch exists; full payload/branches not verified |
| 0x87 | GameserverMessageExaltationDialogRefresh | parseOpenForge, parseTrappers | Dispatch exists; full payload/branches not verified |
| 0x88 | GameserverMessageExaltationHistory | parseBrowseForgeHistory | Dispatch exists; full payload/branches not verified |
| 0x89 | GameserverMessageCloseExaltationDialog | parseCloseForgeWindow | No-payload dispatch checked |
| 0x8A | GameserverMessageExaltationResult | parseForgeResult | Dispatch exists; full payload/branches not verified |
| 0x8B | GameserverMessageCreatureUpdate | parseCreatureData | Dispatch exists; full payload/branches not verified |
| 0x8C | GameserverMessageCreatureHealth | parseCreatureHealth | Dispatch exists; full payload/branches not verified |
| 0x8D | GameserverMessageCreatureLight | parseCreatureLight | Dispatch exists; full payload/branches not verified |
| 0x8E | GameserverMessageCreatureOutfit | parseCreatureOutfit | Dispatch exists; full payload/branches not verified |
| 0x8F | GameserverMessageCreatureSpeed | parseCreatureSpeed | Dispatch exists; full payload/branches not verified |
| 0x90 | GameserverMessageCreatureSkull | parseCreatureSkulls | Dispatch exists; full payload/branches not verified |
| 0x91 | GameserverMessageCreatureParty | parseCreatureShields | Dispatch exists; full payload/branches not verified |
| 0x92 | GameserverMessageCreatureUnpass | parseCreatureUnpass | Dispatch exists; full payload/branches not verified |
| 0x93 | GameserverMessageCreatureMarks | parseCreaturesMark | Dispatch exists; full payload/branches not verified |
| 0x94 | GameserverMessageDepotSearchResult | parseDepotSearchResult, parsePlayerHelpers | Corrected/added; wire layout checked against Linux and schema |
| 0x95 | GameserverMessageCreatureType | parseCreatureType | Corrected/added; wire layout checked against Linux and schema |
| 0x96 | GameserverMessageEditText | parseEditText | Dispatch exists; full payload/branches not verified |
| 0x97 | GameserverMessageEditList | parseEditList | Read widths compared; not a live end-to-end certification |
| 0x98 | GameserverMessageShowGameNews | parseGameNews | Dispatch exists; full payload/branches not verified |
| 0x99 | GameserverMessageDepotSearchDetailList | missing | Not implemented; see remaining work |
| 0x9A | GameserverMessageCloseDepotSearch | inline | Corrected/added; wire layout checked against Linux and schema |
| 0x9B | GameserverMessageBlessingsDialog | parseBlessDialog | Dispatch exists; full payload/branches not verified |
| 0x9C | GameserverMessageBlessings | parseBlessings | Read widths compared; not a live end-to-end certification |
| 0x9D | GameserverMessageSwitchPreset | parsePreset | Read widths compared; not a live end-to-end certification |
| 0x9E | GameserverMessagePremiumTrigger | parsePremiumTrigger | Read widths compared; not a live end-to-end certification |
| 0x9F | GameserverMessagePlayerDataBasic | parsePlayerInfo | Read widths compared; not a live end-to-end certification |
| 0xA0 | GameserverMessagePlayerDataCurrent | parsePlayerStats | Dispatch exists; full payload/branches not verified |
| 0xA1 | GameserverMessagePlayerSkills | parsePlayerSkills | Dispatch exists; full payload/branches not verified |
| 0xA2 | GameserverMessagePlayerState | parsePlayerState | Dispatch exists; full payload/branches not verified |
| 0xA3 | GameserverMessageUpdateTarget | parsePlayerCancelAttack | Read widths compared; not a live end-to-end certification |
| 0xA4 | GameserverMessageSpellDelay | parseSpellCooldown | Read widths compared; not a live end-to-end certification |
| 0xA5 | GameserverMessageSpellGroupDelay | parseSpellGroupCooldown | Read widths compared; not a live end-to-end certification |
| 0xA6 | GameserverMessageMultiUseDelay | parseMultiUseCooldown | Read widths compared; not a live end-to-end certification |
| 0xA7 | GameserverMessageSetTactics | parsePlayerModes | Previously corrected; regression covered |
| 0xA8 | GameserverMessageSetStoreButtonDeeplink | parseSetStoreDeepLink | Corrected/added; wire layout checked against Linux and schema |
| 0xA9 | GameserverMessageRestingAreaState | parseRestingAreaState | Read widths compared; not a live end-to-end certification |
| 0xAA | GameserverMessageTalk | parseTalk | Dispatch exists; full payload/branches not verified |
| 0xAB | GameserverMessageChannels | parseChannelList | Dispatch exists; full payload/branches not verified |
| 0xAC | GameserverMessageOpenChannel | parseOpenChannel | Dispatch exists; full payload/branches not verified |
| 0xAD | GameserverMessagePrivateChannel | parseOpenPrivateChannel | Dispatch exists; full payload/branches not verified |
| 0xAE | GameserverMessageEditGuildMessage | parseRuleViolationChannel | Corrected/added; wire layout checked against Linux and schema |
| 0xAF | GameserverMessageXpChanged | parseExperienceTracker, parseRuleViolationRemove | Dispatch exists; full payload/branches not verified |
| 0xB0 | GameserverMessageOutfitMemorial | parseRuleViolationCancel | Dispatch exists; full payload/branches not verified |
| 0xB1 | GameserverMessageHighscores | parseHighscores, parseRuleViolationLock | Dispatch exists; full payload/branches not verified |
| 0xB2 | GameserverMessageOpenOwnChannel | parseOpenOwnPrivateChannel | Dispatch exists; full payload/branches not verified |
| 0xB3 | GameserverMessageCloseChannel | parseCloseChannel | Dispatch exists; full payload/branches not verified |
| 0xB4 | GameserverMessageMessage | parseTextMessage | Dispatch exists; full payload/branches not verified |
| 0xB5 | GameserverMessageSnapBack | parseCancelWalk | Dispatch exists; full payload/branches not verified |
| 0xB6 | GameserverMessageWait | parseWalkWait | Dispatch exists; full payload/branches not verified |
| 0xB7 | GameserverMessageUnjustifiedPoints | parseUnjustifiedStats | Dispatch exists; full payload/branches not verified |
| 0xB8 | GameserverMessagePvpSituations | parsePvpSituations | Dispatch exists; full payload/branches not verified |
| 0xB9 | GameserverMessageBestiaryTracker | parseBestiaryTracker | Dispatch exists; full payload/branches not verified |
| 0xBA | GameserverMessageSoulSealsDialog | parseTaskHuntingBasicData | Dispatch exists; full payload/branches not verified |
| 0xBB | GameserverMessageShapedPerkReshapeOffers | parseWeaponProficiencyReshapeOffers, parseTaskHuntingData | Dispatch exists; full payload/branches not verified |
| 0xBC | GameserverMessageStop | inline | Corrected/added; wire layout checked against Linux and schema |
| 0xBD | GameserverMessageBossTracking | parseBosstiaryCooldownTimer | Read widths compared; not a live end-to-end certification |
| 0xBE | GameserverMessageTopFloor | parseFloorChangeUp | Dispatch exists; full payload/branches not verified |
| 0xBF | GameserverMessageBottomFloor | parseFloorChangeDown | Dispatch exists; full payload/branches not verified |
| 0xC0 | GameserverMessageUpdateManagedContainers | parseLootContainers | Dispatch exists; full payload/branches not verified |
| 0xC1 | GameserverMessageVocationSpecificPlayerData | parseMonkData | Dispatch exists; full payload/branches not verified |
| 0xC2 | GameserverMessageConfigureCreaturePodium | missing | Not implemented; see remaining work |
| 0xC3 | GameserverMessageCyclopediaHouseActionResult | parseCyclopediaHouseAuctionMessage | Corrected/added; wire layout checked against Linux and schema |
| 0xC4 | GameserverMessageWeaponProficiency | parseWeaponProficiencyInfo | Dispatch exists; full payload/branches not verified |
| 0xC5 | GameserverMessageGemAtelierGemRevealed | inline | Corrected/added; wire layout checked against Linux and schema |
| 0xC6 | GameserverMessageCyclopediaStaticHouseData | parseCyclopediaHousesInfo | Previously corrected; regression covered |
| 0xC7 | GameserverMessageCyclopediaCurrentHouseData | parseCyclopediaHouseList | Dispatch exists; full payload/branches not verified |
| 0xC8 | GameserverMessageOutfit | parseOpenOutfitWindow | Dispatch exists; full payload/branches not verified |
| 0xC9 | GameserverMessageExivaSuppressed | inline | Corrected/added; wire layout checked against Linux and schema |
| 0xCA | GameserverMessageUpdateExivaOptions | parseExivaRestrictions | Dispatch exists; full payload/branches not verified |
| 0xCB | GameserverMessageTransactionDetails | missing | Not implemented; see remaining work |
| 0xCC | GameserverMessageImpactTracking | parseUpdateImpactTracker | Dispatch exists; full payload/branches not verified |
| 0xCD | GameserverMessageMarketStatistics | parseItemsPrice | Dispatch exists; full payload/branches not verified |
| 0xCE | GameserverMessageItemWasted | parseUpdateSupplyTracker | Dispatch exists; full payload/branches not verified |
| 0xCF | GameserverMessageItemLooted | parseUpdateLootTracker | Dispatch exists; full payload/branches not verified |
| 0xD0 | GameserverMessageTrackQuestFlags | parseQuestTracker | Dispatch exists; full payload/branches not verified |
| 0xD1 | GameserverMessageKillTracking | parseKillTracker | Dispatch exists; full payload/branches not verified |
| 0xD2 | GameserverMessageBuddyData | parseVipAdd | Dispatch exists; full payload/branches not verified |
| 0xD3 | GameserverMessageBuddyStatusChange | parseVipState | Dispatch exists; full payload/branches not verified |
| 0xD4 | GameserverMessageBuddyGroupData | parseVipLogout | Dispatch exists; full payload/branches not verified |
| 0xD5 | GameserverMessageMonsterCyclopedia | parseBestiaryRaces | Dispatch exists; full payload/branches not verified |
| 0xD6 | GameserverMessageMonsterCyclopediaMonsters | parseBestiaryOverview | Dispatch exists; full payload/branches not verified |
| 0xD7 | GameserverMessageMonsterCyclopediaRace | parseBestiaryMonsterData | Dispatch exists; full payload/branches not verified |
| 0xD8 | GameserverMessageMonsterCyclopediaBonusEffects | parseBestiaryCharmsData | Dispatch exists; full payload/branches not verified |
| 0xD9 | GameserverMessageMonsterCyclopediaNewDetails | parseBestiaryEntryChanged | Dispatch exists; full payload/branches not verified |
| 0xDA | GameserverMessageCyclopediaCharacterInfo | parseCyclopediaCharacterInfo | Dispatch exists; full payload/branches not verified |
| 0xDB | GameserverMessageHirelingNameChange | inline | Corrected/added; wire layout checked against Linux and schema |
| 0xDC | GameserverMessageTutorialHint | parseTutorialHint | Read widths compared; not a live end-to-end certification |
| 0xDD | GameserverMessageCyclopediaMapData | parseAutomapFlag | Dispatch exists; full payload/branches not verified |
| 0xDE | GameserverMessageDailyRewardCollectionState | parseDailyRewardCollectionState | Read widths compared; not a live end-to-end certification |
| 0xDF | GameserverMessageCreditBalance | parseCoinBalance | Dispatch exists; full payload/branches not verified |
| 0xE0 | GameserverMessageStoreError | parseStoreError | Dispatch exists; full payload/branches not verified |
| 0xE1 | GameserverMessageRequestPurchaseData | parseRequestPurchaseData | Dispatch exists; full payload/branches not verified |
| 0xE2 | GameserverMessageOpenRewardWall | parseOpenRewardWall | Dispatch exists; full payload/branches not verified |
| 0xE3 | GameserverMessageCloseRewardWall | inline | Corrected/added; wire layout checked against Linux and schema |
| 0xE4 | GameserverMessageDailyRewardBasic | parseDailyReward | Dispatch exists; full payload/branches not verified |
| 0xE5 | GameserverMessageDailyRewardHistory | parseRewardHistory | Dispatch exists; full payload/branches not verified |
| 0xE6 | GameserverMessageBosstiaryHighlight | parseBosstiaryEntryChanged, parsePreyFreeRerolls | Read widths compared; not a live end-to-end certification |
| 0xE7 | GameserverMessagePreyTimeLeft | parsePreyTimeLeft | Dispatch exists; full payload/branches not verified |
| 0xE8 | GameserverMessagePreyData | parsePreyData | Dispatch exists; full payload/branches not verified |
| 0xE9 | GameserverMessagePreyPrices | parsePreyRerollPrice | Dispatch exists; full payload/branches not verified |
| 0xEA | GameserverMessageOfferDescription | parseShowDescription | Dispatch exists; full payload/branches not verified |
| 0xEB | GameserverMessageImbuingDialogRefresh | parseImbuementWindow | Dispatch exists; full payload/branches not verified |
| 0xEC | GameserverMessageCloseImbuingDialog | parseCloseImbuementWindow | No-payload dispatch checked |
| 0xED | GameserverMessageShowMessageDialog | parseError | Dispatch exists; full payload/branches not verified |
| 0xEE | GameserverMessageResourceBalance | parseResourceBalance | Previously corrected; regression covered |
| 0xEF | GameserverMessageTibiaTime | parseWorldTime | Read widths compared; not a live end-to-end certification |
| 0xF0 | GameserverMessageQuestLog | parseQuestLog | Dispatch exists; full payload/branches not verified |
| 0xF1 | GameserverMessageQuestLine | parseQuestLine | Dispatch exists; full payload/branches not verified |
| 0xF2 | GameserverMessageUpdatingShopBalance | parseCoinBalanceUpdating | Dispatch exists; full payload/branches not verified |
| 0xF3 | GameserverMessageChannelEvent | parseChannelEvent | Dispatch exists; full payload/branches not verified |
| 0xF4 | GameserverMessageObjectInfo | parseItemInfo | Dispatch exists; full payload/branches not verified |
| 0xF5 | GameserverMessagePlayerInventory | parsePlayerInventory | Dispatch exists; full payload/branches not verified |
| 0xF6 | GameserverMessageMarketEnter | parseMarketEnter | Dispatch exists; full payload/branches not verified |
| 0xF7 | GameserverMessageMarketLeave | inline | Corrected/added; wire layout checked against Linux and schema |
| 0xF8 | GameserverMessageMarketDetail | parseMarketDetail | Dispatch exists; full payload/branches not verified |
| 0xF9 | GameserverMessageMarketBrowse | parseMarketBrowse | Dispatch exists; full payload/branches not verified |
| 0xFA | GameserverMessageShowModalDialog | parseModalDialog | Dispatch exists; full payload/branches not verified |
| 0xFB | GameserverMessageStoreCategories | parseStore | Dispatch exists; full payload/branches not verified |
| 0xFC | GameserverMessageStoreOffers | parseStoreOffers | Dispatch exists; full payload/branches not verified |
| 0xFD | GameserverMessageTransactionHistory | parseStoreTransactionHistory | Dispatch exists; full payload/branches not verified |
| 0xFE | GameserverMessageStoreSuccess | parseCompleteStorePurchase | Dispatch exists; full payload/branches not verified |
