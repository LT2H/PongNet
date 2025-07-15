#pragma once

#include <GameCommon/Common.h>

enum class GameMsgTypes : u32 {
    ServerGetStatus,
    ServerGetPing,
    ServerIsFull,

    ClientAccepted,
    ClientAssignId,
    ClientRegisterWithServer,
    ClientUnregisterWithServer,

    GameAddPlayer,
    GameRemovePlayer,
    GameUpdatePlayer,
    GamePlayerLaunchBall,
    GameReduceLives,
    GameEnds,

    // Audio
    GamePlayPadSound,

    GameAddBall,
    GameUpdateBall,
};