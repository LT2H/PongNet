#pragma once

#include "GameObject.h"
#include "Texture.h"
#include "SpriteRenderer.h"

#include "Common.h"

namespace gc
{
class Player : public GameObject
{
  public:
    Player(u32 lives, const glm::vec2& pos, const glm::vec2& size,
           const Texture2D& sprite);

    u32 lives_;
    u32 unique_id_{ 0 };
};
} // namespace gc