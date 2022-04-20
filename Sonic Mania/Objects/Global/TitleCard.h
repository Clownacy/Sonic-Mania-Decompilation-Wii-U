#ifndef OBJ_TITLECARD_H
#define OBJ_TITLECARD_H

#include "SonicMania.h"

// Object Class
struct ObjectTitleCard {
    RSDK_OBJECT
    uint16 aniFrames;
    StateMachine(finishedCB);
    StateMachine(suppressCB);
};

// Entity Class
struct EntityTitleCard {
    RSDK_ENTITY
    StateMachine(state);
    StateMachine(stateDraw);
    int32 actionTimer;
    int32 timer;
    Vector2 decorationPos;
    int32 stripPos[4];
    Vector2 vertMovePos[2];
    Vector2 vertTargetPos[2];
    Vector2 word2DecorVerts[4];
    Vector2 word1DecorVerts[4];
    Vector2 zoneDecorVerts[4];
    Vector2 stripVertsBlue[4];   // Using Mania Mode colour names since its better than anything else we did
    Vector2 stripVertsRed[4];    // Using Mania Mode colour names since its better than anything else we did
    Vector2 stripVertsOrange[4]; // Using Mania Mode colour names since its better than anything else we did
    Vector2 stripVertsGreen[4];  // Using Mania Mode colour names since its better than anything else we did
    Vector2 bgLCurtainVerts[4];
    Vector2 bgRCurtainVerts[4];
    TextInfo zoneName;
    int32 zoneCharPos[4];
    int32 zoneCharVel[4];
    int32 zoneXPos;
    Vector2 charPos[20];
    int32 charVel[20];
    int32 titleCardWord2;
    int32 word1Width;
    int32 word2Width;
    int32 word1XPos;
    int32 word2XPos;
    uint8 actID;
    int32 actNumScale;
    Vector2 actNumPos;
    bool32 enableIntro;
    Animator decorationAnimator;
    Animator nameLetterAnimator;
    Animator zoneLetterAnimator;
    Animator actNumbersAnimator;
    color colors[5];
};

// Object Struct
extern ObjectTitleCard *TitleCard;

// Standard Entity Events
void TitleCard_Update(void);
void TitleCard_LateUpdate(void);
void TitleCard_StaticUpdate(void);
void TitleCard_Draw(void);
void TitleCard_Create(void *data);
void TitleCard_StageLoad(void);
#if RETRO_INCLUDE_EDITOR
void TitleCard_EditorDraw(void);
void TitleCard_EditorLoad(void);
#endif
void TitleCard_Serialize(void);

// Extra Entity Functions
void TitleCard_SetupColors(void);
void TitleCard_SetupVertices(void);
void TitleCard_SetupTitleWords(void);
void TitleCard_HandleWordMovement(void);
void TitleCard_HandleZoneCharMovement(void);
void TitleCard_HandleCamera(void);

// States
void TitleCard_State_SetupBGElements(void);
void TitleCard_State_OpeningBG(void);
void TitleCard_State_EnterTitle(void);
void TitleCard_State_ShowingTitle(void);
void TitleCard_State_SlideAway(void);
void TitleCard_State_Supressed(void);

// Draw States
void TitleCard_Draw_SlideIn(void);
void TitleCard_Draw_ShowTitleCard(void);
void TitleCard_Draw_SlideAway(void);

#endif //! OBJ_TITLECARD_H
