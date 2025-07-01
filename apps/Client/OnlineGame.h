#pragma once

#include <GameCommon/Game.h>
#include "Client.h"

class OnlineGame : public gc::Game
{
  public:
    OnlineGame(u32 width, u32 height) : gc::Game(width, height) {}

    bool on_user_create() { return true; }

    bool on_user_update(float elapsed_time) { return true; }

  private:
    Client client_{};
};