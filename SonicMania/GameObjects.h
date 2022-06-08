#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#ifdef _MSC_VER
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif

#define minVal(a, b)                      ((a) < (b) ? (a) : (b))
#define maxVal(a, b)                      ((a) > (b) ? (a) : (b))
#define clampVal(value, minimum, maximum) (((value) < (minimum)) ? (minimum) : (((value) > (maximum)) ? (maximum) : (value)))
#define fabs(a)                           ((a) > 0 ? (a) : -(a))

#define setBit(value, set, pos) ((value) ^= (-(int32)(set) ^ (value)) & (1 << (pos)))
#define getBit(b, pos)          ((b) >> (pos)&1)

#define intToVoid(x)   (void *)(size_t)(x)
#define floatToVoid(x) intToVoid(*(int32 *)&(x))
#define voidToInt(x)   (int32)(size_t)(x)
#define voidToFloat(x) *(float *)&(x)

#define unused(x) (void)x

#if RETRO_USE_MOD_LOADER
typedef enum {
    MODCB_GAME_STARTUP,
    MODCB_STAGELOAD,
    MODCB_ONUPDATE,
    MODCB_ONLATEUPDATE,
    MODCB_ONSTATICUPDATE,
    MODCB_ONDRAW,
    MODCB_STAGEUNLOAD,
    MODCB_ONSHADERLOAD,
} ModCallbackEvents;

typedef enum {
    SUPER_UPDATE,
    SUPER_LATEUPDATE,
    SUPER_STATICUPDATE,
    SUPER_DRAW,
    SUPER_CREATE,
    SUPER_STAGELOAD,
    SUPER_EDITORDRAW,
    SUPER_EDITORLOAD,
    SUPER_SERIALIZE
} ModSuper;

// Mod Table
typedef struct {
    // Registration & Core
    void (*RegisterGlobals)(const char *globalsPath, void **globals, uint32 size);
    void (*RegisterObject)(Object **staticVars, const char *name, uint32 entityClassSize, uint32 staticClassSize, void (*update)(void),
                           void (*lateUpdate)(void), void (*staticUpdate)(void), void (*draw)(void), void (*create)(void *), void (*stageLoad)(void),
                           void (*editorDraw)(void), void (*editorLoad)(void), void (*serialize)(void), const char *inherited);
    void *RegisterObject_STD;
    void *(*GetGlobals)(void);
    void (*Super)(int32 classID, ModSuper callback, void *data);

    // Mod Info
    bool32 (*LoadModInfo)(const char *id, String *name, String *description, String *version, bool32 *active);
    void (*GetModPath)(const char *id, String *result);
    int32 (*GetModCount)(bool32 active);
    const char *(*GetModIDByIndex)(uint32 index);
    bool32 (*ForeachModID)(String *id);

    // Mod Callbacks & Public Functions
    void (*AddModCallback)(int32 callbackID, void (*callback)(void *));
    void *AddModCallback_STD;
    void (*AddPublicFunction)(const char *functionName, void *functionPtr);
    void *(*GetPublicFunction)(const char *id, const char *functionName);

    // Mod Settings
    bool32 (*GetSettingsBool)(const char *id, const char *key, bool32 fallback);
    int32 (*GetSettingsInteger)(const char *id, const char *key, int32 fallback);
    void (*GetSettingsString)(const char *id, const char *key, String *result, const char *fallback);
    void (*SetSettingsBool)(const char *key, bool32 val);
    void (*SetSettingsInteger)(const char *key, int32 val);
    void (*SetSettingsString)(const char *key, String *val);
    void (*SaveSettings)(void);

    // Config
    bool32 (*GetConfigBool)(const char *key, bool32 fallback);
    int32 (*GetConfigInteger)(const char *key, int32 fallback);
    void (*GetConfigString)(const char *key, String *result, const char *fallback);
    bool32 (*ForeachConfig)(String *textInfo);
    bool32 (*ForeachConfigCategory)(String *textInfo);

    Object *(*GetObject)(const char *name);

    // Achievements
    void (*RegisterAchievement)(const char *identifier, const char *name, const char *desc);
    void (*GetAchievementInfo)(uint32 id, String *name, String *description, String *identifer, bool32 *achieved);
    int32 (*GetAchievementIndexByID)(const char *identifier);
    int32 (*GetAchievementCount)(void);

    // Shaders
    void (*LoadShader)(const char *shaderName, bool32 linear);
} ModFunctionTable;
#endif

#if RETRO_REV02
// API Table
typedef struct {
    // API Core
    int32 (*GetUserLanguage)(void);
    bool32 (*GetConfirmButtonFlip)(void);
    void (*ExitGame)(void);
    void (*LaunchManual)(void);
    bool32 (*IsOverlayEnabled)(uint32 inputID);
    bool32 (*CheckDLC)(int32 dlc);
#if MANIA_USE_EGS
    bool32 (*SetupExtensionOverlay)(void);
    bool32 (*CanShowExtensionOverlay)(int32 overlay);
#endif
    bool32 (*ShowExtensionOverlay)(int32 overlay);
#if MANIA_USE_EGS
    bool32 (*CanShowAltExtensionOverlay)(int32 overlay);
    bool32 (*ShowAltExtensionOverlay)(int32 overlay);
    int32 (*GetConnectingStringID)(void);
    void (*ShowLimitedVideoOptions)(int32 id);
#endif

    // Achievements
    void (*UnlockAchievement)(AchievementID *id);
    bool32 (*GetAchievementsEnabled)(void);
    void (*SetAchievementsEnabled)(bool32 enabled);
#if MANIA_USE_EGS
    bool32 (*CheckAchievementsEnabled)(void);
    void (*SetAchievementNames)(String **names, int32 count);
#endif

    // Leaderboards
#if MANIA_USE_EGS
    bool32 (*CheckLeaderboardsEnabled)(void);
#endif
    void (*InitLeaderboards)(void);
    void (*FetchLeaderboard)(LeaderboardID *leaderboard, bool32 isUser);
    void (*TrackScore)(LeaderboardID *leaderboard, int32 score, void (*callback)(bool32 success, int32 rank));
    int32 (*GetLeaderboardsStatus)(void);
    LeaderboardAvail (*LeaderboardEntryViewSize)(void);
    LeaderboardAvail (*LeaderboardEntryLoadSize)(void);
    void (*LoadLeaderboardEntries)(int32 start, uint32 end, int32 type);
    void (*ResetLeaderboardInfo)(void);
    LeaderboardEntry *(*ReadLeaderboardEntry)(uint32 entryID);

    // Rich Presence
    void (*SetRichPresence)(int32 id, String *text);

    // Stats
    void (*TryTrackStat)(StatInfo *stat);
    bool32 (*GetStatsEnabled)(void);
    void (*SetStatsEnabled)(bool32 enabled);

    // Authorization
    void (*ClearPrerollErrors)(void);
    void (*TryAuth)(void);
    int32 (*GetUserAuthStatus)(void);
    bool32 (*GetUsername)(String *userName);

    // Storage
    void (*TryInitStorage)(void);
    int32 (*GetStorageStatus)(void);
    int32 (*GetSaveStatus)(void);
    void (*ClearSaveStatus)(void);
    void (*SetSaveStatusContinue)(void);
    void (*SetSaveStatusOK)(void);
    void (*SetSaveStatusForbidden)(void);
    void (*SetSaveStatusError)(void);
    void (*SetNoSave)(bool32 noSave);
    bool32 (*GetNoSave)(void);

    // User File Management
    void (*LoadUserFile)(const char *name, void *buffer, int32 size, void (*callback)(int32 status));                  // load user file from game dir
    void (*SaveUserFile)(const char *name, void *buffer, int32 size, void (*callback)(int32 status), bool32 compressed); // save user file to game dir
    void (*DeleteUserFile)(const char *name, void (*callback)(int32 status)); // delete user file from game dir

    // User DBs
    uint16 (*InitUserDB)(const char *name, ...);
    uint16 (*LoadUserDB)(const char *filename, void (*callback)(int32 status));
    void (*SaveUserDB)(uint16 tableID, void (*callback)(int32 status));
    void (*ClearUserDB)(uint16 tableID);
    void (*ClearAllUserDBs)(void);
    void (*SetupUserDBRowSorting)(uint16 tableID);
    bool32 (*GetUserDBRowsChanged)(uint16 tableID);
    void (*AddRowSortFilter)(uint16 tableID, int32 type, const char *name, void *value);
    void (*SortDBRows)(uint16 tableID, int32 type, const char *name, bool32 sortAscending);
    int32 (*GetSortedUserDBRowCount)(uint16 tableID);
    int32 (*GetSortedUserDBRowID)(uint16 tableID, uint16 row);
    int32 (*AddUserDBRow)(uint16 tableID);
    void (*SetUserDBValue)(uint16 tableID, int32 row, int32 type, const char *name, void *value);
    void (*GetUserDBValue)(uint16 tableID, int32 row, int32 type, const char *name, void *value);
    uint32 (*GetUserDBRowUUID)(uint16 tableID, uint16 row);
    int32 (*GetUserDBRowByID)(uint16 tableID, uint32 uuid);
    void (*GetUserDBRowCreationTime)(uint16 tableID, uint16 row, char *buffer, uint32 bufferSize, const char *format);
    void (*RemoveDBRow)(uint16 tableID, uint16 row);
    void (*RemoveAllDBRows)(uint16 tableID);
} APIFunctionTable;
#endif

// Function Table
typedef struct {
    // Registration
    void (*RegisterGlobalVariables)(void **globals, int32 size);
    void (*RegisterObject)(Object **staticVars, const char *name, uint32 entityClassSize, uint32 staticClassSize, void (*update)(void),
                           void (*lateUpdate)(void), void (*staticUpdate)(void), void (*draw)(void), void (*create)(void *), void (*stageLoad)(void),
                           void (*editorDraw)(void), void (*editorLoad)(void), void (*serialize)(void));
#if RETRO_REV02
    void (*RegisterStaticVariables)(void **varClass, const char *name, uint32 classSize);
#endif

    // Entities & Objects
    bool32 (*GetActiveEntities)(uint16 group, Entity **entity);
    bool32 (*GetEntities)(uint16 classID, Entity **entity);
    void (*BreakForeachLoop)(void);
    void (*SetEditableVar)(uint8 type, const char *name, uint8 classID, int32 storeOffset);
    void *(*GetEntity)(uint16 slot);
    int32 (*GetEntitySlot)(void *entity);
    int32 (*GetEntityCount)(uint16 classID, bool32 isActive);
    int32 (*GetDrawListRef)(uint8 drawGroup, uint16 entitySlot);
    void *(*GetDrawListRefPtr)(uint8 drawGroup, uint16 entitySlot);
    void (*ResetEntityPtr)(void *entity, uint16 classID, void *data);
    void (*ResetEntitySlot)(uint16 slot, uint16 classID, void *data);
    Entity *(*CreateEntity)(uint16 classID, void *data, int32 x, int32 y);
    void (*CopyEntity)(void *destEntity, void *srcEntity, bool32 clearSrcEntity);
    bool32 (*CheckOnScreen)(void *entity, Vector2 *range);
    bool32 (*CheckPosOnScreen)(Vector2 *position, Vector2 *range);
    void (*AddDrawListRef)(uint8 drawGroup, uint16 entityID);
    void (*SwapDrawListEntries)(uint8 drawGroup, uint16 startSlotID, uint16 endSlotID, uint16 count);
    void (*SetDrawLayerProperties)(uint8 drawGroup, bool32 sorted, void (*callback)(void));

    // Scene Management
    void (*SetScene)(const char *categoryName, const char *sceneName);
    void (*SetEngineState)(uint8 state);
#if RETRO_REV02
    void (*ForceHardReset)(bool32 shouldHardReset);
#endif
    bool32 (*CheckValidScene)(void);
    int32 (*CheckStageFolder)(const char *folderName);
    int32 (*LoadScene)(void);
    int32 (*FindObject)(const char *name);

    // Cameras
    void (*ClearCameras)(void);
    void (*AddCamera)(Vector2 *targetPos, int32 offsetX, int32 offsetY, bool32 worldRelative);

    // API (Rev01 only)
#if !RETRO_REV02
    void *(*GetAPIFunction)(const char *funcName);
#endif

    // Window/Video Settings
    int32 (*GetVideoSetting)(int32 id);
    void (*SetVideoSetting)(int32 id, int32 value);
    void (*UpdateWindow)(void);

    // Math
    int32 (*Sin1024)(int32 angle);
    int32 (*Cos1024)(int32 angle);
    int32 (*Tan1024)(int32 angle);
    int32 (*ASin1024)(int32 angle);
    int32 (*ACos1024)(int32 angle);
    int32 (*Sin512)(int32 angle);
    int32 (*Cos512)(int32 angle);
    int32 (*Tan512)(int32 angle);
    int32 (*ASin512)(int32 angle);
    int32 (*ACos512)(int32 angle);
    int32 (*Sin256)(int32 angle);
    int32 (*Cos256)(int32 angle);
    int32 (*Tan256)(int32 angle);
    int32 (*ASin256)(int32 angle);
    int32 (*ACos256)(int32 angle);
    int32 (*Rand)(int32 min, int32 max);
    int32 (*RandSeeded)(int32 min, int32 max, int32 *randSeed);
    void (*SetRandSeed)(int32 key);
    uint8 (*ATan2)(int32 x, int32 y);

    // Matrices
    void (*SetIdentityMatrix)(Matrix *matrix);
    void (*MatrixMultiply)(Matrix *dest, Matrix *matrixA, Matrix *matrixB);
    void (*MatrixTranslateXYZ)(Matrix *matrix, int32 x, int32 y, int32 z, bool32 setIdentity);
    void (*MatrixScaleXYZ)(Matrix *matrix, int32 scaleX, int32 scaleY, int32 scaleZ);
    void (*MatrixRotateX)(Matrix *matrix, int32 angle);
    void (*MatrixRotateY)(Matrix *matrix, int32 angle);
    void (*MatrixRotateZ)(Matrix *matrix, int32 angle);
    void (*MatrixRotateXYZ)(Matrix *matrix, int32 angleX, int32 angleY, int32 angleZ);
    void (*MatrixInverse)(Matrix *dest, Matrix *matrix);
    void (*MatrixCopy)(Matrix *matDest, Matrix *matSrc);

    // Strings
    void (*InitString)(String *string, const char *text, uint32 textLength);
    void (*CopyString)(String *dst, String *src);
    void (*SetString)(String *string, const char *text);
    void (*AppendString)(String *string, String *appendString);
    void (*AppendText)(String *string, const char *appendText);
    void (*LoadStringList)(String *stringList, const char *filePath, uint32 charSize);
    bool32 (*SplitStringList)(String *splitStrings, String *stringList, int32 startStringID, int32 stringCount);
    void (*GetCString)(char *destChars, String *string);
    bool32 (*CompareStrings)(String *string1, String *string2, bool32 exactMatch);

    // Screens & Displays
    void (*GetDisplayInfo)(int32 *displayID, int32 *width, int32 *height, int32 *refreshRate, char *text);
    void (*GetWindowSize)(int32 *width, int32 *height);
    int32 (*SetScreenSize)(uint8 screenID, uint16 width, uint16 height);
    void (*SetClipBounds)(uint8 screenID, int32 x1, int32 y1, int32 x2, int32 y2);
#if RETRO_REV02
    void (*SetScreenRenderVertices)(uint8 startVert2P_S1, uint8 startVert2P_S2, uint8 startVert3P_S1, uint8 startVert3P_S2, uint8 startVert3P_S3);
#endif

    // Spritesheets
    int16 (*LoadSpriteSheet)(const char *path, Scopes scope);

    // Palettes & Colors
#if RETRO_REV02
    void (*SetTintLookupTable)(uint16 *lookupTable);
#else
    uint16 *(*GetTintLookupTable)(void);
#endif
    void (*SetPaletteMask)(uint32 maskColor);
    void (*SetPaletteEntry)(uint8 bankID, uint8 index, uint32 color);
    uint32 (*GetPaletteEntry)(uint8 bankID, uint8 index);
    void (*SetActivePalette)(uint8 newActiveBank, int32 startLine, int32 endLine);
    void (*CopyPalette)(uint8 sourceBank, uint8 srcBankStart, uint8 destinationBank, uint8 destBankStart, uint16 count);
#if RETRO_REV02
    void (*LoadPalette)(uint8 bankID, const char *path, uint16 rowFlags);
#endif
    void (*RotatePalette)(uint8 bankID, uint8 startIndex, uint8 endIndex, bool32 right);
    void (*SetLimitedFade)(uint8 destBankID, uint8 srcBankA, uint8 srcBankB, int16 blendAmount, int32 startIndex, int32 endIndex);
#if RETRO_REV02
    void (*BlendColors)(uint8 destBankID, uint32 *srcColorsA, uint32 *srcColorsB, int32 blendAmount, int32 startIndex, int32 count);
#endif

    // Drawing
    void (*DrawRect)(int32 x, int32 y, int32 width, int32 height, uint32 color, int32 alpha, InkEffects inkEffect, bool32 screenRelative);
    void (*DrawLine)(int32 x1, int32 y1, int32 x2, int32 y2, uint32 color, int32 alpha, InkEffects inkEffect, bool32 screenRelative);
    void (*DrawCircle)(int32 x, int32 y, int32 radius, uint32 color, int32 alpha, InkEffects inkEffect, bool32 screenRelative);
    void (*DrawCircleOutline)(int32 x, int32 y, int32 innerRadius, int32 outerRadius, uint32 color, int32 alpha, InkEffects inkEffect,
                              bool32 screenRelative);
    void (*DrawFace)(Vector2 *verticies, int32 vertCount, int32 r, int32 g, int32 b, int32 alpha, InkEffects inkEffect);
    void (*DrawBlendedFace)(Vector2 *verticies, color *vertColors, int32 vertCount, int32 alpha, InkEffects inkEffect);
    void (*DrawSprite)(Animator *animator, Vector2 *position, bool32 screenRelative);
    void (*DrawDeformedSprite)(uint16 sheet, InkEffects inkEffect, bool32 screenRelative);
    void (*DrawText)(Animator *animator, Vector2 *position, String *info, int32 startFrame, int32 endFrame, int32 align, int32 spacing,
                     void *unused, Vector2 *charOffsets, bool32 screenRelative);
    void (*DrawTile)(uint16 *tileInfo, int32 countX, int32 countY, Vector2 *position, Vector2 *offset, bool32 screenRelative);
    void (*CopyTile)(uint16 dest, uint16 src, uint16 count);
    void (*DrawAniTiles)(uint16 sheetID, uint16 tileIndex, uint16 srcX, uint16 srcY, uint16 width, uint16 height);
    void (*FillScreen)(uint32 color, int32 alphaR, int32 alphaG, int32 alphaB);

    // Meshes & 3D Scenes
    uint16 (*LoadMesh)(const char *filename, uint8 scope);
    uint16 (*Create3DScene)(const char *identifier, uint16 faceCount, uint8 scope);
    void (*Prepare3DScene)(uint16 index);
    void (*SetDiffuseColor)(uint16 index, int32 x, int32 y, int32 z);
    void (*SetDiffuseIntensity)(uint16 index, int32 x, int32 y, int32 z);
    void (*SetSpecularIntensity)(uint16 index, int32 x, int32 y, int32 z);
    void (*AddModelTo3DScene)(uint16 modelFrames, uint16 sceneIndex, uint8 drawMode, Matrix *matWorld, Matrix *matNormal, color color);
    void (*SetModelAnimation)(uint16 modelFrames, Animator *animator, int16 speed, uint8 loopIndex, bool32 forceApply, uint16 frameID);
    void (*AddMeshFrameTo3DScene)(uint16 modelFrames, uint16 sceneIndex, Animator *animator, uint8 drawMode, Matrix *matWorld, Matrix *matNormal,
                                  color color);
    void (*Draw3DScene)(uint16 index);

    // Sprite Animations & Frames
    uint16 (*LoadSpriteAnimation)(const char *path, Scopes scope);
    uint16 (*CreateSpriteAnimation)(const char *filename, uint32 frameCount, uint32 listCount, Scopes scope);
    void (*SetSpriteAnimation)(uint16 aniFrames, uint16 listID, Animator *animator, bool32 forceApply, int16 frameID);
    void (*EditSpriteAnimation)(uint16 aniFrames, uint16 listID, const char *name, int32 frameOffset, uint16 frameCount, int16 speed, uint8 loopIndex,
                                uint8 rotationFlag);
    void (*SetSpriteString)(uint16 aniFrames, uint16 listID, String *info);
    uint16 (*FindSpriteAnimation)(uint16 aniFrames, const char *name);
    SpriteFrame *(*GetFrame)(uint16 aniFrames, uint16 listID, int32 frameID);
    Hitbox *(*GetHitbox)(Animator *animator, uint8 hitboxID);
    int16 (*GetFrameID)(Animator *animator);
    int32 (*GetStringWidth)(uint16 aniFrames, uint16 listID, String *info, int32 startIndex, int32 length, int32 spacing);
    void (*ProcessAnimation)(Animator *animator);

    // Tile Layers
    int32 (*GetTileLayerID)(const char *name);
    TileLayer *(*GetTileLayer)(int32 layerID);
    void (*GetLayerSize)(uint16 layer, Vector2 *size, bool32 usePixelUnits);
    uint16 (*GetTileInfo)(uint16 layer, int32 x, int32 y);
    void (*SetTileInfo)(uint16 layer, int32 x, int32 y, uint16 tile);
    int32 (*CopyTileLayer)(uint16 dstLayerID, int32 dstStartX, int32 dstStartY, uint16 srcLayerID, int32 srcStartX, int32 srcStartY, int32 countX,
                           int32 countY);
    void (*ProcessParallax)(TileLayer *TileLayer);
    ScanlineInfo *(*GetScanlines)(void);

    // Object & Tile Collisions
    bool32 (*CheckObjectCollisionTouchBox)(void *thisEntity, Hitbox *thisHitbox, void *otherEntity, Hitbox *otherHitbox);
    bool32 (*CheckObjectCollisionTouchCircle)(void *thisEntity, int32 thisOffset, void *otherEntity, int32 otherOffset);
    uint8 (*CheckObjectCollisionBox)(void *thisEntity, Hitbox *thisHitbox, void *otherEntity, Hitbox *otherHitbox, bool32 setPos);
    bool32 (*CheckObjectCollisionPlatform)(void *thisEntity, Hitbox *thisHitbox, void *otherEntity, Hitbox *otherHitbox, bool32 setPos);
    bool32 (*ObjectTileCollision)(void *entity, uint16 collisionLayers, uint8 collisionMode, uint8 collisionPlane, int32 xOffset, int32 yOffset,
                                  bool32 setPos);
    bool32 (*ObjectTileGrip)(void *entity, uint16 collisionLayers, uint8 collisionMode, uint8 collisionPlane, int32 xOffset, int32 yOffset,
                             int32 tolerance);
    void (*ProcessObjectMovement)(void *entity, Hitbox *outer, Hitbox *inner);
    int32 (*GetTileAngle)(uint16 tileID, uint8 cPlane, uint8 cMode);
    void (*SetTileAngle)(uint16 tileID, uint8 cPlane, uint8 cMode, uint8 angle);
    uint8 (*GetTileFlags)(uint16 tileID, uint8 cPlane);
    void (*SetTileFlags)(uint16 tileID, uint8 cPlane, uint8 flags);

    // Audio
    int32 (*GetSfx)(const char *path);
    int32 (*PlaySfx)(uint16 sfx, int32 loopPoint, int32 priority);
    void (*StopSfx)(uint16 sfx);
    int32 (*PlayStream)(const char *filename, uint32 channel, uint32 startPos, uint32 loopPoint, bool32 loadASync);
    void (*SetChannelAttributes)(uint8 channel, float volume, float pan, float speed);
    void (*StopChannel)(uint32 channel);
    void (*PauseChannel)(uint32 channel);
    void (*ResumeChannel)(uint32 channel);
    bool32 (*IsSfxPlaying)(uint16 sfxID);
    bool32 (*ChannelActive)(uint32 channel);
    uint32 (*GetChannelPos)(uint32 channel);

    // Videos & "HD Images"
    void (*LoadVideo)(const char *filename, double startDelay, bool32 (*skipCallback)(void));
    bool32 (*LoadImage)(const char *filename, double displayLength, double speed, bool32 (*skipCallback)(void));

    // Input
#if RETRO_REV02
    int32 (*ControllerIDForInputID)(uint8 controllerID);
    int32 (*MostRecentActiveControllerID)(bool32 confirmOnly, bool32 unassignedOnly, uint32 maxInactiveTimer);
    int32 (*GetControllerType)(uint32 inputID);
    int32 (*GetAssignedControllerID)(uint32 inputID);
    int32 (*GetInputUnknown)(uint32 inputID);
    int32 (*InputUnknown1)(uint32 inputID, int32 unknown1, int32 unknown2);
    int32 (*InputUnknown2)(uint32 inputID, int32 unknown1, int32 unknown2);
    int32 (*GetControllerUnknown)(void);
    int32 (*ControllerUnknown1)(uint8 controllerID, int32 unknown1, int32 unknown2);
    int32 (*ControllerUnknown2)(uint8 controllerID, int32 unknown1, int32 unknown2);
    void (*AssignControllerID)(uint8 controllerID, uint32 inputID);
    bool32 (*ControllerIsAssigned)(uint8 controllerID);
    void (*ResetControllerAssignments)(void);
#endif
#if !RETRO_REV02
    void (*GetUnknownInputValue)(int32 controllerID, int32 type, int32 *value);
#endif

    // User File Management
    int32 (*LoadUserFile)(const char *filename, void *buffer, uint32 size); // load user file from exe dir
    int32 (*SaveUserFile)(const char *fileName, void *buffer, uint32 size); // save use file to exe dir

    // Printing (Rev02)
#if RETRO_REV02
    void (*PrintLog)(PrintModes printType, const char *message, ...);
    void (*PrintText)(PrintModes printType, const char *message);
    void (*PrintString)(PrintModes printType, String *message);
    void (*PrintUInt32)(PrintModes printType, const char *message, uint32 i);
    void (*PrintInt32)(PrintModes printType, const char *message, int32 i);
    void (*PrintFloat)(PrintModes printType, const char *message, float f);
    void (*PrintVector2)(PrintModes printType, const char *message, int32 x, int32 y);
    void (*PrintHitbox)(PrintModes printType, const char *message, Hitbox *hitbox);
#endif

    // Editor
    void (*SetActiveVariable)(int32 classID, const char *name);
    void (*AddVarEnumValue)(const char *name);

    // Debugging
#if RETRO_REV02
    void (*ClearViewableVariables)(void);
    void (*AddViewableVariable)(const char *name, void *value, ViewableVarTypes type, int32 min, int32 max);
#endif

    // Printing (Rev01)
#if !RETRO_REV02
    void (*PrintMessage)(void *message, uint8 type);
#endif
} RSDKFunctionTable;

extern RSDKFunctionTable RSDK;
#if RETRO_REV02
extern APIFunctionTable API;
#endif
#if RETRO_USE_MOD_LOADER
extern ModFunctionTable Mod;
#endif

#include "All.h"

#define RSDK_EDITABLE_VAR(object, type, var) RSDK.SetEditableVar(type, #var, (uint8)object->classID, offsetof(Entity##object, var))
#define RSDK_EDITABLE_ARRAY(object, type, var, count, arrType)                                                                                       \
    for (int i = 0; i < (count); ++i) {                                                                                                              \
        char buffer[0x40];                                                                                                                           \
        sprintf_s(buffer, (int32)sizeof(buffer), "%s%d", #var, i);                                                                                   \
        RSDK.SetEditableVar(type, buffer, (uint8)object->classID, offsetof(Entity##object, var) + sizeof(arrType) * i);                              \
    }

#if RETRO_INCLUDE_EDITOR
// Some extra precaution to prevent crashes in editor
#define RSDK_ACTIVE_VAR(object, var)                                                                                                                 \
    if (object) {                                                                                                                                    \
        RSDK.SetActiveVariable(object->classID, #var);                                                                                               \
    }                                                                                                                                                \
    else {                                                                                                                                           \
        RSDK.SetActiveVariable(-1, #var);                                                                                                            \
    }
#define RSDK_ENUM_VAR(name, var) RSDK.AddVarEnumValue(name)

#define RSDK_DRAWING_OVERLAY(isDrawingOverlay) SceneInfo->debugMode = isDrawingOverlay

#define RSDK_REGISTER_OBJECT(object)                                                                                                                 \
    RSDK.RegisterObject((Object **)&object, #object, sizeof(Entity##object), sizeof(Object##object), object##_Update, object##_LateUpdate,           \
                        object##_StaticUpdate, object##_Draw, object##_Create, object##_StageLoad, object##_EditorDraw, object##_EditorLoad,         \
                        object##_Serialize)
#else
#define RSDK_REGISTER_OBJECT(object)                                                                                                                 \
    RSDK.RegisterObject((Object **)&object, #object, sizeof(Entity##object), sizeof(Object##object), object##_Update, object##_LateUpdate,           \
                        object##_StaticUpdate, object##_Draw, object##_Create, object##_StageLoad, NULL, NULL, object##_Serialize)
#endif

#if RETRO_USE_MOD_LOADER
#define MOD_REGISTER_OBJECT(object, inherit)                                                                                                         \
    Mod.RegisterObject((Object **)&object, #object, sizeof(Entity##object), sizeof(Object##object), object##_Update, object##_LateUpdate,            \
                       object##_StaticUpdate, object##_Draw, object##_Create, object##_StageLoad, object##_EditorDraw, object##_EditorLoad,          \
                       object##_Serialize, inherit)
#endif

#if RETRO_REV02
#define RSDK_REGISTER_STATIC_VARIABLES(variables) RSDK.RegisterStaticVariables((void **)&variables, #variables, sizeof(Object##variables))
#endif

#define RSDK_THIS(class)                  Entity##class *self = (Entity##class *)SceneInfo->entity
#define RSDK_THIS_GEN()                   Entity *self = SceneInfo->entity
#define RSDK_GET_ENTITY(slot, class)      ((Entity##class *)RSDK.GetEntity(slot))
#define RSDK_GET_ENTITY_GEN(slot)         ((Entity *)RSDK.GetEntity(slot))
#define CREATE_ENTITY(object, data, x, y) ((Entity##object *)RSDK.CreateEntity(object->classID, data, x, y))

#define INIT_STRING(string)                                                                                                                          \
    string.chars  = NULL;                                                                                                                            \
    string.length = 0;                                                                                                                               \
    string.size   = 0

// Initializes entity values to the defaults
#define INIT_ENTITY(entity)                                                                                                                          \
    (entity)->active        = ACTIVE_BOUNDS;                                                                                                         \
    (entity)->visible       = false;                                                                                                                 \
    (entity)->updateRange.x = 0x800000;                                                                                                              \
    (entity)->updateRange.y = 0x800000;

#define foreach_active(type, entityOut)                                                                                                              \
    Entity##type *entityOut = NULL;                                                                                                                  \
    while (RSDK.GetActiveEntities(type->classID, (Entity **)&entityOut))
#define foreach_all(type, entityOut)                                                                                                                 \
    Entity##type *entityOut = NULL;                                                                                                                  \
    while (RSDK.GetEntities(type->classID, (Entity **)&entityOut))

#define foreach_active_group(group, entityOut)                                                                                                       \
    Entity *entityOut = NULL;                                                                                                                        \
    while (RSDK.GetActiveEntities(group, (Entity **)&entityOut))
#define foreach_all_group(group, entityOut)                                                                                                          \
    Entity *entityOut = NULL;                                                                                                                        \
    while (RSDK.GetEntities(group, (Entity **)&entityOut))

#if RETRO_USE_MOD_LOADER
#define foreach_config(text)                                                                                                                         \
    String *text = NULL;                                                                                                                           \
    while (Mod.ForeachConfig(&text))
#define foreach_configCategory(text)                                                                                                                 \
    String *text = NULL;                                                                                                                           \
    while (Mod.ForeachConfigCategory(&text))
#endif

#define foreach_break                                                                                                                                \
    RSDK.BreakForeachLoop();                                                                                                                         \
    break
#define foreach_return                                                                                                                               \
    RSDK.BreakForeachLoop();                                                                                                                         \
    return

#define destroyEntity(entity)   RSDK.ResetEntityPtr(entity, TYPE_BLANK, NULL)
#define destroyEntitySlot(slot) RSDK.ResetEntitySlot(slot, TYPE_BLANK, NULL)

#if RETRO_INCLUDE_EDITOR
#define showGizmos() (SceneInfo->listPos == SceneInfo->entitySlot || SceneInfo->effectGizmo)
#endif

#if RETRO_REV02
DLLExport void LinkGameLogicDLL(EngineInfo *info);
#else
DLLExport void LinkGameLogicDLL(EngineInfo info);
#endif

#if RETRO_USE_MOD_LOADER
DLLExport bool32 LinkModLogic(EngineInfo *info, const char *id);
#endif

#endif //! GAMEOBJECTS_H