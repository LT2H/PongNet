#pragma once

#include "Texture.h"
#include "SpriteRenderer.h"

#include "Common.h"

// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.

namespace gc
{
class GameObject
{
  public:
    explicit GameObject();
    GameObject(const glm::vec2& pos, const glm::vec2& size, const Texture2D& sprite,
               const glm::vec3 color     = glm::vec3{ 1.0f },
               const glm::vec2& velocity = glm::vec2{ 0.0f, 0.0f });

    // draw sprite
    virtual void draw(SpriteRenderer& renderer);

    constexpr bool is_solid() const { return is_solid_; }

    constexpr glm::vec2 pos() const { return pos_; }

    constexpr glm::vec2 size() const { return size_; }

    void decrease_pos_x(const float& velocity) { pos_.x -= velocity; }
    
    void increase_pos_x(const float& velocity) { pos_.x += velocity; }

    void set_is_solid(bool value) { is_solid_ = true; }

    constexpr bool destroyed() const { return destroyed_; }

  private:
    // Obj state
    glm::vec2 pos_;
    glm::vec2 size_;
    glm::vec2 velocity_;
    glm::vec3 color_;
    float rotation_;
    bool is_solid_;
    bool destroyed_;

    // Render state
    Texture2D sprite_;
};

} // namespace gc