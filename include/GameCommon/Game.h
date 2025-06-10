#pragma once

#include "GameLevel.h"
#include "Common.h"
#include "GameObject.h"
#include "BallObject.h"
#include "glm/fwd.hpp"
#include <tuple>

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

    void do_collision();

    GameState state;

    std::array<bool, 1024> keys;

    std::vector<GameLevel> levels{};
    u32 current_level{ 0 };

  private:
    bool check_collision(const GameObject& one, const GameObject& two);

    Collision check_collision(const BallObject& one, const GameObject& two);

    Direction vector_direction(const glm::vec2& target);

    u32 width_;
    u32 height_;
};
} // namespace gc