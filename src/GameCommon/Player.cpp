#include "GameCommon/GameObject.h"
#include "GameCommon/ScreenInfo.h"
#include <GameCommon/Player.h>

gcom::Player::Player(u32 lives, const glm::vec2& pos, const glm::vec2& size,
                   const Texture2D& sprite, ScreenInfo screen_info)
    : GameObject{ pos, size, sprite }, lives_{ lives }, screen_info_(screen_info)
{
}