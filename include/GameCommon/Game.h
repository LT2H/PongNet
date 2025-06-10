#pragma once

#include "GameLevel.h"
#include "Common.h"
#include "GameObject.h"

namespace gc
{
enum class GameState
{
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
};
class Game
{
  public:
    Game(u32 width, u32 height);

    ~Game();
    void init();

    void process_input(float dt);

    void update(float dt);

    void render();

    void do_collision();

    GameState state;

    std::array<bool, 1024> keys;

    std::vector<GameLevel> levels{};
    u32 current_level{ 0 };

  private:
    bool check_collision(const GameObject& one, const GameObject& two);

    u32 width_;
    u32 height_;
};
} // namespace gc