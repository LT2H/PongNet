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
};