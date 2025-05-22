#include <GameCommon/ResourceManager.h>
#include <GameCommon/Game.h>
#include <GameCommon/SpriteRenderer.h>

constexpr glm::vec2 PLAYER_SIZE{ 100.0f, 20.0f };
constexpr float PLAYER_VELOCITY{ 500.0f };

gc::GameObject* player;

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
    gc::ResourceManager::load_texture("res/textures/paddle.png", true, "paddle");

    // Load levels
    GameLevel one;
    one.load("res/levels/one.lvl", width_, height_ / 2);
    GameLevel two;
    two.load("res/levels/two.lvl", width_, height_ / 2);
    GameLevel three;
    three.load("res/levels/three.lvl", width_, height_ / 2);
    GameLevel four;
    four.load("res/levels/four.lvl", width_, height_ / 2);

    levels.push_back(one);
    levels.push_back(two);
    levels.push_back(three);
    levels.push_back(four);

    glm::vec2 player_pos{ glm::vec2{ width_ / 2.0f - PLAYER_SIZE.x / 2.0f,
                                     height_ - PLAYER_SIZE.y } };

    player = new GameObject{ player_pos,
                             PLAYER_SIZE,
                             ResourceManager::get_texture("paddle") };
}

void gc::Game::process_input(float dt)
{
    if (state == GameState::GAME_ACTIVE)
    {
        float velocity{ PLAYER_VELOCITY * dt };
        //move player's paddle
        if (keys[GLFW_KEY_A]) {
            if (player->pos().x >= 0.0f) {
                player->decrease_pos_x(velocity);
            }
        }
        if (keys[GLFW_KEY_D]) {
            if (player->pos().x <= width_ - player->size().x) {
                player->increase_pos_x(velocity);
            }
        }
    }
}

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

        player->draw(*renderer);
    }
}
