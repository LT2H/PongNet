#include <GameCommon/Common.h>
#include <NetCommon/NetCommon.h>
#include "../GameMsgTypes.h"
#include <GameCommon/PlayerDesc.h>
#include "GameCommon/ResourceManager.h"
#include "GameCommon/Game.h"
#include "GameCommon/ScreenInfo.h"
#include "NetCommon/NetConnection.h"
#include "NetCommon/NetMessage.h"
#include "NetCommon/NetServer.h"
#include <GameCommon/BallDesc.h>

class Server : public net::ServerInterface<GameMsgTypes>
{
  public:
    Server(u16 port) : net::ServerInterface<GameMsgTypes>{ port }
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW\n";
        }
    }

    ~Server() { glfwTerminate(); }

  protected:
    bool
    on_client_connect(std::shared_ptr<net::Connection<GameMsgTypes>> client) override
    {
        // We will allow all clients for now
        return true;
    }

    void on_client_validated(
        std::shared_ptr<net::Connection<GameMsgTypes>> client) override
    {
        // Client passed validation check, so send them a message informing them they
        // can continue to communicate
        net::Message<GameMsgTypes> msg{};
        msg.header.id = GameMsgTypes::ClientAccepted;
        client->send(msg);
    }

    void on_client_disconnect(
        std::shared_ptr<net::Connection<GameMsgTypes>> client) override
    {
        std::cout << "A player disconnected\n";
        if (client)
        {
            if (!map_player_roster_.contains(client->id()))
            {
            }
            else
            {
                auto& player_disconnect{ map_player_roster_[client->id()] };
                std::cout << "[Disconnected Unexpectedly]:" +
                                 std::to_string(player_disconnect.unique_id) + "\n";
                map_player_roster_.erase(client->id());
                garbage_ids_.push_back(client->id());
            }
        }
    }

    void on_message(std::shared_ptr<net::Connection<GameMsgTypes>> client,
                    net::Message<GameMsgTypes>& msg) override
    {
        if (!garbage_ids_.empty())
        {
            for (auto player_id : garbage_ids_)
            {
                net::Message<GameMsgTypes> m{};
                m.header.id = GameMsgTypes::GameRemovePlayer;
                m << player_id;
                std::cout << "Removing " << player_id << "\n";
                message_all_clients(m);
            }
            garbage_ids_.clear();
        }

        switch (msg.header.id)
        {
        case GameMsgTypes::ClientRegisterWithServer:
        {
            PlayerDesc player_desc{ 0, 3, { 0.0f, 0.0f } };
            msg >> player_desc;

            temp_player_id_ = player_desc.unique_id;

            if (map_player_roster_.size() < 2)
            {
                player_desc.unique_id = client->id();
                // map_player_roster_.insert_or_assign(player_desc.unique_id,
                //                                     player_desc);

                net::Message<GameMsgTypes> msg_send_id{};
                msg_send_id.header.id = GameMsgTypes::ClientAssignId;
                msg_send_id << player_desc.unique_id;
                message_client(client, msg_send_id);

                net::Message<GameMsgTypes> msg_add_player{};
                msg_add_player.header.id = GameMsgTypes::GameAddPlayer;

                glm::vec2 player_pos{
                    glm::vec2{ player_desc.screen_info.width / 2.0f -
                                   player_desc.size.x / 2.0f,
                               0 },
                }; // position for player 2

                if (!has_player_one_)
                {
                    player_pos = glm::vec2{
                        player_desc.screen_info.width / 2.0f -
                            player_desc.size.x / 2.0f,
                        player_desc.screen_info.height - player_desc.size.y
                    }; // Set the position for player 1

                    has_player_one_           = true;
                    player_desc.player_number = PlayerNumber::One;
                }
                else
                {
                    player_desc.player_number = PlayerNumber::Two;
                    // Also add the ball now that we have 2 players
                    const glm::vec2 player_size{ 100.0f, 20.0f };
                    const float player_velocity{ 500.0f };

                    glm::vec2 player1_pos{ glm::vec2{
                        player_desc.screen_info.width / 2.0f - player_size.x / 2.0f,
                        player_desc.screen_info.height - player_size.y } };

                    glm::vec2 ball_pos{
                        player1_pos + glm::vec2{ player_size.x / 2.0f - ball_radius_,
                                                 -ball_radius_ * 2.0f }
                    };

                    ball_ = { ball_radius_,
                              true,
                              ball_pos,
                              initial_ball_velocity_,
                              glm::vec2{ ball_radius_ * 2.0, ball_radius_ * 2.0 } };

                    net::Message<GameMsgTypes> msg_add_ball{};
                    msg_add_ball.header.id = GameMsgTypes::GameAddBall;
                    msg_add_ball << ball_;
                    message_all_clients(msg_add_ball);

                    game_playing_ = true; // Start the game
                }
                player_desc.pos = player_pos;

                msg_add_player << player_desc;
                message_all_clients(msg_add_player);

                // Also update player's desc on the server
                map_player_roster_.insert_or_assign(player_desc.unique_id,
                                                    player_desc);

                for (const auto& player : map_player_roster_)
                {
                    net::Message<GameMsgTypes> msg_add_other_players{};
                    msg_add_other_players.header.id = GameMsgTypes::GameAddPlayer;
                    msg_add_other_players << player.second;
                    message_client(client, msg_add_other_players);
                }
            }
            else
            {
                net::Message<GameMsgTypes> msg_server_is_full{};
                msg_server_is_full.header.id = GameMsgTypes::ServerIsFull;
                message_client(client, msg_server_is_full);
            }

            break;
        }
        case GameMsgTypes::ClientUnregisterWithServer:
        {
            break;
        }
        case GameMsgTypes::GameUpdatePlayer:
        {
            PlayerDesc player_desc{ 0, 3, { 0.0f, 0.0f } };
            msg >> player_desc;
            temp_player_id_ = player_desc.unique_id;

            if (ball_.stuck &&
                map_player_roster_[player_desc.unique_id].player_number ==
                    PlayerNumber::One)
            {
                glm::vec2 ball_pos{ player_desc.pos +
                                    glm::vec2{ player_desc.size.x / 2.0f -
                                                   ball_radius_,
                                               -ball_radius_ * 2.0f } };
                ball_.pos = ball_pos;
            }

            // Bounce update to everyone except incoming client
            net::Message<GameMsgTypes> msg_bounce{};
            msg_bounce.header.id = GameMsgTypes::GameUpdatePlayer;
            msg_bounce << player_desc;
            message_all_clients(msg_bounce, client);

            // Also update player's desc on the server
            map_player_roster_.insert_or_assign(player_desc.unique_id, player_desc);

            break;
        }
        case GameMsgTypes::GamePlayerLaunchBall:
        {
            ball_.stuck = false;
            break;
        }
        }
    }

    void do_collisions()
    {
        if (map_player_roster_.contains(temp_player_id_))
        {
            // check collisions for player 1 pad (unless stuck)
            gc::Collision result{ check_collision(
                ball_, map_player_roster_[temp_player_id_]) };
            if (map_player_roster_[temp_player_id_].player_number ==
                PlayerNumber::One)
            {
                if (!ball_.stuck && std::get<0>(result))
                {
                    // check where it hit the board, and change velocity based on
                    // where it hit the board
                    float center_board{ map_player_roster_[temp_player_id_].pos.x +
                                        map_player_roster_[temp_player_id_].size.x /
                                            2.0f };
                    float distance{ ball_.pos.x + ball_.radius - center_board };
                    float percentage{ distance /
                                      (map_player_roster_[temp_player_id_].size.x /
                                       2.0f) };

                    // then move accordingly
                    float strength{ 2.0f };
                    glm::vec2 old_velocity{ ball_.velocity };
                    ball_.velocity.x =
                        initial_ball_velocity_.x * percentage * strength;
                    // ball_.velocity.y = -ball_.velocity.y;
                    ball_.velocity.y =
                        -1.0f * abs(ball_.velocity.y); // avoid sticky paddle issue
                    ball_.velocity =
                        glm::normalize(ball_.velocity) * glm::length(old_velocity);

                    // ball_.stuck = ball_.sticky;

                    // ma_engine_play_sound(&engine_, "res/audio/bleep.wav",
                    // nullptr);
                    net::Message<GameMsgTypes> msg_play_pad_sound{};
                    msg_play_pad_sound.header.id = GameMsgTypes::GamePlayPadSound;
                    message_all_clients(msg_play_pad_sound);
                }
            }

            if (map_player_roster_[temp_player_id_].player_number ==
                PlayerNumber::Two)
            {
                if (!ball_.stuck && std::get<0>(result))
                {
                    // check where it hit the board, and change velocity based on
                    // where it hit the board
                    float center_board{ map_player_roster_[temp_player_id_].pos.x +
                                        map_player_roster_[temp_player_id_].size.x /
                                            2.0f };
                    float distance{ ball_.pos.x + ball_.radius - center_board };
                    float percentage{ distance /
                                      (map_player_roster_[temp_player_id_].size.x /
                                       2.0f) };

                    // then move accordingly
                    float strength{ 2.0f };
                    glm::vec2 old_velocity{ ball_.velocity };
                    ball_.velocity.x =
                        initial_ball_velocity_.x * percentage * strength;
                    // ball_.velocity.y = -ball_.velocity.y;
                    ball_.velocity.y =
                        -1.0f * abs(ball_.velocity.y); // avoid sticky paddle issue
                    ball_.velocity =
                        glm::normalize(-ball_.velocity) *
                        glm::length(
                            old_velocity); // Flip the velocity_ of the player 2
                                           // pad to shoot the ball downward

                                           // ball_->stuck_ = ball_->sticky_;

                    // ma_engine_play_sound(&engine_, "res/audio/bleep.wav",
                    // nullptr);
                    net::Message<GameMsgTypes> msg_play_pad_sound{};
                    msg_play_pad_sound.header.id = GameMsgTypes::GamePlayPadSound;
                    message_all_clients(msg_play_pad_sound);
                }
            }
        }
        // check collisions for player 2 pad
    }


    gc::Collision check_collision(const BallDesc& one, const PlayerDesc& two)
    {
        // get center point circle first
        glm::vec2 center{ one.pos + one.radius };
        // calculate AABB info (center, half-extents)
        glm::vec2 aabb_half_extents{ two.size.x / 2.0f, two.size.y / 2.0f };
        glm::vec2 aabb_center{ two.pos.x + aabb_half_extents.x,
                               two.pos.y + aabb_half_extents.y };
        // get difference vector between both centers
        glm::vec2 difference{ center - aabb_center };
        glm::vec2 clamped{ glm::clamp(
            difference, -aabb_half_extents, aabb_half_extents) };
        // add clamped value to AABB_center and we get the value of box closest to
        // circle
        glm::vec2 closest{ aabb_center + clamped };
        // retrieve vector between center circle and closest point AABB and check if
        // length <= radius
        difference = closest - center;
        if (glm::length(difference) <= one.radius)
            return std::make_tuple(true, vector_direction(difference), difference);
        else
            return std::make_tuple(false, gc::Direction::UP, glm::vec2(0.0f, 0.0f));
    }

    gc::Direction vector_direction(const glm::vec2& target)
    {
        std::array<glm::vec2, 4> compass{
            glm::vec2(0.0f, 1.0f),  // up
            glm::vec2(1.0f, 0.0f),  // right
            glm::vec2(0.0f, -1.0f), // down
            glm::vec2(-1.0f, 0.0f)  // left
        };

        float max{ 0.0f };
        u32 best_match{ 4294967295 };
        for (u32 i{ 0 }; i < 4; ++i)
        {
            float dot_product{ glm::dot(glm::normalize(target), compass[i]) };
            if (dot_product > max)
            {
                max        = dot_product;
                best_match = i;
            }
        }
        return static_cast<gc::Direction>(best_match);
    }

  public:
    void update(size_t max_messages, bool wait) override
    {
        if (wait)
            messages_in_.wait();

        size_t message_count{ 0 };
        // calculate delta time
        double current_frame = glfwGetTime();
        delta_time_          = current_frame - last_frame_;
        last_frame_          = current_frame;
        while (message_count < max_messages && !messages_in_.empty())
        {

            // Grab the front message
            auto msg{ messages_in_.pop_front() };

            // Pass to message handler
            on_message(msg.remote, msg.msg);
            tick(delta_time_);

            ++message_count;
        }
    }

  private:
    void tick(float dt)
    {
        if (game_playing_)
        {
            update_ball(dt);
            do_collisions();
            broadcast_game_state();
        }
    }

    void update_ball(float dt)
    {
        // if not stuck to player board
        if (!ball_.stuck)
        {
            // move the ball
            ball_.pos += ball_.velocity * dt;
            // check if outside window bounds; if so, reverse velocity and
            // restore at correct pos
            if (ball_.pos.x <= 0.0f)
            {
                ball_.velocity.x = -ball_.velocity.x;
                ball_.pos.x      = 0.0f;
            }
            else if (ball_.pos.x + ball_.size.x >=
                     map_player_roster_[temp_player_id_].screen_info.width)
            {
                ball_.velocity.x = -ball_.velocity.x;
                ball_.pos.x = map_player_roster_[temp_player_id_].screen_info.width -
                              ball_.size.x;
            }
            if (ball_.pos.y <= 0.0f)
            {
                ball_.velocity.y = -ball_.velocity.y;
                ball_.pos.y      = 0.0f;
            }
            else if (ball_.pos.y + ball_.size.y >=
                     map_player_roster_[temp_player_id_].screen_info.height)
            {
                ball_.velocity.y = -ball_.velocity.y;
                ball_.pos.y =
                    map_player_roster_[temp_player_id_].screen_info.height -
                    ball_.size.y;
            }
        }
    }

    void broadcast_game_ends(PlayerNumber winner)
    {
        net::Message<GameMsgTypes> msg_game_ends{};
        msg_game_ends.header.id = GameMsgTypes::GameEnds;
        winner_                 = winner;
        msg_game_ends << winner_;
        message_all_clients(msg_game_ends);
        game_playing_ = false;
    }

    void broadcast_game_state()
    {
        // reduce player 1 and 2 lives
        PlayerDesc sending_player_desc{};

        for (auto& player_desc : map_player_roster_)
        {
            if (map_player_roster_.size() < 2)
            {
                broadcast_game_ends(player_desc.second.player_number);
            }

            if (player_desc.second.player_number == PlayerNumber::One &&
                player_desc.second.lives <= 0)
            {
                broadcast_game_ends(PlayerNumber::Two);
                break;
            }

            if (player_desc.second.player_number == PlayerNumber::Two &&
                player_desc.second.lives <= 0)
            {
                broadcast_game_ends(PlayerNumber::One);
                break;
            }

            if (player_desc.second.player_number == PlayerNumber::One &&
                ball_.pos.y >= player_desc.second.screen_info.height - ball_.size.y)
            {
                --player_desc.second.lives;
                sending_player_desc = player_desc.second;
                break;
            }

            if (player_desc.second.player_number == PlayerNumber::Two &&
                ball_.pos.y <= 0)
            {
                --player_desc.second.lives;
                sending_player_desc = player_desc.second;
                break;
            }
        }
        net::Message<GameMsgTypes> msg_reduce_lives{};
        msg_reduce_lives.header.id = GameMsgTypes::GameReduceLives;
        msg_reduce_lives << sending_player_desc;
        message_all_clients(msg_reduce_lives);

        net::Message<GameMsgTypes> msg_update_ball{};
        msg_update_ball.header.id = GameMsgTypes::GameUpdateBall;
        msg_update_ball << ball_;
        message_all_clients(msg_update_ball);
    }

  private:
    std::unordered_map<u32, PlayerDesc> map_player_roster_{};
    BallDesc ball_;
    std::vector<u32> garbage_ids_;
    bool has_player_one_{ false };
    const glm::vec2 initial_ball_velocity_{ 100.0f, -350.0f };
    const float ball_radius_{ 12.5f };
    const float player_velocity_{ 500.0f };
    u32 temp_player_id_{};

    PlayerNumber winner_{ PlayerNumber::Zero };

    double delta_time_{ 0.0 };
    double last_frame_{ 0.0 };

    bool game_playing_{ false };
};

int main()
{
    Server server{ 60000 };
    server.start();
    while (true)
    {
        server.update(65535, true);
    }
    return 0;
}
