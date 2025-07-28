#pragma once

#include <NetCommon/NetCommon.h>
#include "../GameMsgTypes.h"

class Client : public net::ClientInterface<GameMsgTypes>
{
  public:
    std::array<char, net::array_size>& ip_to_connect()
    {
        return ip_to_connect_;
    }

  private:
    std::array<char, net::array_size> ip_to_connect_{};
};