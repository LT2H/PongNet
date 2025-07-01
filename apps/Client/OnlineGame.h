#pragma once

#include <GameCommon/Game.h>
#include "Client.h"
#include "GameCommon/Player.h"

class OnlineGame : public gc::Game
{
  public:
    OnlineGame(u32 width, u32 height) : gc::Game(width, height) {}

    bool on_user_create() { return true; }

    bool update(float dt) override
    {
        // update objects
        ball_->move(dt, width_, height_);

        // Check for collisions
        do_collisions();

        // update particles_
        particles_->update(dt, *ball_, 2, glm::vec2{ ball_->radius_ / 2.0f });

        update_powerups(dt);

        // reduce shake time
        if (shake_time_ > 0.0f)
        {
            shake_time_ -= dt;
            if (shake_time_ <= 0.0f)
            {
                effects_->shake_ = false;
            }
        }

        // reduce player 1 lives
        if (ball_->pos_.y >= height_ - ball_->size_.y)
        {
            --player1_->lives_;
        }

        // reduce player 2 lives
        if (ball_->pos_.y <= 0)
        {
            --player2_->lives_;
        }

        // check win condition of player 1
        if (state_ == gc::GameState::GAME_ACTIVE && player2_->lives_ == 0)
        {
            reset_players();
            reset_level();
            effects_->chaos_ = true;
            state_           = gc::GameState::GAME_WIN;
            winner_          = gc::Winner::Player1;
        }

        // TODO: check win condition of player 2
        if (state_ == gc::GameState::GAME_ACTIVE && player1_->lives_ == 0)
        {
            reset_players();
            reset_level();
            effects_->chaos_ = true;
            state_           = gc::GameState::GAME_WIN;
            winner_          = gc::Winner::Player2;
        }
        return true;
    }

  private:
    std::unordered_map<u32, gc::Player> map_objects{};
    u32 player_id{ 0 };
    Client client_{};
};