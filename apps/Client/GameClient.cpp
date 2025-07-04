#include "OnlineGame.h"
#include <GameCommon/Common.h>

constexpr u32 SCR_WIDTH{ 800 };
constexpr u32 SCR_HEIGHT{ 600 };

int main()
{
    OnlineGame game{ SCR_WIDTH, SCR_HEIGHT };
    if (game.init())
    {
        game.run();
        std::cout << "Game is running\n";
    }
    
    return 0;
}