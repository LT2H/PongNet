#include <GameCommon/BallObject.h>
#include <GameCommon/Common.h>
#include "GameCommon/GameObject.h"

gcom::BallObject::BallObject()
    : GameObject{}, radius_{ 12.5f }, stuck_{ true }, sticky_{ false },
      passthrough_{ false }
{
}

gcom::BallObject::BallObject(const glm::vec2& pos, float radius,
                           const glm::vec2& velocity, const Texture2D& sprite)
    : GameObject{ pos,
                  glm::vec2{ radius * 2.0, radius * 2.0 },
                  sprite,
                  glm::vec3{ 1.0f },
                  velocity },
      radius_{ radius }, stuck_{ true }, sticky_{ false }, passthrough_{ false }
{
}

glm::vec2 gcom::BallObject::move(float dt, u32 window_width, u32 window_height)
{
    // if not stuck to player board
    if (!stuck_)
    {
        // move the ball
        pos_ += velocity_ * dt;

        // check if outside window bounds; if so, reverse velocity and restore at
        // correct pos
        if (pos_.x <= 0.0f)
        {
            velocity_.x = -velocity_.x;
            pos_.x      = 0.0f;
        }
        else if (pos_.x + size_.x >= window_width)
        {
            velocity_.x = -velocity_.x;
            pos_.x      = window_width - size_.x;
        }
        if (pos_.y <= 0.0f)
        {
            velocity_.y = -velocity_.y;
            pos_.y      = 0.0f;
        }
        else if (pos_.y + size_.y >= window_height)
        {
            velocity_.y = -velocity_.y;
            pos_.y      = window_height - size_.y;
        }
    }
    return pos_;
}

// resets the ball to initial stuck position (if ball is outside window bounds)
void gcom::BallObject::reset(const glm::vec2& position, const glm::vec2& velocity)
{
    pos_         = position;
    velocity_    = velocity;
    stuck_       = true;
    sticky_      = false;
    passthrough_ = false;
}