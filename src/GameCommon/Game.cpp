#include <GameCommon/Game.h>

gc::Game::Game(unsigned int width, unsigned int height)
    : state{ GameState::GAME_ACTIVE }, keys{}, width{}, height{}
{
}

gc::Game::~Game() {}

void gc::Game::init() {}

void gc::Game::process_input(float dt) {}

void gc::Game::update(float dt) {}

void gc::Game::render() {}
