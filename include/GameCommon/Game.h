#pragma once

#include "GameLevel.h"
#include "Common.h"
#include "GameObject.h"
#include "BallObject.h"
#include "PowerUp.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "TextRenderer.h"

namespace gc
{
enum class GameState
{
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
};

enum class Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

using Collision = std::tuple<bool, Direction, glm::vec2>;

class Game
{
  public:
    Game(u32 width, u32 height);

    ~Game();
    void init();

    void process_input(float dt);

    void update(float dt);

    void reset_level();

    void reset_player();

    void render();

    void do_collisions();

    void spawn_powerups(GameObject& block);
    void active_powerup(const PowerUp& powerup);
    void update_powerups(float dt);

    GameState state_;

    std::array<bool, 1024> keys_;
    std::array<bool, 1024> keys_processed_;

    std::vector<GameLevel> levels_{};
    u32 current_level_{ 0 };

    std::vector<PowerUp> powerups_{};

    u32 lives_{ 3 };

  private:
    bool check_collision(const GameObject& one, const GameObject& two);

    Collision check_collision(const BallObject& one, const GameObject& two);

    Direction vector_direction(const glm::vec2& target);

    u32 width_;
    u32 height_;

    std::unique_ptr<BallObject> ball_;

    std::unique_ptr<GameObject> player_;

    std::unique_ptr<SpriteRenderer> renderer_;

    std::unique_ptr<ParticleGenerator> particles_;

    std::unique_ptr<PostProcessor> effects_;

    std::unique_ptr<TextRender> text_;

    // Audio
    ma_result result_{};
    ma_engine engine_{};
};
} // namespace gc