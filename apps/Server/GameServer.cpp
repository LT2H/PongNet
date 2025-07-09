#include <GameCommon/Common.h>
#include <NetCommon/NetCommon.h>
#include "../GameMsgTypes.h"
#include <GameCommon/PlayerDesc.h>
#include <string>
#include "GameCommon/ResourceManager.h"
#include "NetCommon/NetConnection.h"
#include "NetCommon/NetMessage.h"
#include "NetCommon/NetServer.h"

class Server : public net::ServerInterface<GameMsgTypes>
{
  public:
    Server(u16 port) : net::ServerInterface<GameMsgTypes>{ port } {}

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
        }
    }

  private:
    std::unordered_map<u32, PlayerDesc> map_player_roster_{};
    std::vector<u32> garbage_ids_;
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