#include <GameCommon/ResourceManager.h>
#include <GameCommon/Game.h>
#include <GameCommon/SpriteRenderer.h>

gc::SpriteRenderer* renderer;

gc::Game::Game(u32 width, u32 height)
    : state{ GameState::GAME_ACTIVE }, keys{}, width_{ width }, height_{ height }
{
}

gc::Game::~Game() { delete renderer; }

void gc::Game::init()
{
    // Load shaders
    gc::ResourceManager::load_shader(
        "res/shaders/shader.vert", "res/shaders/shader.frag", "", "sprite");

    // configure shaders
    glm::mat4 projection{ glm::ortho(0.0f,
                                     static_cast<float>(width_),
                                     static_cast<float>(height_),
                                     0.0f,
                                     -1.0f,
                                     1.0f) };

    gc::ResourceManager::get_shader("sprite").use().set_integer("image", 0);

    gc::ResourceManager::get_shader("sprite").set_matrix4("projection", projection);

    // Set render-specific controls
    renderer = new gc::SpriteRenderer{ gc::ResourceManager::get_shader("sprite") };

    // Load textures
    gc::ResourceManager::load_texture("res/textures/awesomeface.png", true, "face");
    gc::ResourceManager::load_texture(
        "res/textures/background.jpg", false, "background");
    gc::ResourceManager::load_texture("res/textures/block.png", false, "block");
    gc::ResourceManager::load_texture(
        "res/textures/indestructible_block.png", false, "indestructible_block");

    // Load levels
    GameLevel one;
    one.load("res/levels/one.lvl", width_, height_);
    GameLevel two;
    two.load("res/levels/two.lvl", width_, height_);
    GameLevel three;
    three.load("res/levels/three.lvl", width_, height_);
    GameLevel four;
    four.load("res/levels/four.lvl", width_, height_);
}

void gc::Game::process_input(float dt) {}

void gc::Game::update(float dt) {}

void gc::Game::render()
{
    if (state == GameState::GAME_ACTIVE)
    {
        // Draw background
        renderer->draw_sprite(gc::ResourceManager::get_texture("background"),
                              glm::vec2(0.0f, 0.0f),
                              glm::vec2(width_, height_),
                              0.0f);

        // Draw level
        levels[current_level].draw(*renderer);
    }
}
