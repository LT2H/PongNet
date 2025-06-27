#include "GameCommon/GameObject.h"
#include <GameCommon/Player.h>

gc::Player::Player(u32 lives, const glm::vec2& pos, const glm::vec2& size,
                   const Texture2D& sprite)
    : GameObject{ pos, size, sprite }, lives_{ lives }
{
}