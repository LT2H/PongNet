#include <GameCommon/Common.h>
#include <NetCommon/NetCommon.h>
#include "../GameMsgTypes.h"
#include "GameCommon/Player.h"
#include "GameCommon/ResourceManager.h"
#include "NetCommon/NetConnection.h"
#include "NetCommon/NetMessage.h"
#include "NetCommon/NetServer.h"
#include "glm/fwd.hpp"

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
    }

    void on_message(std::shared_ptr<net::Connection<GameMsgTypes>> client,
                    net::Message<GameMsgTypes>& msg) override
    {
        switch (msg.header.id)
        {
        case GameMsgTypes::ClientRegisterWithServer:
        {
            gc::Player player{ 0,
                               glm::vec2{ 0.0f, 0.0f },
                               glm::vec2{ 100.0f, 20.0f },
                               gc::ResourceManager::get_texture("paddle") };
            msg >> player;
            player.unique_id_ = client->id();
            map_player_roster_.insert_or_assign(player.unique_id_, player);

            net::Message<GameMsgTypes> msg_send_id{};
            msg_send_id.header.id = GameMsgTypes::ClientAssignId;
            msg_send_id << player.unique_id_;
            message_client(client, msg_send_id);

            net::Message<GameMsgTypes> msg_add_player{};
            msg_add_player.header.id = GameMsgTypes::GameAddPlayer;
            msg_add_player << player;
            message_all_clients(msg_add_player);
        }
        }
    }

  private:
    std::unordered_map<u32, gc::Player> map_player_roster_{};
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