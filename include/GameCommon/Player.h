#pragma once

#include "GameObject.h"
#include "Texture.h"
#include "SpriteRenderer.h"
#include "PlayerDesc.h"

#include "Common.h"

namespace gc
{
class Player : public GameObject
{
  public:
    explicit Player(){}

    Player(u32 lives, const glm::vec2& pos, const glm::vec2& size,
           const Texture2D& sprite);

    u32 unique_id_{ 0 };
    u32 lives_;

    PlayerDesc get_desc() { return { unique_id_, lives_, pos_, size_ }; }

    void set_props(const PlayerDesc& player_desc)
    {
        unique_id_ = player_desc.unique_id;
        lives_     = player_desc.lives;
        pos_       = player_desc.pos;
        size_      = player_desc.size;
    }
};
} // namespace gc