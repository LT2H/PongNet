#pragma once 

#include "Common.h"
#include "GameCommon/GameObject.h"
#include "GameCommon/Texture.h"

namespace gcom
{
constexpr glm::vec2 SIZE{ 60.0f, 20.0f };
constexpr glm::vec2 VELOCITY{ 0.0F, 150.0f };

class PowerUp : public GameObject
{
  public:
    // powerup state
    std::string type_;
    float duration_;
    bool activated_;

    PowerUp(std::string_view type, const glm::vec3& color, float duration,
            const glm::vec2& position, const Texture2D texture)
        : GameObject{ position, SIZE, texture, color, VELOCITY }, type_{ type },
          duration_{ duration }, activated_{}
    {
    }
};
} // namespace gcom