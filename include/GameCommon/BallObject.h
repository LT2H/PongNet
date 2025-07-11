#pragma once

#include "Common.h"
#include "GameObject.h"
#include "Texture.h"
#include "BallDesc.h"

namespace gc
{
class BallObject : public GameObject
{
  public: // ball state
    float radius_;
    bool stuck_;
    bool sticky_;
    bool passthrough_;

    explicit BallObject();
    BallObject(const glm::vec2& pos, float radius, const glm::vec2& velocity,
               const Texture2D& sprite);

    BallDesc get_desc() const
    {
        return BallDesc{ radius_, stuck_, pos_, velocity_ };
    }

    void set_props(const BallDesc ball_desc)
    {
        radius_   = ball_desc.radius;
        stuck_    = ball_desc.stuck;
        pos_      = ball_desc.pos;
        velocity_ = ball_desc.velocity;
        size_     = ball_desc.size;
    }

    glm::vec2 move(float dt, u32 window_width, u32 window_height);
    void reset(const glm::vec2& position, const glm::vec2& velocity);
};
} // namespace gc
