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

            client_screen_info_ = player_desc.screen_info;

            if (map_player_roster_.size() < 2)
            {
                player_desc.unique_id = client->id();
                map_player_roster_.insert_or_assign(player_desc.unique_id,
                                                    player_desc);

                net::Message<GameMsgTypes> msg_send_id{};
                msg_send_id.header.id = GameMsgTypes::ClientAssignId;
                msg_send_id << player_desc.unique_id;
                message_client(client, msg_send_id);

                net::Message<GameMsgTypes> msg_add_player{};
                msg_add_player.header.id = GameMsgTypes::GameAddPlayer;

                glm::vec2 player_pos{
                    glm::vec2{ client_screen_info_.width / 2.0f -
                                   map_player_roster_[player_desc.unique_id].size.x /
                                       2.0f,
                               0 },
                }; // position for player 2

                if (!has_player_one_)
                {
                    player_pos = glm::vec2{
                        client_screen_info_.width / 2.0f -
                            map_player_roster_[player_desc.unique_id].size.x / 2.0f,
                        client_screen_info_.height -
                            map_player_roster_[player_desc.unique_id].size.y
                    }; // Set the position for player 1

                    has_player_one_ = true;
                }
                else
                {
                    // Also add the ball now that we have 2 players
                    const glm::vec2 player_size{ 100.0f, 20.0f };
                    const float player_velocity{ 500.0f };

                    glm::vec2 player1_pos{ glm::vec2{
                        map_player_roster_[player_desc.unique_id].screen_info.width /
                                2.0f -
                            player_size.x / 2.0f,
                        map_player_roster_[player_desc.unique_id]
                                .screen_info.height -
                            player_size.y } };

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
                }
                player_desc.pos = player_pos;
                msg_add_player << player_desc;
                message_all_clients(msg_add_player);

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
            // Bounce update to everyone except incoming client
            message_all_clients(msg, client);
            break;
        }
        case GameMsgTypes::GamePlayerLaunchBall:
        {
            ball_.stuck = false;
            break;
        }
        }
    }

  public:
    void update(size_t max_messages, bool wait)
    {
        if (wait)
            messages_in_.wait();

        size_t message_count{ 0 };
        // calculate delta time
        double delta_time{ 0.0 };
        double current_frame = glfwGetTime();
        delta_time = current_frame - last_frame_;
        last_frame_ = current_frame;
        while (message_count < max_messages && !messages_in_.empty())
        {

            // Grab the front message
            auto msg{ messages_in_.pop_front() };

            tick(client_screen_info_, delta_time);
            // Pass to message handler
            on_message(msg.remote, msg.msg);

            ++message_count;
        }
    }

  private:
    void tick(ScreenInfo screen_info, float dt)
    {
        update_ball(screen_info, dt);
        broadcast_game_state();
    }

    void update_ball(ScreenInfo screen_info, float dt)
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
            else if (ball_.pos.x + ball_.size.x >= screen_info.width)
            {
                ball_.velocity.x = -ball_.velocity.x;
                ball_.pos.x      = screen_info.width - ball_.size.x;
            }
            if (ball_.pos.y <= 0.0f)
            {
                ball_.velocity.y = -ball_.velocity.y;
                ball_.pos.y      = 0.0f;
            }
            else if (ball_.pos.y + ball_.size.y >= screen_info.height)
            {
                ball_.velocity.y = -ball_.velocity.y;
                ball_.pos.y      = screen_info.height - ball_.size.y;
            }
        }
    }

    void broadcast_game_state()
    {
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
    ScreenInfo client_screen_info_{};

    double last_frame_{ 0.0 };
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