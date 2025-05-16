#pragma once

#include "Common.h"

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
    Game(unsigned int width, unsigned int height);

    ~Game();
    void init();

    void process_input(float dt);

    void update(float dt);

    void render();

    GameState state;

    std::array<bool, 1024> keys;

  private:
    unsigned int width_;
    unsigned int height_;
};
} // namespace gc