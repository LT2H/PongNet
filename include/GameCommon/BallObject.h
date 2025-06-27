#pragma once 

#include "Common.h"
#include "GameObject.h"
#include "Texture.h"

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

    glm::vec2 move(float dt, u32 window_width, u32 window_height);
    void reset(const glm::vec2& position, const glm::vec2& velocity);
};
} // namespace gc
