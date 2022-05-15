// ---------------------------------------------------------------------
// RSDK Project: Sonic Mania
// Object Description: Zone Object
// Object Author: Christian Whitehead/Simon Thomley/Hunter Bridges
// Decompiled by: Rubberduckycooly & RMGRich
// ---------------------------------------------------------------------

#include "SonicMania.h"
#include <time.h>

ObjectZone *Zone;

void Zone_Update(void) {}

void Zone_LateUpdate(void)
{
    RSDK_THIS(Zone);

    if (SceneInfo->entitySlot != SLOT_ZONE) {
        StateMachine_Run(self->state);
    }
    else {
        foreach_active(Player, player)
        {
            int32 playerID = SLOT_PLAYER1;
            if (!player->sidekick)
                playerID = RSDK.GetEntityID(player);

            Hitbox *playerHitbox = Player_GetHitbox(player);

            // Left Boundary
            if (Zone->playerBoundActiveL[playerID]) {
                int32 offset = -0x10000 * playerHitbox->left;
                if (player->position.x - offset <= Zone->playerBoundsL[playerID]) {
                    player->position.x = Zone->playerBoundsL[playerID] + offset;

                    if (player->onGround) {
                        if (player->groundVel < Zone->autoScrollSpeed) {
                            player->velocity.x = Zone->autoScrollSpeed;
                            player->groundVel  = Zone->autoScrollSpeed;
                            player->pushing    = false;
                        }
                    }
                    else if (player->velocity.x < Zone->autoScrollSpeed) {
                        player->velocity.x = Zone->autoScrollSpeed;
                        player->groundVel  = 0;
                    }
                }
            }

            // Right Boundary
            if (Zone->playerBoundActiveR[playerID]) {
                int32 offset = playerHitbox->right << 16;
                if (player->position.x + offset >= Zone->playerBoundsR[playerID]) {
                    player->position.x = Zone->playerBoundsR[playerID] - offset;

                    if (player->onGround) {
                        if (player->groundVel > Zone->autoScrollSpeed) {
                            player->velocity.x = Zone->autoScrollSpeed;
                            player->groundVel  = Zone->autoScrollSpeed;
                            player->pushing    = false;
                        }
                    }
                    else {
                        if (player->velocity.x > Zone->autoScrollSpeed) {
                            player->velocity.x = Zone->autoScrollSpeed;
                            player->groundVel  = 0;
                        }
                    }
                }
            }

            // Top Boundary
            if (Zone->playerBoundActiveT[playerID]) {
                if (player->position.y - 0x140000 < Zone->playerBoundsT[playerID]) {
                    player->position.y = Zone->playerBoundsT[playerID] + 0x140000;
                    player->velocity.y = 0;
                }
            }

            // Death Boundary
            if (player->state != Player_State_Die && !player->deathType) {
                if (Zone->playerBoundsB[playerID] <= Zone->deathBoundary[playerID]) {
                    if (player->position.y > Zone->deathBoundary[playerID]) {
                        player->deathType                  = PLAYER_DEATH_DIE_NOSFX;
                        Zone->playerBoundActiveB[playerID] = false;
                    }
                }
                else if (player->position.y > Zone->playerBoundsB[playerID]) {
                    player->deathType                  = PLAYER_DEATH_DIE_NOSFX;
                    Zone->playerBoundActiveB[playerID] = false;
                }
            }

            // Bottom Boundary
            if (Zone->playerBoundActiveB[playerID]) {
                if (player->position.y + 0x140000 > Zone->playerBoundsB[playerID]) {
                    player->position.y = Zone->playerBoundsB[playerID] - 0x140000;
                    player->velocity.y = 0;
                    player->onGround   = true;
                }
            }
        }

        // Handle States
        StateMachine_Run(self->state);

        // Handle Time Overs
        if (SceneInfo->minutes == 10
#if RETRO_USE_PLUS
            && !(globals->medalMods & MEDAL_NOTIMEOVER)
#endif
        ) {
            SceneInfo->minutes      = 9;
            SceneInfo->seconds      = 59;
            SceneInfo->milliseconds = 99;
            SceneInfo->timeEnabled  = false;
            RSDK.PlaySfx(Player->sfxHurt, false, 0xFF);

            EntityCompetitionSession *session = (EntityCompetitionSession *)globals->competitionSession;
            foreach_active(Player, playerLoop)
            {
                bool32 canDie = true;
#if RETRO_USE_PLUS
                if (globals->gameMode == MODE_COMPETITION && (session->finishState[playerLoop->playerID]) == FINISHFLAG_FINISHED)
                    canDie = false;
#endif
                if (!playerLoop->sidekick && canDie)
                    playerLoop->deathType = PLAYER_DEATH_DIE_USESFX;
            }

            Zone->gotTimeOver = true;
            StateMachine_Run(Zone->timeOverCallback);
        }

#if RETRO_USE_PLUS
        // You took an hour to beat the stage... no time bonus for you!
        if (SceneInfo->minutes == 59 && SceneInfo->seconds == 59)
            ActClear->disableTimeBonus = true;
#endif

        // Player Draw order sorting
        // Ensure P1 is always on top
        if (Player->playerCount > 0) {
            EntityPlayer *sidekick = RSDK_GET_ENTITY(SLOT_PLAYER2, Player);
            if ((sidekick->state != Player_State_FlyIn && sidekick->state != Player_State_JumpIn) || sidekick->characterID == ID_TAILS
                || sidekick->scale.x == 0x200) {
                player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);
                RSDK.SwapDrawListEntries(player->drawOrder, SLOT_PLAYER1, SLOT_PLAYER2, Player->playerCount);
            }
        }
    }
}

void Zone_StaticUpdate(void)
{
    // Global timer, used to keep various objects in sync
    // This timer is reset every time a stage is loaded
    ++Zone->timer;
    Zone->timer &= 0x7FFF;

    // Persistent timer, this
    // This timer is NOT reset when a same-folder act transition happens (aka GHZ1 -> GHZ2 or CPZ1 -> CPZ2)
    // otherwise it is reset, it's used when a form of persistence is needed between acts
    ++Zone->persistentTimer;
    Zone->persistentTimer &= 0x7FFF;

    // Ring Frame timer, used to keep all the rings animating using the same frame
    if (!(Zone->timer & 1)) {
        ++Zone->ringFrame;
        Zone->ringFrame &= 0xF;
    }

#if RETRO_USE_PLUS
    // Handle times for the summary screen
    int32 zone = Zone_GetZoneID();

    if (zone >= ZONE_AIZ)
        zone = ZONE_AIZ;
    else if (zone == ZONE_INVALID)
        return;

    int32 act = Zone->actID;
    if (act >= 3)
        act = 0;

    int32 pos = act + 2 * zone;
    if (pos >= 0 && SceneInfo->timeEnabled && globals->gameMode < MODE_TIMEATTACK)
        ++SaveGame->saveRAM->zoneTimes[pos];
#endif
}

void Zone_Draw(void)
{
    RSDK_THIS(Zone);

    if (self->screenID >= PLAYER_MAX || self->screenID == SceneInfo->currentScreenID) {
        StateMachine_Run(self->stateDraw);
    }
}

void Zone_Create(void *data)
{
    RSDK_THIS(Zone);
    self->active = ACTIVE_ALWAYS;

    if (!self->stateDraw) {
        self->visible   = false;
        self->drawOrder = -1;
    }
}

void Zone_StageLoad(void)
{
#if RETRO_USE_PLUS
    // Set the random seed to a "random" value
    Zone->randSeed = (uint32)time(NULL);

    // Setup encore character flags & stock if needed
    EntitySaveGame *saveRAM = SaveGame->saveRAM;
    if (globals->gameMode == MODE_ENCORE) {
        if (globals->characterFlags == ID_NONE) {
            globals->characterFlags = 0;

            if ((globals->playerID >> 0) & 0xFF) {
                int32 charID = -1;
                for (int32 i = ((globals->playerID >> 0) & 0xFF); i > 0; ++charID) i >>= 1;

                globals->characterFlags |= 1 << charID;
            }

            if ((globals->playerID >> 8) & 0xFF) {
                int32 charID = -1;
                for (int32 i = ((globals->playerID >> 8) & 0xFF); i > 0; ++charID) i >>= 1;

                globals->characterFlags |= 1 << charID;
            }

            if ((globals->stock >> 0) & 0xFF) {
                int32 charID = -1;
                for (int32 i = ((globals->stock >> 0) & 0xFF); i > 0; ++charID) i >>= 1;

                globals->characterFlags |= 1 << charID;
            }

            if ((globals->stock >> 8) & 0xFF) {
                int32 charID = -1;
                for (int32 i = ((globals->stock >> 8) & 0xFF); i > 0; ++charID) i >>= 1;

                globals->characterFlags |= 1 << charID;
            }

            if ((globals->stock >> 16) & 0xFF) {
                int32 charID = -1;
                for (int32 i = ((globals->stock >> 16) & 0xFF); i > 0; ++charID) i >>= 1;

                globals->characterFlags |= 1 << charID;
            }

            saveRAM->playerID       = globals->playerID;
            saveRAM->characterFlags = globals->characterFlags;
        }

        if (!TitleCard || TitleCard->suppressCB != Zone_TitleCard_SupressCB) {
            globals->characterFlags = saveRAM->characterFlags;
            globals->stock          = saveRAM->stock;
            globals->playerID       = saveRAM->playerID;
        }
    }
#endif

    Zone->timer           = 0;
    Zone->autoScrollSpeed = 0;
    Zone->ringFrame       = 0;
    Zone->gotTimeOver     = false;
    Zone->vsSwapCBCount   = 0;

    // Setup draw order "constants" (shouldn't be changed after this, but can be if really needed)
    Zone->fgLayerLow     = 0;
    Zone->objectDrawLow  = 2;
    Zone->playerDrawLow  = 4;
    Zone->fgLayerHigh    = 6;
    Zone->objectDrawHigh = 8;
    Zone->playerDrawHigh = 12;
    Zone->hudDrawOrder   = 14;

    // Layer IDs
    Zone->fgLow     = RSDK.GetTileLayerID("FG Low");
    Zone->fgHigh    = RSDK.GetTileLayerID("FG High");
    Zone->moveLayer = RSDK.GetTileLayerID("Move");
#if RETRO_USE_PLUS
    Zone->scratchLayer = RSDK.GetTileLayerID("Scratch");
#endif

    // Layer Masks

    // (Not sure why this one is different from the two below, but whatever works)
    if (Zone->fgLowMask)
        Zone->fgLowMask = 1 << Zone->fgLow;

    if (Zone->fgHigh)
        Zone->fgHighMask = 1 << Zone->fgHigh;

    if (Zone->moveLayer)
        Zone->moveMask = 1 << Zone->moveLayer;

    Zone->collisionLayers = 1 << Zone->fgLow;
    Zone->collisionLayers |= 1 << Zone->fgHigh;

    // Get Layer size and setup default bounds
    Vector2 layerSize;
    RSDK.GetLayerSize(Zone->fgLow, &layerSize, true);

#if RETRO_USE_PLUS
    if (!Zone->swapGameMode) {
#endif
        for (int32 s = 0; s < PLAYER_MAX; ++s) {
            Zone->cameraBoundsL[s] = 0;
            Zone->cameraBoundsR[s] = layerSize.x;
            Zone->cameraBoundsT[s] = 0;
            Zone->cameraBoundsB[s] = layerSize.y;

            Zone->playerBoundsL[s] = Zone->cameraBoundsL[s] << 0x10;
            Zone->playerBoundsR[s] = Zone->cameraBoundsR[s] << 0x10;
            Zone->playerBoundsT[s] = Zone->cameraBoundsT[s] << 0x10;
            Zone->playerBoundsB[s] = Zone->cameraBoundsB[s] << 0x10;

            Zone->deathBoundary[s]      = Zone->cameraBoundsB[s] << 0x10;
            Zone->playerBoundActiveL[s] = true;
            Zone->playerBoundActiveB[s] = false;
        }
#if RETRO_USE_PLUS
    }
#endif

    // Setup cool bonus
    if (!globals->initCoolBonus) {
        globals->coolBonus[0]  = 10000;
        globals->coolBonus[1]  = 10000;
        globals->coolBonus[2]  = 10000;
        globals->coolBonus[3]  = 10000;
        globals->initCoolBonus = true;
    }

    // Destroy any zone entities placed in the scene
    foreach_all(Zone, entity) { destroyEntity(entity); }
    // ... and ensure we have a zone entity in the correct reserved slot
    RSDK.ResetEntitySlot(SLOT_ZONE, Zone->classID, NULL);

    // Setup Competition options (or ensure they're not active if not in competition mode)
    EntityCompetitionSession *session = (EntityCompetitionSession *)globals->competitionSession;
    if (globals->gameMode == MODE_COMPETITION) {
        if (RSDK.CheckStageFolder("Puyo")) {
            if (globals->gameMode == MODE_COMPETITION) {
                RSDK.SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);
            }
            else {
#if RETRO_USE_PLUS
                Competition_ResetOptions();
#else
                CompetitionSession_ResetOptions();
#endif
                RSDK.SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);
            }
        }
        else {
            session->playerCount = clampVal(session->playerCount, 2, PLAYER_MAX);
            RSDK.SetVideoSetting(VIDEOSETTING_SCREENCOUNT, session->playerCount);
        }
    }
    else {
#if RETRO_USE_PLUS
        Competition_ResetOptions();
#else
        CompetitionSession_ResetOptions();
#endif
        RSDK.SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);
    }

    // Setup Rich Presence for this game mode
    String message;
    switch (globals->gameMode) {
#if !RETRO_USE_PLUS
        case MODE_NOSAVE:
#endif
        case MODE_MANIA:
            Localization_GetString(&message, STR_RPC_MANIA);
            API_SetRichPresence(PRESENCE_MANIA, &message);
            break;

#if RETRO_USE_PLUS
        case MODE_ENCORE:
            Localization_GetString(&message, STR_RPC_ENCORE);
            API_SetRichPresence(PRESENCE_ENCORE, &message);
            break;

#endif
        case MODE_TIMEATTACK:
            Localization_GetString(&message, STR_RPC_TA);
            API_SetRichPresence(PRESENCE_TA, &message);
            break;

        case MODE_COMPETITION:
            Localization_GetString(&message, STR_RPC_COMP);
            API_SetRichPresence(PRESENCE_COMP, &message);
            break;

        default: break;
    }

    Zone->sfxFail = RSDK.GetSfx("Stage/Fail.wav");
}

int32 Zone_GetZoneID(void)
{
    if (RSDK.CheckStageFolder("GHZ"))
        return ZONE_GHZ;
    if (RSDK.CheckStageFolder("CPZ"))
        return ZONE_CPZ;
    if (RSDK.CheckStageFolder("SPZ1") || RSDK.CheckStageFolder("SPZ2"))
        return ZONE_SPZ;
    if (RSDK.CheckStageFolder("FBZ"))
        return ZONE_FBZ;
    if (RSDK.CheckStageFolder("PSZ1") || RSDK.CheckStageFolder("PSZ2"))
        return ZONE_PGZ;
    if (RSDK.CheckStageFolder("SSZ1") || RSDK.CheckStageFolder("SSZ2"))
        return ZONE_SSZ;
    if (RSDK.CheckStageFolder("HCZ"))
        return ZONE_HCZ;
    if (RSDK.CheckStageFolder("MSZ"))
        return ZONE_MSZ;
    if (RSDK.CheckStageFolder("OOZ1") || RSDK.CheckStageFolder("OOZ2"))
        return ZONE_OOZ;
    if (RSDK.CheckStageFolder("LRZ1") || RSDK.CheckStageFolder("LRZ2") || RSDK.CheckStageFolder("LRZ3"))
        return ZONE_LRZ;
    if (RSDK.CheckStageFolder("MMZ"))
        return ZONE_MMZ;
    if (RSDK.CheckStageFolder("TMZ1") || RSDK.CheckStageFolder("TMZ2") || RSDK.CheckStageFolder("TMZ3"))
        return ZONE_TMZ;
    if (RSDK.CheckStageFolder("ERZ"))
        return ZONE_ERZ;
#if RETRO_USE_PLUS
    if (RSDK.CheckStageFolder("AIZ") && globals->gameMode == MODE_ENCORE)
        return ZONE_AIZ;
#endif
    return ZONE_INVALID;
}

void Zone_StoreEntities(int32 xOffset, int32 yOffset)
{
    // "Normalize" the positions of players, signposts & itemboxes when we store them
    // (this is important for later)

    int32 count   = 0;
    int32 dataPos = 0;
    foreach_active(Player, player)
    {
        player->position.x -= xOffset;
        player->position.y -= yOffset;
        globals->atlEntitySlot[count] = RSDK.GetEntityID(player);
        RSDK.CopyEntity(&globals->atlEntityData[dataPos], player, false);
        count++;
        dataPos += 0x200;
    }

    foreach_active(SignPost, signPost)
    {
        signPost->position.x -= xOffset;
        signPost->position.y -= yOffset;
        globals->atlEntitySlot[count] = RSDK.GetEntityID(signPost);
        RSDK.CopyEntity(&globals->atlEntityData[dataPos], signPost, false);
        count++;
        dataPos += 0x200;
    }

    foreach_active(ItemBox, itemBox)
    {
        itemBox->position.x -= xOffset;
        itemBox->position.y -= yOffset;
        globals->atlEntitySlot[count] = RSDK.GetEntityID(itemBox);
        RSDK.CopyEntity(&globals->atlEntityData[dataPos], itemBox, false);
        count++;
        dataPos += 0x200;
    }

    // store any relevant info about the player
    EntityPlayer *player1    = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);
    globals->restartLives[0] = player1->lives;
    globals->restartScore    = player1->score;
    globals->restartPowerups = player1->shield;
    globals->atlEntityCount  = count;
    globals->atlEnabled      = true;
}

void Zone_ReloadStoredEntities(int32 xOffset, int32 yOffset, bool32 setATLBounds)
{
    // reload any stored entities we have
    for (int32 e = 0; e < globals->atlEntityCount; ++e) {
        Entity *storedEntity = (Entity *)&globals->atlEntityData[e << 9];
        Entity *entity       = NULL;

        // only players & powerups get to be overridden, everything else is just added to the temp area
        if (globals->atlEntitySlot[e] >= SLOT_ZONE)
            entity = RSDK.CreateEntity(TYPE_BLANK, NULL, 0, 0);
        else
            entity = RSDK_GET_ENTITY_GEN(globals->atlEntitySlot[e]);

        if (storedEntity->classID == Player->classID) {
            EntityPlayer *storedPlayer = (EntityPlayer *)storedEntity;
            EntityPlayer *player       = (EntityPlayer *)entity;
            player->shield             = storedPlayer->shield;

            if (player->shield && player->superState != SUPERSTATE_SUPER && player->invincibleTimer <= 0) {
                EntityShield *shield = RSDK_GET_ENTITY(Player->playerCount + RSDK.GetEntityID(player), Shield);
                RSDK.ResetEntityPtr(shield, Shield->classID, player);
            }
        }
        else {
            RSDK.CopyEntity(entity, storedEntity, false);
        }

        entity->position.x = storedEntity->position.x + xOffset;
        entity->position.y = storedEntity->position.y + yOffset;
    }

    // clear ATL data, we dont wanna do it again
    memset(globals->atlEntityData, 0, globals->atlEntityCount << 9);

    // if we're allowing the new boundary, update our camera to use ATL bounds instead of the default ones
    Zone->setATLBounds = setATLBounds;
    if (setATLBounds) {
        EntityPlayer *player   = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);
        player->camera         = NULL;
        EntityCamera *camera   = RSDK_GET_ENTITY(SLOT_CAMERA1, Camera);
        camera->position.x     = xOffset;
        camera->position.y     = yOffset;
        camera->state          = 0;
        camera->target         = NULL;
        camera->boundsL        = (xOffset >> 16) - ScreenInfo->centerX;
        camera->boundsR        = (xOffset >> 16) + ScreenInfo->centerX;
        camera->boundsT        = (yOffset >> 16) - ScreenInfo->height;
        camera->boundsB        = yOffset >> 16;
        Camera->centerBounds.x = 0x80000;
        Camera->centerBounds.y = 0x40000;
    }

    Player->savedLives      = globals->restartLives[0];
    Player->savedScore      = globals->restartScore;
    Player->powerups        = globals->restartPowerups;
    globals->atlEntityCount = 0;
}

void Zone_StartFadeOut(int32 fadeSpeed, int32 fadeColor)
{
    EntityZone *zone = RSDK_GET_ENTITY(SLOT_ZONE, Zone);

    zone->fadeColor = fadeColor;
    zone->fadeSpeed = fadeSpeed;
    zone->screenID  = PLAYER_MAX;
    zone->timer     = 0;
    zone->state     = Zone_State_Fadeout;
    zone->stateDraw = Zone_Draw_Fade;
    zone->visible   = true;
    zone->drawOrder = DRAWGROUP_COUNT - 1;
}

void Zone_StartFadeIn(int32 fadeSpeed, int32 fadeColor)
{
    EntityZone *zone = CREATE_ENTITY(Zone, NULL, 0, 0);

    zone->fadeColor = fadeColor;
    zone->fadeSpeed = fadeSpeed;
    zone->screenID  = PLAYER_MAX;
    zone->timer     = 640;
    zone->state     = Zone_State_FadeIn;
    zone->stateDraw = Zone_Draw_Fade;
    zone->visible   = true;
    zone->drawOrder = DRAWGROUP_COUNT - 1;
}

void Zone_StartFadeOut_MusicFade(int32 fadeSpeed, int32 fadeColor)
{
    EntityZone *zone = RSDK_GET_ENTITY(SLOT_ZONE, Zone);

    zone->fadeColor = fadeColor;
    zone->fadeSpeed = fadeSpeed;
    zone->screenID  = PLAYER_MAX;
    zone->timer     = 0;
    zone->state     = Zone_State_Fadeout;
    zone->stateDraw = Zone_Draw_Fade;
    zone->visible   = true;
    zone->drawOrder = DRAWGROUP_COUNT - 1;
    Music_FadeOut(0.025);
}

void Zone_StartFadeOut_Competition(int32 fadeSpeed, int32 fadeColor)
{
    EntityZone *zone = RSDK_GET_ENTITY(SLOT_ZONE, Zone);

    zone->fadeColor = fadeColor;
    zone->fadeSpeed = fadeSpeed;
    zone->screenID  = PLAYER_MAX;
    zone->timer     = 0;
    zone->state     = Zone_State_Fadeout_Competition;
    zone->stateDraw = Zone_Draw_Fade;
    zone->visible   = true;
    zone->drawOrder = DRAWGROUP_COUNT - 1;
    Music_FadeOut(0.025);
}

void Zone_RotateOnPivot(Vector2 *position, Vector2 *pivot, int32 angle)
{
    int32 x     = (position->x - pivot->x) >> 8;
    int32 y     = (position->y - pivot->y) >> 8;
    position->x = pivot->x + (y * RSDK.Sin256(angle)) + x * RSDK.Cos256(angle);
    position->y = pivot->y + (y * RSDK.Cos256(angle)) - x * RSDK.Sin256(angle);
}

void Zone_ReloadScene(int32 screen)
{
    EntityZone *entity = CREATE_ENTITY(Zone, NULL, 0, 0);

    entity->screenID  = screen;
    entity->timer     = 640;
    entity->fadeSpeed = 16;
    entity->fadeColor = 0xF0F0F0;

#if RETRO_USE_PLUS
    if (globals->gameMode != MODE_ENCORE || EncoreIntro) {
#endif
        entity->state     = Zone_State_Fadeout_Destroy;
        entity->stateDraw = Zone_Draw_Fade;
        entity->visible   = true;
        entity->drawOrder = DRAWGROUP_COUNT - 1;
#if RETRO_USE_PLUS
    }
    else {
        entity->state     = Zone_State_ReloadScene;
        entity->stateDraw = Zone_Draw_Fade;
        entity->visible   = true;
        entity->drawOrder = DRAWGROUP_COUNT - 1;
    }
#endif
}

void Zone_StartTeleportAction(void)
{
    EntityZone *entity = CREATE_ENTITY(Zone, NULL, 0, 0);

    entity->fadeColor = 0xF0F0F0;
    entity->timer     = 640;
    entity->screenID  = PLAYER_MAX;
    entity->fadeSpeed = 16;
    entity->state     = Zone_State_SwapPlayers;
    entity->stateDraw = Zone_Draw_Fade;
    entity->visible   = true;
    entity->drawOrder = DRAWGROUP_COUNT - 1;
#if RETRO_USE_PLUS
    Zone->teleportActionActive = true;
#endif
}

void Zone_ApplyWorldBounds(void)
{
    if (Zone->setATLBounds) {
        EntityCamera *camera = RSDK_GET_ENTITY(SLOT_CAMERA1, Camera);

        foreach_active(Player, player)
        {
            int32 camWorldL = camera->boundsL << 16;
            if (player->position.x - 0xA0000 <= camWorldL) {
                player->position.x = camWorldL + 0xA0000;
                if (player->onGround) {
                    if (player->groundVel < 0) {
                        player->velocity.x = 0;
                        player->groundVel  = 0;
                        player->pushing    = false;
                    }
                }
                else if (player->velocity.x < 0) {
                    player->velocity.x = 0;
                    player->groundVel  = 0;
                }
            }

            int32 camWorldR = camera->boundsR << 16;
            if (player->position.x + 0xA0000 >= camWorldR) {
                player->position.x = camWorldR - 0xA0000;
                if (player->onGround) {
                    if (player->groundVel > 0) {
                        player->velocity.x = 0;
                        player->groundVel  = 0;
                        player->pushing    = false;
                    }
                }
                else if (player->velocity.x > 0) {
                    player->velocity.x = 0;
                    player->groundVel  = 0;
                }
            }
        }
    }
}

// Generally, this is just "isAct2", however stuff like LRZ3, SSZ boss, TMZ3 & ERZ's cases prove thats not always the case
bool32 Zone_IsZoneLastAct(void)
{
    if ((RSDK.CheckStageFolder("GHZ") && Zone->actID == 1) || (RSDK.CheckStageFolder("CPZ") && Zone->actID == 1) || RSDK.CheckStageFolder("SPZ2")
        || (RSDK.CheckStageFolder("FBZ") && Zone->actID == 1) || RSDK.CheckStageFolder("PSZ2")) {
        return true;
    }

    if (RSDK.CheckStageFolder("SSZ2")) {
        if (RSDK.GetTileLayerID("Tower") < LAYER_COUNT)
            return true;
    }
    else if ((RSDK.CheckStageFolder("HCZ") && Zone->actID == 1) || (RSDK.CheckStageFolder("MSZ") && Zone->actID == 1) || RSDK.CheckStageFolder("OOZ2")
             || RSDK.CheckStageFolder("LRZ3") || (RSDK.CheckStageFolder("MMZ") && Zone->actID == 1) || RSDK.CheckStageFolder("TMZ3")
             || RSDK.CheckStageFolder("ERZ")) {
        return true;
    }

    return false;
}

#if RETRO_USE_PLUS
int32 Zone_GetEncoreStageID(void)
{
    int32 maniaListPos = SceneInfo->listPos;

    RSDK.SetScene("Mania Mode", "");
    int32 maniaOffset = maniaListPos - SceneInfo->listPos;

    RSDK.SetScene("Encore Mode", "");
    int32 encoreOffset = SceneInfo->listPos;

    int32 encoreListPos = 0;
    if (maniaOffset >= 15) {
        if (maniaOffset == 15 || maniaOffset == 16) {
            encoreListPos = encoreOffset + 15;
        }
        else {
            encoreListPos = maniaOffset + (encoreOffset - 1);
        }
    }
    else {
        encoreListPos = maniaOffset + encoreOffset;
    }
    SceneInfo->listPos = maniaListPos;

    LogHelpers_Print("Mania Mode offset %d, pos %d -> Encore Mode offset %d, pos %d", maniaOffset, maniaListPos, encoreListPos - encoreOffset,
                     encoreListPos);

    return encoreListPos;
}
int32 Zone_GetManiaStageID(void)
{
    int32 encoreListPos = SceneInfo->listPos;

    RSDK.SetScene("Encore Mode", "");
    int32 encoreOffset = encoreListPos - SceneInfo->listPos;

    RSDK.SetScene("Mania Mode", "");
    int32 maniaOffset = SceneInfo->listPos;

    int32 maniaListPos = 0;
    if (encoreOffset >= 15) {
        if (encoreOffset == 15) {
            if (checkPlayerID(ID_KNUCKLES, 1))
                maniaListPos = maniaOffset + 16;
            else
                maniaListPos = maniaOffset + 15;
        }
        else {
            maniaListPos = encoreOffset + maniaOffset + 1;
        }
    }
    else {
        maniaListPos = encoreOffset + maniaOffset;
    }
    SceneInfo->listPos = encoreListPos;

    LogHelpers_Print("Encore Mode offset %d, pos %d -> Mania Mode offset %d, pos %d", encoreOffset, encoreListPos, maniaListPos - maniaOffset,
                     maniaListPos);

    return maniaListPos;
}
#endif

void Zone_Draw_Fade(void)
{
    RSDK_THIS(Zone);
    RSDK.FillScreen(self->fadeColor, self->timer, self->timer - 0x80, self->timer - 0x100);
}

void Zone_State_Fadeout(void)
{
    RSDK_THIS(Zone);

    self->timer += self->fadeSpeed;
    if (self->timer > 1024) {
#if RETRO_USE_PLUS
        if (Zone->swapGameMode) {
            if (SceneInfo->filter == (FILTER_BOTH | FILTER_MANIA)) {
                if (RSDK.CheckValidScene())
                    SceneInfo->listPos = Zone_GetEncoreStageID();

                globals->gameMode = MODE_ENCORE;
            }
            else if (SceneInfo->filter == (FILTER_BOTH | FILTER_ENCORE)) {
                if (RSDK.CheckValidScene())
                    SceneInfo->listPos = Zone_GetManiaStageID();

                globals->gameMode = MODE_MANIA;
            }
            SceneInfo->filter ^= 6;

            globals->enableIntro         = true;
            globals->suppressAutoMusic   = true;
            globals->suppressTitlecard   = true;
            globals->restartMilliseconds = SceneInfo->milliseconds;
            globals->restartSeconds      = SceneInfo->seconds;
            globals->restartMinutes      = SceneInfo->minutes;

            EntityPlayer *player = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);
            RSDK.CopyEntity(Zone->entityStorage, player, false);
            if (player->camera)
                RSDK.CopyEntity(Zone->entityStorage[8], player->camera, false);
        }
#endif

        RSDK.LoadScene();
    }
}

void Zone_State_FadeIn(void)
{
    RSDK_THIS(Zone);

    SceneInfo->timeEnabled = true;

    if (self->timer <= 0) {
        globals->suppressAutoMusic = false;
        globals->suppressTitlecard = false;
        destroyEntity(self);
    }
    else {
        self->timer -= self->fadeSpeed;
    }
}

void Zone_State_Fadeout_Competition(void)
{
    RSDK_THIS(Zone);
    EntityCompetitionSession *session = (EntityCompetitionSession *)globals->competitionSession;

    self->timer += self->fadeSpeed;
    if (self->timer > 1024) {
        session->completedStages[session->stageIndex] = true;
#if RETRO_USE_PLUS
        session->matchID = session->prevMatchID + 1;
#else
        session->matchID++;
#endif

        RSDK.SetScene("Presentation", "Menu");
        RSDK.SetVideoSetting(VIDEOSETTING_SCREENCOUNT, 1);
        RSDK.LoadScene();
    }
}

#if RETRO_USE_PLUS
void Zone_TitleCard_SupressCB(void)
{
    RSDK_THIS(Zone);

    SceneInfo->timeEnabled = true;
    SaveGame_LoadPlayerState();
    if (Music->activeTrack != Music->restartTrackID)
        Music_TransitionTrack(Music->restartTrackID, 0.04);

    EntityZone *zone = CREATE_ENTITY(Zone, NULL, 0, 0);
    zone->screenID   = 0;
    zone->timer      = 640;
    zone->fadeSpeed  = 16;
    zone->fadeColor  = 0xF0F0F0;
    zone->state      = Zone_State_Fadeout_Destroy;
    zone->stateDraw  = Zone_Draw_Fade;
    zone->visible    = true;
    zone->drawOrder  = 15;

    globals->suppressTitlecard = false;
    TitleCard->suppressCB      = StateMachine_None;
    Player->rings              = 0;

    destroyEntity(self);
}

void Zone_State_ReloadScene(void)
{
    EntityPlayer *player1 = RSDK_GET_ENTITY(SLOT_PLAYER1, Player);

    StarPost->storedMinutes    = SceneInfo->minutes;
    StarPost->storedSeconds    = SceneInfo->seconds;
    StarPost->storedMS         = SceneInfo->milliseconds;
    globals->suppressAutoMusic = true;
    globals->suppressTitlecard = true;
    TitleCard->suppressCB      = Zone_TitleCard_SupressCB;
    SaveGame_SavePlayerState();
    Player->rings = player1->rings;

    RSDK.LoadScene();
}
#endif

void Zone_State_Fadeout_Destroy(void)
{
    RSDK_THIS(Zone);

    if (self->timer <= 0)
        destroyEntity(self);
    else
        self->timer -= self->fadeSpeed;
}

void Zone_HandlePlayerSwap(void)
{
    int32 playerBoundActiveB[PLAYER_MAX];
    int32 playerBoundActiveT[PLAYER_MAX];
    int32 playerBoundActiveR[PLAYER_MAX];
    int32 playerBoundActiveL[PLAYER_MAX];
    int32 deathBounds[PLAYER_MAX];
    int32 playerBoundsB[PLAYER_MAX];
    int32 playerBoundsT[PLAYER_MAX];
    int32 playerBoundsR[PLAYER_MAX];
    int32 playerBoundsL[PLAYER_MAX];
    int32 cameraBoundsB[PLAYER_MAX];
    int32 cameraBoundsT[PLAYER_MAX];
    int32 cameraBoundsR[PLAYER_MAX];
    int32 cameraBoundsL[PLAYER_MAX];
    int32 layerIDs[LAYER_COUNT];

#if RETRO_USE_PLUS
    for (int32 p = 0; p < Player->playerCount; ++p) {
        EntityPlayer *player = RSDK_GET_ENTITY(Zone->preSwapPlayerIDs[p], Player);
        RSDK.CopyEntity(&Zone->entityStorage[p], player, false);

        cameraBoundsL[p]      = Zone->cameraBoundsL[p];
        cameraBoundsR[p]      = Zone->cameraBoundsR[p];
        cameraBoundsT[p]      = Zone->cameraBoundsT[p];
        cameraBoundsB[p]      = Zone->cameraBoundsB[p];
        playerBoundsL[p]      = Zone->playerBoundsL[p];
        playerBoundsR[p]      = Zone->playerBoundsR[p];
        playerBoundsT[p]      = Zone->playerBoundsT[p];
        playerBoundsB[p]      = Zone->playerBoundsB[p];
        deathBounds[p]        = Zone->deathBoundary[p];
        playerBoundActiveL[p] = Zone->playerBoundActiveL[p];
        playerBoundActiveR[p] = Zone->playerBoundActiveR[p];
        playerBoundActiveT[p] = Zone->playerBoundActiveT[p];
        playerBoundActiveB[p] = Zone->playerBoundActiveB[p];

        uint8 *layerPlanes = (uint8 *)&layerIDs[2 * p];
        for (int32 l = 0; l < LAYER_COUNT; ++l) {
            TileLayer *layer = RSDK.GetTileLayer(l);
            if (layer)
                layerPlanes[l] = layer->drawLayer[Zone->preSwapPlayerIDs[p]];
            else
                layerPlanes[l] = DRAWGROUP_COUNT;
        }

        EntityCamera *camera = player->camera;
        RSDK.CopyEntity(&Zone->entityStorage[8 + p], camera, false);
        Zone->screenPosX[p] = ScreenInfo[camera->screenID].position.x;
        Zone->screenPosY[p] = ScreenInfo[camera->screenID].position.y;

        RSDK.CopyEntity(&Zone->entityStorage[4 + p], RSDK_GET_ENTITY(Player->playerCount + Zone->preSwapPlayerIDs[p], Shield), false);
        RSDK.CopyEntity(&Zone->entityStorage[12 + p], RSDK_GET_ENTITY((2 * Player->playerCount) + Zone->preSwapPlayerIDs[p], ImageTrail), false);
    }

    for (int32 p = 0; p < Player->playerCount; ++p) {
        EntityPlayer *player       = RSDK_GET_ENTITY(Zone->swappedPlayerIDs[p], Player);
        EntityPlayer *storedPlayer = (EntityPlayer *)Zone->entityStorage[p];

        void *state = storedPlayer->state;
        if (state == Player_State_Ground || state == Player_State_Air || state == Player_State_Roll || state == Player_State_ForceRoll_Ground
            || state == Player_State_ForceRoll_Air) {
            player->state           = state;
            player->nextAirState    = storedPlayer->nextAirState;
            player->nextGroundState = storedPlayer->nextGroundState;
            player->onGround        = storedPlayer->onGround;
            player->groundedStore   = storedPlayer->groundedStore;
            for (int32 i = 0; i < 8; ++i) {
                player->abilityValues[i] = storedPlayer->abilityValues[i];
                player->abilityPtrs[i]   = storedPlayer->abilityPtrs[i];
            }
            player->angle          = storedPlayer->angle;
            player->rotation       = storedPlayer->rotation;
            player->direction      = storedPlayer->direction;
            player->tileCollisions = storedPlayer->tileCollisions;
            player->interaction    = storedPlayer->interaction;
            RSDK.SetSpriteAnimation(player->aniFrames, storedPlayer->animator.animationID, &player->animator, false, 0);
        }
        else {
            player->state = Player_State_Air;
            RSDK.SetSpriteAnimation(player->aniFrames, ANI_JUMP, &player->animator, false, 0);
            player->tileCollisions = true;
            player->interaction    = true;
        }

        player->position.x      = storedPlayer->position.x;
        player->position.y      = storedPlayer->position.y;
        player->velocity.x      = storedPlayer->velocity.x;
        player->velocity.y      = storedPlayer->velocity.y;
        player->groundVel       = storedPlayer->groundVel;
        player->shield          = storedPlayer->shield;
        player->collisionLayers = storedPlayer->collisionLayers;
        player->collisionPlane  = storedPlayer->collisionPlane;
        player->collisionMode   = storedPlayer->collisionMode;
        player->invincibleTimer = storedPlayer->invincibleTimer;
        player->speedShoesTimer = storedPlayer->speedShoesTimer;
        player->blinkTimer      = storedPlayer->blinkTimer;
        player->visible         = storedPlayer->visible;
        Player_UpdatePhysicsState(player);

        Zone->cameraBoundsL[Zone->swappedPlayerIDs[p]]      = cameraBoundsL[p];
        Zone->cameraBoundsR[Zone->swappedPlayerIDs[p]]      = cameraBoundsR[p];
        Zone->cameraBoundsT[Zone->swappedPlayerIDs[p]]      = cameraBoundsT[p];
        Zone->cameraBoundsB[Zone->swappedPlayerIDs[p]]      = cameraBoundsB[p];
        Zone->playerBoundsL[Zone->swappedPlayerIDs[p]]      = playerBoundsL[p];
        Zone->playerBoundsR[Zone->swappedPlayerIDs[p]]      = playerBoundsR[p];
        Zone->playerBoundsT[Zone->swappedPlayerIDs[p]]      = playerBoundsT[p];
        Zone->playerBoundsB[Zone->swappedPlayerIDs[p]]      = playerBoundsB[p];
        Zone->deathBoundary[Zone->swappedPlayerIDs[p]]      = deathBounds[p];
        Zone->playerBoundActiveL[Zone->swappedPlayerIDs[p]] = playerBoundActiveL[p];
        Zone->playerBoundActiveR[Zone->swappedPlayerIDs[p]] = playerBoundActiveR[p];
        Zone->playerBoundActiveT[Zone->swappedPlayerIDs[p]] = playerBoundActiveT[p];
        Zone->playerBoundActiveB[Zone->swappedPlayerIDs[p]] = playerBoundActiveB[p];

        uint8 *layerPlanes = (uint8 *)&layerIDs[2 * p];
        for (int32 l = 0; l < LAYER_COUNT; ++l) {
            TileLayer *layer                            = RSDK.GetTileLayer(l);
            layer->drawLayer[Zone->swappedPlayerIDs[p]] = layerPlanes[l];
        }

        EntityCamera *camera = player->camera;
        void *camTarget      = camera->target;
        void *camState       = camera->state;
        int32 camScreen      = camera->screenID;
        RSDK.CopyEntity(camera, &Zone->entityStorage[8 + p], false);

        camera->target                          = camTarget;
        camera->screenID                        = camScreen;
        camera->state                           = camState;
        ScreenInfo[camera->screenID].position.x = Zone->screenPosX[p];
        ScreenInfo[camera->screenID].position.y = Zone->screenPosY[p];

        EntityShield *shield = RSDK_GET_ENTITY(Player->playerCount + Zone->swappedPlayerIDs[p], Shield);
        RSDK.CopyEntity(shield, &Zone->entityStorage[4 + p], false);
        shield->player = storedPlayer;

        EntityImageTrail *trail = RSDK_GET_ENTITY(Player->playerCount + Zone->swappedPlayerIDs[p], ImageTrail);
        RSDK.CopyEntity(trail, &Zone->entityStorage[12 + p], false);
        trail->player = storedPlayer;

        EntityCamera *cam = player->camera;
        if (cam) {
            cam->position.x = player->position.x;
            cam->position.y = player->position.y;
        }
        memset(&Zone->entityStorage[0 + p], 0, ENTITY_SIZE);
        memset(&Zone->entityStorage[4 + p], 0, ENTITY_SIZE);
        memset(&Zone->entityStorage[8 + p], 0, ENTITY_SIZE);
        memset(&Zone->entityStorage[12 + p], 0, ENTITY_SIZE);
    }
#else
    int32 preSwapPlayerIDs[] = { 0, 1, 1, 1 };
    int32 swappedPlayerIDs[] = { 1, 0, 0, 0 };

    EntityPlayer *storedPlayers[] = { NULL, NULL, NULL, NULL };
    EntityShield *storedShields[] = { NULL, NULL, NULL, NULL };
    EntityCamera *storedCameras[] = { NULL, NULL, NULL, NULL };
    Entity *storedPowerups[] = { NULL, NULL, NULL, NULL };
    Vector2 screenPos[4];

    // Store Player info
    for (int32 p = 0; p < Player->playerCount; ++p) {
        EntityPlayer *player = RSDK_GET_ENTITY(preSwapPlayerIDs[p], Player);

        storedPlayers[p] = (EntityPlayer *)RSDK.CreateEntity(TYPE_BLANK, NULL, 0, 0);
        RSDK.CopyEntity(storedPlayers[p], player, false);

        cameraBoundsL[p]        = Zone->cameraBoundsL[p];
        cameraBoundsR[p]        = Zone->cameraBoundsR[p];
        cameraBoundsT[p]        = Zone->cameraBoundsT[p];
        cameraBoundsB[p]        = Zone->cameraBoundsB[p];
        playerBoundsL[p]        = Zone->playerBoundsL[p];
        playerBoundsR[p]        = Zone->playerBoundsR[p];
        playerBoundsT[p]        = Zone->playerBoundsT[p];
        playerBoundsB[p]        = Zone->playerBoundsB[p];
        deathBounds[p]          = Zone->deathBoundary[p];
        playerBoundActiveL[p]   = Zone->playerBoundActiveL[p];
        playerBoundActiveR[p]   = Zone->playerBoundActiveR[p];
        playerBoundActiveT[p]   = Zone->playerBoundActiveT[p];
        playerBoundActiveB[p]   = Zone->playerBoundActiveB[p];

        uint8 *layerPlanes = (uint8 *)&layerIDs[2 * p];
        for (int32 l = 0; l < LAYER_COUNT; ++l) {
            TileLayer *layer = RSDK.GetTileLayer(l);
            if (layer)
                layerPlanes[l] = layer->drawLayer[preSwapPlayerIDs[p]];
            else
                layerPlanes[l] = DRAWGROUP_COUNT;
        }

        EntityCamera *camera = player->camera;
        storedCameras[p] = (EntityCamera *)RSDK.CreateEntity(TYPE_BLANK, NULL, 0, 0);
        RSDK.CopyEntity(storedCameras[p], camera, false);
        screenPos[p].x = ScreenInfo[camera->screenID].position.x;
        screenPos[p].y = ScreenInfo[camera->screenID].position.y;

        storedShields[p] = (EntityShield *)RSDK.CreateEntity(TYPE_BLANK, NULL, 0, 0);
        storedPowerups[p] = RSDK.CreateEntity(TYPE_BLANK, NULL, 0, 0);
        RSDK.CopyEntity(storedShields[p], RSDK_GET_ENTITY(Player->playerCount + preSwapPlayerIDs[p], Shield), false);
        RSDK.CopyEntity(storedPowerups[p], RSDK_GET_ENTITY((2 * Player->playerCount) + preSwapPlayerIDs[p], ImageTrail), false);
    }

    // Reload & Swap Player info
    for (int32 p = 0; p < Player->playerCount; ++p) {
        EntityPlayer *player = RSDK_GET_ENTITY(swappedPlayerIDs[p], Player);
        EntityPlayer *storedPlayer = (EntityPlayer *)storedPlayers[p];
        void *state = storedPlayer->state;

        if (state == Player_State_Ground || state == Player_State_Air || state == Player_State_Roll || state == Player_State_ForceRoll_Ground
            || state == Player_State_ForceRoll_Air) {
            player->state = state;
            player->nextAirState = storedPlayer->nextAirState;
            player->nextGroundState = storedPlayer->nextGroundState;
            player->onGround = storedPlayer->onGround;
            player->groundedStore = storedPlayer->groundedStore;
            for (int i = 0; i < 8; ++i) {
                player->abilityValues[i] = storedPlayer->abilityValues[i];
                player->abilityPtrs[i] = storedPlayer->abilityPtrs[i];
            }
            player->angle = storedPlayer->angle;
            player->rotation = storedPlayer->rotation;
            player->direction = storedPlayer->direction;
            player->tileCollisions = storedPlayer->tileCollisions;
            player->interaction = storedPlayer->interaction;
            RSDK.SetSpriteAnimation(player->aniFrames, storedPlayer->animator.animationID, &player->animator, false, 0);
        }
        else {
            player->state = Player_State_Air;
            RSDK.SetSpriteAnimation(player->aniFrames, ANI_JUMP, &player->animator, false, 0);
            player->tileCollisions = true;
            player->interaction = true;
        }

        player->position.x = storedPlayer->position.x;
        player->position.y = storedPlayer->position.y;
        player->velocity.x = storedPlayer->velocity.x;
        player->velocity.y = storedPlayer->velocity.y;
        player->groundVel = storedPlayer->groundVel;
        player->shield = storedPlayer->shield;
        player->collisionLayers = storedPlayer->collisionLayers;
        player->collisionPlane = storedPlayer->collisionPlane;
        player->collisionMode = storedPlayer->collisionMode;
        player->invincibleTimer = storedPlayer->invincibleTimer;
        player->speedShoesTimer = storedPlayer->speedShoesTimer;
        player->blinkTimer = storedPlayer->blinkTimer;
        player->visible = storedPlayer->visible;
        Player_UpdatePhysicsState(player);
        Zone->cameraBoundsL[swappedPlayerIDs[p]] = cameraBoundsL[p];
        Zone->cameraBoundsR[swappedPlayerIDs[p]] = cameraBoundsR[p];
        Zone->cameraBoundsT[swappedPlayerIDs[p]] = cameraBoundsT[p];
        Zone->cameraBoundsB[swappedPlayerIDs[p]] = cameraBoundsB[p];
        Zone->playerBoundsL[swappedPlayerIDs[p]] = playerBoundsL[p];
        Zone->playerBoundsR[swappedPlayerIDs[p]] = playerBoundsR[p];
        Zone->playerBoundsT[swappedPlayerIDs[p]] = playerBoundsT[p];
        Zone->playerBoundsB[swappedPlayerIDs[p]] = playerBoundsB[p];
        Zone->deathBoundary[swappedPlayerIDs[p]] = deathBounds[p];
        Zone->playerBoundActiveL[swappedPlayerIDs[p]] = playerBoundActiveL[p];
        Zone->playerBoundActiveR[swappedPlayerIDs[p]] = playerBoundActiveR[p];
        Zone->playerBoundActiveT[swappedPlayerIDs[p]] = playerBoundActiveT[p];
        Zone->playerBoundActiveB[swappedPlayerIDs[p]] = playerBoundActiveB[p];

        uint8 *layerPlanes = (uint8 *)&layerIDs[2 * p];
        for (int32 l = 0; l < LAYER_COUNT; ++l) {
            TileLayer *layer = RSDK.GetTileLayer(l);
            layer->drawLayer[swappedPlayerIDs[p]] = layerPlanes[l];
        }

        EntityCamera *camera = player->camera;
        void *camTarget = camera->target;
        void *camState = camera->state;
        int32 camScreen = camera->screenID;
        RSDK.CopyEntity(camera, storedCameras[p], false);
        camera->target = camTarget;
        camera->screenID = camScreen;
        camera->state = camState;
        ScreenInfo[camScreen].position.x = screenPos[p].x;
        ScreenInfo[camScreen].position.y = screenPos[p].y;

        EntityShield *shield = RSDK_GET_ENTITY(Player->playerCount + swappedPlayerIDs[p], Shield);
        RSDK.CopyEntity(shield, storedShields[p], false);
        shield->player = storedPlayer;

        EntityImageTrail *trail = RSDK_GET_ENTITY(Player->playerCount + swappedPlayerIDs[p], ImageTrail);
        RSDK.CopyEntity(trail, storedPowerups[p], false);
        trail->player = storedPlayer;

        EntityCamera *cam = storedPlayer->camera;
        if (cam) {
            cam->position.x = player->position.x;
            cam->position.y = player->position.y;
        }

        destroyEntity(storedPlayers[p]);
        destroyEntity(storedShields[p]);
        destroyEntity(storedCameras[p]);
        destroyEntity(storedPowerups[p]);
    }
#endif
}

void Zone_State_SwapPlayers(void)
{
    RSDK_THIS(Zone);

    if (self->timer > 512) {
        self->timer = self->timer - self->fadeSpeed;
#if RETRO_USE_PLUS
        Zone->teleportActionActive = true;
#endif
    }
    else {
#if RETRO_USE_PLUS
        Zone->swapPlayerCount = 0;
        Zone->swapPlayerID    = 0;

        for (Zone->swapPlayerID = 0; Zone->swapPlayerID < Player->playerCount; ++Zone->swapPlayerID) {
            Zone->playerSwapEnabled[Zone->swapPlayerID] = true;
            EntityPlayer *player                        = RSDK_GET_ENTITY(Zone->swapPlayerID, Player);

            if (!Player_CheckValidState(player) || !player->interaction || !player->tileCollisions)
                Zone->playerSwapEnabled[Zone->swapPlayerID] = false;

            EntityCompetition *manager = Competition->sessionManager;
            if (manager && manager->playerFinished[Zone->swapPlayerID])
                Zone->playerSwapEnabled[Zone->swapPlayerID] = false;

            for (int32 i = 0; i < Zone->vsSwapCBCount; ++i) {
                StateMachine_Run(Zone->vsSwapCB[i]);
            }

            if (Zone->playerSwapEnabled[Zone->swapPlayerID]) {
                Zone->preSwapPlayerIDs[Zone->swapPlayerCount] = Zone->swapPlayerID;
                ++Zone->swapPlayerCount;
            }
        }

        if (Zone->swapPlayerCount <= 1) {
            RSDK.PlaySfx(Zone->sfxFail, false, 255);
        }
        else {
            EntityCompetitionSession *session = (EntityCompetitionSession *)globals->competitionSession;

            uint8 playerIDs = 0;

            // TODO: idk if swapType is ever actually set anywhere
            switch (session->swapType) {
                case 0:
                    for (Zone->swapPlayerID = 1; Zone->swapPlayerID < Zone->swapPlayerCount; ++Zone->swapPlayerID) {
                        Zone->swappedPlayerIDs[Zone->swapPlayerID] = Zone->preSwapPlayerIDs[Zone->swapPlayerID - 1];
                    }
                    Zone->swappedPlayerIDs[0] = Zone->preSwapPlayerIDs[Zone->swapPlayerCount - 1];
                    break;

                case 1:
                    for (Zone->swapPlayerID = 0; Zone->swapPlayerID < Zone->swapPlayerCount; ++Zone->swapPlayerID) {
                        Zone->swappedPlayerIDs[Zone->swapPlayerID] = Zone->preSwapPlayerIDs[Zone->swapPlayerID];
                    }

                    Zone->swapPlayerID = 0;
                    for (Zone->swapPlayerID = 0; Zone->swapPlayerID < Zone->swapPlayerCount; ++Zone->swapPlayerID) {

                        Zone->swappedPlayerIDs[Zone->swapPlayerID] = Zone->preSwapPlayerIDs[RSDK.Rand(0, Zone->swapPlayerCount)];
                        while ((1 << Zone->swappedPlayerIDs[Zone->swapPlayerID]) & playerIDs)
                            Zone->swappedPlayerIDs[Zone->swapPlayerID] = Zone->preSwapPlayerIDs[RSDK.Rand(0, Zone->swapPlayerCount)];

                        if (Zone->swappedPlayerIDs[Zone->swapPlayerID] != Zone->preSwapPlayerIDs[Zone->swapPlayerID]) {
                            playerIDs |= 1 << Zone->swappedPlayerIDs[Zone->swapPlayerID++];
                            if (Zone->swapPlayerID >= Zone->swapPlayerCount)
                                break;
                        }
                        else if (Zone->swapPlayerID >= Zone->swapPlayerCount - 1) {
                            int32 id                                   = RSDK.Rand(0, Zone->swapPlayerID - 1);
                            int32 store                                = Zone->swappedPlayerIDs[id];
                            Zone->swappedPlayerIDs[id]                 = Zone->swappedPlayerIDs[Zone->swapPlayerID];
                            Zone->swappedPlayerIDs[Zone->swapPlayerID] = store;
                            playerIDs |= 1 << Zone->swappedPlayerIDs[id];
                            playerIDs |= 1 << Zone->swappedPlayerIDs[Zone->swapPlayerID++];
                        }
                    }
                    break;
            }
#else
        Zone->playerSwapEnabled = true;
        for (int32 p = 0; p < Player->playerCount; ++p) {
            EntityPlayer *player = RSDK_GET_ENTITY(p, Player);
            if (player->state == Player_State_Drown || player->state == Player_State_None || player->state == Player_State_Die || !player->interaction
                || !player->tileCollisions)
                Zone->playerSwapEnabled = false;
        }

        if (Competition->sessionManager)
            Zone->playerSwapEnabled = false;

        for (int i = 0; i < Zone->vsSwapCBCount; ++i) {
            StateMachine_Run(Zone->vsSwapCB[i]);
        }

        if (!Zone->playerSwapEnabled) {
            RSDK.PlaySfx(Zone->sfxFail, false, 255);
        }
        else {
#endif

            Zone_HandlePlayerSwap();
        }
        self->state = Zone_State_HandleSwapFadeIn;
#if RETRO_USE_PLUS
        Zone->teleportActionActive = true;
#endif
    }
}

void Zone_State_HandleSwapFadeIn(void)
{
    RSDK_THIS(Zone);

    if (self->timer <= 0) {
#if RETRO_USE_PLUS
        Zone->teleportActionActive = false;
#endif
        destroyEntity(self);
    }
    else {
        self->timer -= self->fadeSpeed;
#if RETRO_USE_PLUS
        Zone->teleportActionActive = true;
#endif
    }
}

#if RETRO_INCLUDE_EDITOR
void Zone_EditorDraw(void) {}

void Zone_EditorLoad(void)
{

    Zone->fgLayerLow     = 0;
    Zone->objectDrawLow  = 2;
    Zone->playerDrawLow  = 4;
    Zone->fgLayerHigh    = 6;
    Zone->objectDrawHigh = 8;
    Zone->playerDrawHigh = 12;
    Zone->hudDrawOrder   = 14;
}
#endif

void Zone_Serialize(void) {}
