#pragma once

#include <GameCommon/Common.h>

struct BallDesc
{
    float radius;
    bool stuck;
    glm::vec2 pos;
    glm::vec2 velocity;
    glm::vec2 size;
};