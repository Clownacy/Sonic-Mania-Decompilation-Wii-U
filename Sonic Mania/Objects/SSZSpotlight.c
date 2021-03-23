#include "../SonicMania.h"

ObjectSSZSpotlight *SSZSpotlight;

void SSZSpotlight_Update()
{
    RSDK_THIS(SSZSpotlight);
    entity->angle    = (entity->angle + entity->speed) & 0x1FF;
    entity->rotation = RSDK.Sin512(entity->angle) >> 2;
    Vector2 *vert    = entity->vertPos;

    for (int i = 0; i < 8; ++i) {
        vert->x = vert[-8].x;
        vert->y = vert[-8].y;

        int xOff = (vert->x - entity->offsetPos.x) >> 4;
        int yOff = (vert->y - entity->offsetPos.y) >> 4;
        int x    = yOff * RSDK.Sin1024(entity->rotation) >> 6;
        vert->x  = x + (xOff * RSDK.Cos1024(entity->rotation) >> 6) + entity->offsetPos.x;
        int y    = yOff * RSDK.Cos1024(entity->rotation) >> 6;
        vert->y  = y - (xOff * RSDK.Sin1024(entity->rotation) >> 6) + entity->offsetPos.y;
        ++vert;
    }

    if (entity->flashSpeed)
        entity->alpha = (RSDK.Sin256((Zone->timer + entity->offset) << entity->flashSpeed) >> 2) + 0xC0;
}

void SSZSpotlight_LateUpdate() {}

void SSZSpotlight_StaticUpdate() {}

void SSZSpotlight_Draw()
{
    RSDK_THIS(SSZSpotlight);
    Vector2 *vertPosPtr = entity->vertPos;
    Vector2 vertPos[4];
    colour vertClrs[4];

    int screenX = RSDK_sceneInfo->entity->position.x - (RSDK_screens[RSDK_sceneInfo->currentScreenID].position.x << 16);

    for (int i = 0; i < 6; i += 2) {
        vertPos[0].x = screenX + vertPosPtr[0].x;
        vertPos[0].y = vertPosPtr[0].y;
        vertClrs[0]  = entity->vertClrPtrs[i];

        vertPos[1].x = screenX + vertPosPtr[1].x;
        vertPos[1].y = vertPosPtr[1].y;
        vertClrs[1]  = entity->vertClrPtrs[i + 1];

        vertPos[2].x = screenX + vertPosPtr[2].x;
        vertPos[2].y = vertPosPtr[2].y;
        vertClrs[2]  = entity->vertClrPtrs[i + 3];

        vertPos[3].x = screenX + vertPosPtr[3].x;
        vertPos[3].y = vertPosPtr[3].y;
        vertClrs[3]  = entity->vertClrPtrs[i + 2];

        RSDK.DrawBlendedQuad(vertPos, vertClrs, 4, entity->alpha, INK_ADD);
        vertPosPtr++;
    }
}

void SSZSpotlight_Create(void *data)
{
    RSDK_THIS(SSZSpotlight);
    if (!RSDK_sceneInfo->inEditor) {
        entity->visible = true;
        switch (entity->drawFlag) {
            default: break;
            case 0: entity->drawOrder = Zone->drawOrderLow; break;
            case 1: entity->drawOrder = Zone->drawOrderHigh; break;
            case 2: entity->drawOrder = Zone->drawOrderLow - 1; break;
        }

        entity->angle = entity->offset;

        int sizes[3];
        sizes[0]      = 4;
        sizes[1]      = 8;
        sizes[2]      = 16;
        int s         = -sizes[entity->size];
        entity->alpha = 256;
        s <<= 16;
        entity->offsetPos.y    = 0x1100000;
        entity->vertStore[0].x = s;
        entity->vertStore[1].x = s;

        entity->vertStore[2].x = -0x4000 * sizes[entity->size];
        entity->vertStore[3].x = -0x4000 * sizes[entity->size];

        entity->vertStore[4].x = sizes[entity->size] << 14;
        entity->vertStore[5].x = sizes[entity->size] << 14;

        entity->vertStore[6].x = sizes[entity->size] << 16;
        entity->vertStore[7].x = sizes[entity->size] << 16;

        Vector2 *vertPtr = entity->vertStore;
        for (int i = 0; i < 4; ++i) {
            int store  = vertPtr->x;
            vertPtr->x = 720 * (store >> 8);
            vertPtr += 2;
        }

        entity->vertStore[0].y = -0xC00000;
        entity->vertStore[2].y = -0xC00000;
        entity->vertStore[4].y = -0xC00000;
        entity->vertStore[6].y = -0xC00000;
        entity->vertStore[1].y = 0x1000000;
        entity->vertStore[3].y = 0x1000000;
        entity->vertStore[5].y = 0x1000000;
        entity->vertStore[7].y = 0x1000000;
        if (RSDK_sceneInfo->filter & FILTER_ENCORE) {
            if (entity->color)
                entity->vertClrPtrs = SSZSpotlight->coloursEncoreB;
            else
                entity->vertClrPtrs = SSZSpotlight->coloursEncoreA;
        }
        else if (entity->color) {
            entity->vertClrPtrs = SSZSpotlight->coloursManiaB;
        }
        else {
            entity->vertClrPtrs = SSZSpotlight->coloursManiaA;
        }

        entity->active        = ACTIVE_XBOUNDS;
        entity->updateRange.x = 0x1000000;
        entity->updateRange.y = 0x800000;
    }
}

void SSZSpotlight_StageLoad() {}

void SSZSpotlight_EditorDraw() {}

void SSZSpotlight_EditorLoad() {}

void SSZSpotlight_Serialize()
{
    RSDK_EDITABLE_VAR(SSZSpotlight, VAR_UINT8, color);
    RSDK_EDITABLE_VAR(SSZSpotlight, VAR_UINT8, size);
    RSDK_EDITABLE_VAR(SSZSpotlight, VAR_UINT8, drawFlag);
    RSDK_EDITABLE_VAR(SSZSpotlight, VAR_UINT8, speed);
    RSDK_EDITABLE_VAR(SSZSpotlight, VAR_UINT8, offset);
    RSDK_EDITABLE_VAR(SSZSpotlight, VAR_UINT8, flashSpeed);
}