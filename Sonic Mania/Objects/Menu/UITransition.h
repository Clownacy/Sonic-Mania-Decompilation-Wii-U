#ifndef OBJ_UITRANSITION_H
#define OBJ_UITRANSITION_H

#include "SonicMania.h"

// Object Class
typedef struct {
    RSDK_OBJECT
    Entity *activeTransition;
    const char *newTag;
} ObjectUITransition;

// Entity Class
typedef struct {
    RSDK_ENTITY
    StateMachine(state);
    StateMachine(callback);
    Entity *prevEntity;
    int timer;
    int timeLimit;
    bool32 field_6C;
    Vector2 drawPos[3];
} EntityUITransition;

// Object Struct
extern ObjectUITransition *UITransition;

// Standard Entity Events
void UITransition_Update(void);
void UITransition_LateUpdate(void);
void UITransition_StaticUpdate(void);
void UITransition_Draw(void);
void UITransition_Create(void* data);
void UITransition_StageLoad(void);
void UITransition_EditorDraw(void);
void UITransition_EditorLoad(void);
void UITransition_Serialize(void);

// Extra Entity Functions
void UITransition_StartTransition(void (*callback)(void), int timeLimit);
void UITransition_MatchNewTag(void);
void UITransition_SetNewTag(const char *text);

void UITransition_DrawShapes(void);

void UITransition_State_Setup(void);
void UITransition_State_TransitionIn(void);
void UITransition_State_TransitionOut(void);

#endif //!OBJ_UITRANSITION_H