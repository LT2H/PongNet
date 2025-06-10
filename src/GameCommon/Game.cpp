#include <GameCommon/ResourceManager.h>
#include <GameCommon/Game.h>
#include <GameCommon/SpriteRenderer.h>
#include <GameCommon/Common.h>
#include <GameCommon/BallObject.h>

constexpr glm::vec2 PLAYER_SIZE{ 100.0f, 20.0f };
constexpr float PLAYER_VELOCITY{ 500.0f };

// Initial velocity of the Ball
constexpr glm::vec2 INITIAL_BALL_VELOCITY{ 100.0f, -350.0f };

// Radius of the ball object
constexpr float BALL_RADIUS{ 12.5f };

gc::BallObject* ball;

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

    glm::vec2 ball_pos{ player_pos + glm::vec2{ PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
                                                -BALL_RADIUS * 2.0f } };

    ball = new BallObject{ ball_pos,
                           BALL_RADIUS,
                           INITIAL_BALL_VELOCITY,
                           ResourceManager::get_texture("face") };
}

void gc::Game::process_input(float dt)
{
    if (state == GameState::GAME_ACTIVE)
    {
        float velocity{ PLAYER_VELOCITY * dt };
        // move player's paddle
        if (keys[GLFW_KEY_A])
        {
            if (player->pos_.x >= 0.0f)
            {
                player->pos_.x -= velocity;
                if (ball->stuck_)
                {
                    ball->pos_.x -= velocity;
                }
            }
        }
        if (keys[GLFW_KEY_D])
        {
            if (player->pos_.x <= width_ - player->size_.x)
            {
                player->pos_.x += velocity;
                if (ball->stuck_)
                {
                    ball->pos_.x += velocity;
                }
            }
        }
        if (keys[GLFW_KEY_SPACE])
        {
            ball->stuck_ = false;
        }
    }
}

void gc::Game::update(float dt)
{
    // update objects
    ball->move(dt, width_);

    // Check for collisions
    do_collision();
}

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
        ball->draw(*renderer);
    }
}

void gc::Game::do_collision()
{
    for (GameObject& box : levels[current_level].bricks)
    {
        if (!box.destroyed_)
        {
            if (check_collision(*ball, box))
            {
                if (!box.is_solid_)
                {
                    box.destroyed_ = true;
                }
            }
        }
    }
}

bool gc::Game::check_collision(const GameObject& one, const GameObject& two)
{
    // collsion x-axis?
    bool collision_x{ one.pos_.x + one.size_.x >= two.pos_.x &&
                      two.pos_.x + two.size_.x >= one.pos_.x };

    // collision y-axis?
    bool collision_y{ one.pos_.y + one.size_.y >= two.pos_.y &&
                      two.pos_.y + two.size_.y >= one.pos_.y };

    // collision only if on both axes
    return collision_x && collision_y;
}
