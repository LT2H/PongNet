#pragma once

#include <GameCommon/Common.h>

struct PlayerDesc
{
    u32 unique_id;
    u32 lives{ 0 };
    glm::vec2 pos{ 0.0f, 0.0f };
    glm::vec2 size{ 100.0f, 20.0f };
};