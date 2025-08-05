#include <GameCommon/GameObject.h>

gcom::GameObject::GameObject()
    : pos_{ 0.0f, 0.0f }, size_{ 1.0f, 1.0f }, velocity_{ 0.0f }, color_{ 1.0f },
      rotation_{ 0.0f }, sprite_{}, is_solid_{ false }, destroyed_{ false }
{
}

gcom::GameObject::GameObject(const glm::vec2& pos, const glm::vec2& size,
                           const Texture2D& sprite, const glm::vec3 color,
                           const glm::vec2& velocity)
    : pos_{ pos }, size_{ size }, sprite_(sprite), color_{ color },
      velocity_{ velocity }, destroyed_{ false }, rotation_{0.0f}, is_solid_{ false }
{
}

void gcom::GameObject::draw(SpriteRenderer& renderer)
{
    renderer.draw_sprite(sprite_, pos_, size_, rotation_, color_);
}

