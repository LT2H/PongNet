#include <GameCommon/TextRenderer.h>
#include <GameCommon/GameObject.h>
#include <GameCommon/ResourceManager.h>
#include <GameCommon/Game.h>
#include <GameCommon/SpriteRenderer.h>
#include <GameCommon/Common.h>
#include <GameCommon/BallObject.h>
#include <GameCommon/ParticleGenerator.h>
#include <GameCommon/PostProcessor.h>

constexpr glm::vec2 PLAYER_SIZE{ 100.0f, 20.0f };
constexpr float PLAYER_VELOCITY{ 500.0f };

// Initial velocity of the Ball
constexpr glm::vec2 INITIAL_BALL_VELOCITY{ 100.0f, -350.0f };

// Radius of the ball object
constexpr float BALL_RADIUS{ 12.5f };

float shake_time{ 0.0f };

gc::BallObject* ball;

gc::GameObject* player;

gc::SpriteRenderer* renderer;

gc::ParticleGenerator* particles;

gc::PostProcessor* effects;

gc::TextRender* text;

// Audio
ma_result result{};
ma_engine engine{};

gc::Game::Game(u32 width, u32 height)
    : state_{ GameState::GAME_MENU }, keys_{}, width_{ width }, height_{ height }
{
}

gc::Game::~Game()
{
    delete renderer;
    delete player;
    delete ball;
    delete effects;
    delete particles;
    delete text;
    ma_engine_uninit(&engine);
}

void gc::Game::init()
{
    // Load shaders
    ResourceManager::load_shader(
        "res/shaders/sprite.vert", "res/shaders/sprite.frag", "", "sprite");
    ResourceManager::load_shader(
        "res/shaders/particle.vert", "res/shaders/particle.frag", "", "particle");
    ResourceManager::load_shader("res/shaders/postprocessing.vert",
                                 "res/shaders/postprocessing.frag",
                                 "",
                                 "postprocessing");

    // configure shaders
    glm::mat4 projection{ glm::ortho(0.0f,
                                     static_cast<float>(width_),
                                     static_cast<float>(height_),
                                     0.0f,
                                     -1.0f,
                                     1.0f) };

    ResourceManager::get_shader("sprite").use().set_integer("image", 0);
    ResourceManager::get_shader("sprite").set_matrix4("projection", projection);

    ResourceManager::get_shader("particle").use().set_integer("sprite", 0);
    ResourceManager::get_shader("particle").set_matrix4("projection", projection);

    // Load textures
    ResourceManager::load_texture("res/textures/awesomeface.png", true, "face");
    ResourceManager::load_texture(
        "res/textures/background.jpg", false, "background");
    ResourceManager::load_texture("res/textures/block.png", false, "block");
    ResourceManager::load_texture(
        "res/textures/indestructible_block.png", false, "indestructible_block");
    ResourceManager::load_texture("res/textures/paddle.png", true, "paddle");

    ResourceManager::load_texture("res/textures/particle.png", true, "particle");

    ResourceManager::load_texture(
        "res/textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::load_texture(
        "res/textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::load_texture(
        "res/textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::load_texture(
        "res/textures/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::load_texture(
        "res/textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::load_texture(
        "res/textures/powerup_passthrough.png", true, "powerup_passthrough");

    // Set render-specific controls
    renderer = new gc::SpriteRenderer{ gc::ResourceManager::get_shader("sprite") };
    particles =
        new gc::ParticleGenerator{ gc::ResourceManager::get_shader("particle"),
                                   ResourceManager::get_texture("particle"),
                                   500 };
    effects = new PostProcessor{ ResourceManager::get_shader("postprocessing"),
                                 width_,
                                 height_ };

    // Load levels
    GameLevel one;
    one.load("res/levels/one.lvl", width_, height_ / 2);
    GameLevel two;
    two.load("res/levels/two.lvl", width_, height_ / 2);
    GameLevel three;
    three.load("res/levels/three.lvl", width_, height_ / 2);
    GameLevel four;
    four.load("res/levels/four.lvl", width_, height_ / 2);

    levels_.push_back(one);
    levels_.push_back(two);
    levels_.push_back(three);
    levels_.push_back(four);

    // configure game objects
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

    text = new TextRender{ width_, height_ };
    text->load("res/fonts/OCRAEXT.TTF", 24);

    // Main Menu theme
    result = ma_engine_init(nullptr, &engine);
    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio\n";
        return;
    }
    ma_engine_play_sound(&engine, "res/audio/breakout.mp3", nullptr);
}

void gc::Game::process_input(float dt)
{
    if (state_ == GameState::GAME_MENU)
    {
        if (keys_[GLFW_KEY_ENTER] && !keys_processed_[GLFW_KEY_ENTER])
        {
            state_ = GameState::GAME_ACTIVE;
            keys_processed_[GLFW_KEY_ENTER] = true;
        }
        if (keys_[GLFW_KEY_W] && !keys_processed_[GLFW_KEY_W])
        {
            current_level_ = (current_level_ + 1) % 4;
            keys_processed_[GLFW_KEY_W] = true;
        }
        if (keys_[GLFW_KEY_S] && !keys_processed_[GLFW_KEY_S])
        {
            if (current_level_ > 0)
            {
                --current_level_;
            }
            else
            {
                current_level_ = 3;
            }
            keys_processed_[GLFW_KEY_S] = true;
        }
    }

    if (state_ == GameState::GAME_ACTIVE)
    {
        float velocity{ PLAYER_VELOCITY * dt };
        // move player's paddle
        if (keys_[GLFW_KEY_A])
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
        if (keys_[GLFW_KEY_D])
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
        if (keys_[GLFW_KEY_SPACE])
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
    do_collisions();

    // update particles
    particles->update(dt, *ball, 2, glm::vec2{ ball->radius_ / 2.0f });

    update_powerups(dt);

    // reduce shake time
    if (shake_time > 0.0f)
    {
        shake_time -= dt;
        if (shake_time <= 0.0f)
        {
            effects->shake_ = false;
        }
    }

    // check loss condition
    if (ball->pos_.y >= height_)
    {
        --lives_;

        if (lives_ == 0)
        {
            reset_level();
            state_ = GameState::GAME_MENU;
        }
        reset_player();
    }
}

void gc::Game::reset_level()
{
    if (current_level_ == 0)
    {
        levels_[0].load("res/levels/one.lvl", width_, height_ / 2);
    }
    else if (current_level_ == 1)
    {
        levels_[1].load("res/levels/two.lvl", width_, height_ / 2);
    }
    else if (current_level_ == 2)
    {
        levels_[2].load("res/levels/three.lvl", width_, height_ / 2);
    }
    else if (current_level_ == 3)
    {
        levels_[3].load("res/levels/four.lvl", width_, height_ / 2);
    }

    lives_ = 3;
}

void gc::Game::reset_player()
{
    // reset player/ball stats
    player->size_ = PLAYER_SIZE;
    player->pos_ =
        glm::vec2{ width_ / 2.0f - PLAYER_SIZE.x / 2.0f, height_ - PLAYER_SIZE.y };
    ball->reset(player->pos_ + glm::vec2{ PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
                                          -(BALL_RADIUS * 2.0f) },
                INITIAL_BALL_VELOCITY);
    // also disable all active powerups
    effects->chaos_    = false;
    effects->confuse_  = false;
    ball->passthrough_ = false;
    ball->sticky_      = false;
    player->color_     = glm::vec3{ 1.0f };
    ball->color_       = glm::vec3{ 1.0f };
}


void gc::Game::render()
{
    if (state_ == GameState::GAME_ACTIVE || state_ == GameState::GAME_MENU)
    {
        effects->begin_render();
        // Draw background
        renderer->draw_sprite(gc::ResourceManager::get_texture("background"),
                              glm::vec2(0.0f, 0.0f),
                              glm::vec2(width_, height_),
                              0.0f);

        // Draw level
        levels_[current_level_].draw(*renderer);

        player->draw(*renderer);

        // NO
        for (auto& powerup : powerups_)
        {
            if (!powerup.destroyed_)
            {
                powerup.draw(*renderer);
            }
        }

        particles->draw();
        ball->draw(*renderer);


        effects->end_render();
        effects->render(glfwGetTime());

        // Show lives
        std::stringstream ss;
        ss << lives_;
        text->render_text("Lives" + ss.str(), 5.0f, 5.0f, 1.0f);
    }

    if (state_ == GameState::GAME_MENU)
    {
        text->render_text("Press ENTER to start", 250.0f, height_ / 2, 1.0f);
        text->render_text(
            "Press W or S to select level", 245.0f, height_ / 2 + 20.0f, 0.75f);
    }
}

void gc::Game::do_collisions()
{
    for (GameObject& box : levels_[current_level_].bricks)
    {
        if (!box.destroyed_)
        {
            Collision collision{ check_collision(*ball, box) };
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.is_solid_)
                {
                    box.destroyed_ = true;
                    spawn_powerups(box);
                    ma_engine_play_sound(&engine, "res/audio/bleep.mp3", nullptr);
                }
                else
                { // if block is solid, enable shake effect
                    shake_time      = 0.05f;
                    effects->shake_ = true;
                    ma_engine_play_sound(&engine, "res/audio/solid.mp3", nullptr);
                }

                // collision resolution
                Direction direction{ std::get<1>(collision) };
                glm::vec2 diff_vector{ std::get<2>(collision) };
                if (!(ball->passthrough_ &&
                      !box.is_solid_)) // don't do collision resolution on non-solid
                                       // bricks if pass-through is activated
                {
                    if (direction == Direction::LEFT ||
                        direction == Direction::RIGHT) // horizontal collision
                    {
                        ball->velocity_.x =
                            -ball->velocity_.x;        // reverse horizontal velocity
                        // relocate
                        float penetration{ ball->radius_ - std::abs(diff_vector.x) };
                        if (direction == Direction::LEFT)
                        {
                            ball->pos_.x += penetration; // move ball to right
                        }
                        else
                        {
                            ball->pos_.x -= penetration; // move ball to left;
                        }
                    }
                    else                                 // vertical collision
                    {
                        ball->velocity_.y =
                            -ball->velocity_.y;          // reverse vertical velocity
                        // relocate
                        float penetration{ ball->radius_ - std::abs(diff_vector.y) };
                        if (direction == Direction::UP)
                        {
                            ball->pos_.y -= penetration; // move ball back up
                        }
                        else
                        {
                            ball->pos_.y += penetration; // move ball back down
                        }
                    }
                }
            }
        }
    }
    // also check collisions on PowerUps and if so, activate them
    for (PowerUp& powerup : powerups_)
    {
        if (!powerup.destroyed_)
        {
            // first check if powerup passed bottom edge, if so: keep as inactive and
            // destroy
            if (powerup.pos_.y >= height_)
            {
                powerup.destroyed_ = true;
            }
            if (check_collision(*player, powerup))
            {
                // collided with player, now active powerup
                active_powerup(powerup);
                powerup.destroyed_ = true;
                powerup.activated_ = true;
                ma_engine_play_sound(&engine, "res/audio/powerup.wav", nullptr);
            }
        }
    }

    // and finally check collisions for player pad (unless stuck)
    Collision result{ check_collision(*ball, *player) };
    if (!ball->stuck_ && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit
        // the board
        float center_board{ player->pos_.x + player->size_.x / 2.0f };
        float distance{ ball->pos_.x + ball->radius_ - center_board };
        float percentage{ distance / (player->size_.x / 2.0f) };

        // then move accordingly
        float strength{ 2.0f };
        glm::vec2 old_velocity{ ball->velocity_ };
        ball->velocity_.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        // ball->velocity_.y = -ball->velocity_.y;
        ball->velocity_.y =
            -1.0f * abs(ball->velocity_.y); // avoid sticky paddle issue
        ball->velocity_ =
            glm::normalize(ball->velocity_) * glm::length(old_velocity);

        ball->stuck_ = ball->sticky_;

        ma_engine_play_sound(&engine, "res/audio/bleep.wav", nullptr);
    }
}

bool should_spawn(u32 chance)
{
    u32 random{ std::rand() % chance };
    return random == 0;
}

void gc::Game::spawn_powerups(GameObject& block)
{
    if (should_spawn(75)) // 1 in 75 chance
    {
        powerups_.emplace_back(
            PowerUp{ "speed",
                     glm::vec3{ 0.5f, 0.5f, 1.0f },
                     0.0f, // here a duration of 0.0f means its duration is infinite
                     block.pos_,
                     ResourceManager::get_texture("powerup_speed") });
    }
    if (should_spawn(75))
    {
        powerups_.emplace_back(
            PowerUp{ "sticky",
                     glm::vec3{ 1.0f, 0.5f, 1.0f },
                     20.0f,
                     block.pos_,
                     ResourceManager::get_texture("powerup_sticky") });
    }
    if (should_spawn(75))
    {
        powerups_.emplace_back(
            PowerUp{ "passthrough",
                     glm::vec3{ 0.5f, 1.0f, 0.5f },
                     10.0f,
                     block.pos_,
                     ResourceManager::get_texture("powerup_passthrough") });
    }
    if (should_spawn(75))
    {
        powerups_.emplace_back(
            PowerUp{ "pad-size-increase",
                     glm::vec3{ 1.0f, 0.6f, 0.4 },
                     0.0f,
                     block.pos_,
                     ResourceManager::get_texture("powerup_increase") });
    }
    if (should_spawn(15)) // Negative powerups should spawn more often
    {
        powerups_.emplace_back(
            PowerUp{ "confuse",
                     glm::vec3{ 1.0f, 0.3f, 0.3f },
                     15.0f,
                     block.pos_,
                     ResourceManager::get_texture("powerup_confuse") });
    }
    if (should_spawn(15))
    {
        powerups_.emplace_back(
            PowerUp{ "chaos",
                     glm::vec3{ 0.9f, 0.25f, 0.25f },
                     15.0f,
                     block.pos_,
                     ResourceManager::get_texture("powerup_chaos") });
    }
}

void gc::Game::active_powerup(const PowerUp& powerup)
{
    if (powerup.type_ == "speed")
    {
        ball->velocity_ *= 1.2;
    }
    else if (powerup.type_ == "sticky")
    {
        ball->sticky_  = true;
        player->color_ = glm::vec3{ 1.0f, 0.5f, 1.0f };
    }
    else if (powerup.type_ == "pass-through")
    {
        ball->passthrough_ = true;
        ball->color_       = glm::vec3{ 1.0f, 0.5f, 0.5f };
    }
    else if (powerup.type_ == "pad-size-increase")
    {
        player->size_.x += 50;
    }
    else if (powerup.type_ == "confuse")
    {
        if (!effects->chaos_)
        {
            effects->confuse_ = true; // only active if chaos wasn't already active
        }
    }
    else if (powerup.type_ == "chaos")
    {
        if (!effects->confuse_)
        {
            effects->chaos_ = true;
        }
    }
}

bool is_other_powerup_active(std::span<gc::PowerUp> powerups, std::string_view type)
{
    for (const auto& powerup : powerups)
    {
        if (powerup.activated_)
        {
            if (powerup.type_ == type)
            {
                return true;
            }
        }
    }
    return false;
}

void gc::Game::update_powerups(float dt)
{
    for (auto& powerup : powerups_)
    {
        powerup.pos_ += powerup.velocity_ * dt;
        if (powerup.activated_)
        {
            powerup.duration_ -= dt;

            if (powerup.duration_ <= 0.0f)
            {
                // remove powerup from list (will later be removed)
                powerup.activated_ = false;
                // deactive effects
                if (powerup.type_ == "sticky")
                {
                    if (!is_other_powerup_active(powerups_, "sticky"))
                    { // only reset if no other PowerUp of type sticky is active
                        ball->sticky_  = false;
                        player->color_ = glm::vec3{ 1.0f };
                    }
                }
                else if (powerup.type_ == "pass-through")
                {
                    if (!is_other_powerup_active(powerups_, "pass-through"))
                    {
                        ball->passthrough_ = false;
                        ball->color_       = glm::vec3{ 1.0f };
                    }
                }
                else if (powerup.type_ == "confuse")
                {
                    if (!is_other_powerup_active(powerups_, "confuse"))
                    {
                        ball->passthrough_ = false;
                        ball->color_       = glm::vec3{ 1.0f };
                    }
                }
                else if (powerup.type_ == "chaos")
                {
                    if (!is_other_powerup_active(powerups_, "chaos"))
                    {
                        ball->passthrough_ = false;
                        ball->color_       = glm::vec3{ 1.0f };
                    }
                }
            }
        }
    }
    // Remove all PowerUps from vector that are destroyed AND !activated (thus either
    // off the map or finished)
    // Note we use a lambda expression to remove each PowerUp which is destroyed and
    // not activated
    powerups_.erase(
        std::remove_if(powerups_.begin(),
                       powerups_.end(),
                       [](const PowerUp& powerup)
                       { return powerup.destroyed_ && !powerup.activated_; }),
        powerups_.end());
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

gc::Collision gc::Game::check_collision(const BallObject& one, const GameObject& two)
{
    // get center point circle first
    glm::vec2 center{ one.pos_ + one.radius_ };
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents{ two.size_.x / 2.0f, two.size_.y / 2.0f };
    glm::vec2 aabb_center{ two.pos_.x + aabb_half_extents.x,
                           two.pos_.y + aabb_half_extents.y };
    // get difference vector between both centers
    glm::vec2 difference{ center - aabb_center };
    glm::vec2 clamped{ glm::clamp(
        difference, -aabb_half_extents, aabb_half_extents) };
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest{ aabb_center + clamped };
    // retrieve vector between center circle and closest point AABB and check if
    // length <= radius
    difference = closest - center;
    if (glm::length(difference) <= one.radius_)
        return std::make_tuple(true, vector_direction(difference), difference);
    else
        return std::make_tuple(false, gc::Direction::UP, glm::vec2(0.0f, 0.0f));
}

gc::Direction gc::Game::vector_direction(const glm::vec2& target)
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
    return static_cast<Direction>(best_match);
}
