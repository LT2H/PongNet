#include <GameCommon/TextRenderer.h>
#include <GameCommon/GameObject.h>
#include <GameCommon/Player.h>
#include <GameCommon/ResourceManager.h>
#include <GameCommon/Game.h>
#include <GameCommon/SpriteRenderer.h>
#include <GameCommon/Common.h>
#include <GameCommon/BallObject.h>
#include <GameCommon/ParticleGenerator.h>
#include <GameCommon/PostProcessor.h>

std::array<bool, 1024> gc::Game::keys_{};
std::array<bool, 1024> gc::Game::keys_processed_{};

gc::Game::Game(u32 width, u32 height)
    : state_{ GameState::GAME_MENU }, width_{ width }, height_{ height }
{
    window_ = nullptr;
}

gc::Game::~Game() { ma_engine_uninit(&engine_); }

bool gc::Game::init()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    // glfw window creation
    // --------------------
    window_ = glfwCreateWindow(width_, height_, "Breakout", nullptr, nullptr);
    if (!window_)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window_);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glfwSetKeyCallback(window_, key_callback);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

    // OpenGL configuration
    // --------------------
    glViewport(0, 0, width_, height_);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    sprite_renderer_ = std::make_unique<gc::SpriteRenderer>(
        gc::ResourceManager::get_shader("sprite"));
    particles_ = std::make_unique<gc::ParticleGenerator>(
        gc::ResourceManager::get_shader("particle"),
        ResourceManager::get_texture("particle"),
        500);
    effects_ = std::make_unique<PostProcessor>(
        ResourceManager::get_shader("postprocessing"), width_, height_);

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
    // Player 1
    glm::vec2 player1_pos{ glm::vec2{ width_ / 2.0f - player_size_.x / 2.0f,
                                      height_ - player_size_.y } };

    player1_ = std::make_unique<Player>(
        3, player1_pos, player_size_, ResourceManager::get_texture("paddle"));

    glm::vec2 ball_pos{ player1_pos + glm::vec2{ player_size_.x / 2.0f - ball_radius_,
                                                 -ball_radius_ * 2.0f } };

    ball_ = std::make_unique<BallObject>(ball_pos,
                                         ball_radius_,
                                         initial_ball_velocity_,
                                         ResourceManager::get_texture("face"));

    // Player 2
    glm::vec2 player2_pos{ glm::vec2{ width_ / 2.0f - player_size_.x / 2.0f, 0 } };

    player2_ = std::make_unique<Player>(
        3, player2_pos, player_size_, ResourceManager::get_texture("paddle"));

    text_ = std::make_unique<TextRender>(width_, height_);
    text_->load("res/fonts/OCRAEXT.TTF", font_size_);

    // Main Menu theme
    result_ = ma_engine_init(nullptr, &engine_);
    if (result_ != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio\n";
        return false;
    }
    ma_engine_play_sound(&engine_, "res/audio/breakout.mp3", nullptr);

    return true;
}

void gc::Game::run()
{
    // Delta time vars
    double delta_time{ 0.0f };
    double last_frame{ 0.0f };
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window_))
    {
        // calculate delta time
        double current_frame{ glfwGetTime() };
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        glfwPollEvents();

        // manage users input
        // -----------------
        process_player1_input(delta_time);
        process_player2_input(delta_time);

        // update game state
        // -----------------
        update(delta_time);

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        render();

        glfwSwapBuffers(window_);
    }

    // The sprite renderer must be reset before cleaning up other resources.
    // Otherwise a segmentation fault will occur
    // ---------------------------------------------------------
    shutdown();

    // delete all resources as loaded using the resource manager
    // ---------------------------------------------------------
    gc::ResourceManager::clear();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

void gc::Game::process_player1_input(float dt)
{
    if (state_ == GameState::GAME_MENU)
    {
        if (keys_[GLFW_KEY_ENTER] && !keys_processed_[GLFW_KEY_ENTER])
        {
            state_                          = GameState::GAME_ACTIVE;
            keys_processed_[GLFW_KEY_ENTER] = true;
        }
        if (keys_[GLFW_KEY_W] && !keys_processed_[GLFW_KEY_W])
        {
            current_level_              = (current_level_ + 1) % 4;
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

    if (state_ == GameState::GAME_WIN)
    {
        if (keys_[GLFW_KEY_ENTER])
        {
            keys_processed_[GLFW_KEY_ENTER] = true;
            effects_->chaos_                = false;
            state_                          = GameState::GAME_MENU;
        }
    }

    if (state_ == GameState::GAME_ACTIVE)
    {
        float velocity{ player_velocity_ * dt };
        // move player1_'s paddle
        if (keys_[GLFW_KEY_A])
        {
            if (player1_->pos_.x >= 0.0f)
            {
                player1_->pos_.x -= velocity;
                if (ball_->stuck_)
                {
                    ball_->pos_.x -= velocity;
                }
            }
        }
        if (keys_[GLFW_KEY_D])
        {
            if (player1_->pos_.x <= width_ - player1_->size_.x)
            {
                player1_->pos_.x += velocity;
                if (ball_->stuck_)
                {
                    ball_->pos_.x += velocity;
                }
            }
        }
        if (keys_[GLFW_KEY_SPACE])
        {
            ball_->stuck_ = false;
        }
    }
}

void gc::Game::process_player2_input(float dt)
{
    if (state_ == GameState::GAME_MENU)
    {
        if (keys_[GLFW_KEY_ENTER] && !keys_processed_[GLFW_KEY_ENTER])
        {
            state_                          = GameState::GAME_ACTIVE;
            keys_processed_[GLFW_KEY_ENTER] = true;
        }
        if (keys_[GLFW_KEY_UP] && !keys_processed_[GLFW_KEY_UP])
        {
            current_level_               = (current_level_ + 1) % 4;
            keys_processed_[GLFW_KEY_UP] = true;
        }
        if (keys_[GLFW_KEY_DOWN] && !keys_processed_[GLFW_KEY_DOWN])
        {
            if (current_level_ > 0)
            {
                --current_level_;
            }
            else
            {
                current_level_ = 3;
            }
            keys_processed_[GLFW_KEY_DOWN] = true;
        }
    }

    if (state_ == GameState::GAME_WIN)
    {
        if (keys_[GLFW_KEY_ENTER])
        {
            keys_processed_[GLFW_KEY_ENTER] = true;
            effects_->chaos_                = false;
            state_                          = GameState::GAME_MENU;
        }
    }

    if (state_ == GameState::GAME_ACTIVE)
    {
        float velocity{ player_velocity_ * dt };
        // move player2_'s paddle
        if (keys_[GLFW_KEY_LEFT])
        {
            if (player2_->pos_.x >= 0.0f)
            {
                player2_->pos_.x -= velocity;
            }
        }
        if (keys_[GLFW_KEY_RIGHT])
        {
            if (player2_->pos_.x <= width_ - player2_->size_.x)
            {
                player2_->pos_.x += velocity;
            }
        }
    }
}


bool gc::Game::update(float dt)
{
    // update objects
    ball_->move(dt, width_, height_);

    // Check for collisions
    do_collisions();

    // update particles_
    particles_->update(dt, *ball_, 2, glm::vec2{ ball_->radius_ / 2.0f });

    update_powerups(dt);

    // reduce shake time
    if (shake_time_ > 0.0f)
    {
        shake_time_ -= dt;
        if (shake_time_ <= 0.0f)
        {
            effects_->shake_ = false;
        }
    }

    // reduce player 1 lives
    if (ball_->pos_.y >= height_ - ball_->size_.y)
    {
        --player1_->lives_;
    }

    // reduce player 2 lives
    if (ball_->pos_.y <= 0)
    {
        --player2_->lives_;
    }

    // check win condition of player 1
    if (state_ == GameState::GAME_ACTIVE && player2_->lives_ == 0)
    {
        reset_players();
        reset_level();
        effects_->chaos_ = true;
        state_           = GameState::GAME_WIN;
        winner_          = Winner::Player1;
    }

    // TODO: check win condition of player 2
    if (state_ == GameState::GAME_ACTIVE && player1_->lives_ == 0)
    {
        reset_players();
        reset_level();
        effects_->chaos_ = true;
        state_           = GameState::GAME_WIN;
        winner_          = Winner::Player2;
    }

    return 0;
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

    player1_->lives_ = 3;
    player2_->lives_ = 3;

    winner_ = Winner::NoOne;
}

void gc::Game::reset_players()
{
    // reset player1_/ball_ stats
    player1_->size_ = player_size_;
    player1_->pos_ =
        glm::vec2{ width_ / 2.0f - player_size_.x / 2.0f, height_ - player_size_.y };
    ball_->reset(player1_->pos_ + glm::vec2{ player_size_.x / 2.0f - ball_radius_,
                                             -(ball_radius_ * 2.0f) },
                 initial_ball_velocity_);
    // also disable all active powerups
    effects_->chaos_    = false;
    effects_->confuse_  = false;
    ball_->passthrough_ = false;
    ball_->sticky_      = false;
    player1_->color_    = glm::vec3{ 1.0f };
    ball_->color_       = glm::vec3{ 1.0f };

    // TODO: disable all active powerups for Player2?
    player2_->size_  = player_size_;
    player2_->pos_   = glm::vec2{ width_ / 2.0f - player_size_.x / 2.0f, 0 };
    player2_->color_ = glm::vec3{ 1.0f };
}


void gc::Game::render()
{
    if (state_ == GameState::GAME_ACTIVE || state_ == GameState::GAME_MENU ||
        state_ == GameState::GAME_WIN)
    {
        effects_->begin_render();
        // Draw background
        sprite_renderer_->draw_sprite(gc::ResourceManager::get_texture("background"),
                                      glm::vec2(0.0f, 0.0f),
                                      glm::vec2(width_, height_),
                                      0.0f);

        // Draw level
        levels_[current_level_].draw(*sprite_renderer_);

        player1_->draw(*sprite_renderer_);
        player2_->draw(*sprite_renderer_);

        // NO
        for (auto& powerup : powerups_)
        {
            if (!powerup.destroyed_)
            {
                powerup.draw(*sprite_renderer_);
            }
        }

        particles_->draw();
        ball_->draw(*sprite_renderer_);

        effects_->end_render();
        effects_->render(glfwGetTime());

        // Show lives
        std::stringstream ss;
        ss << player2_->lives_;
        text_->render_text("Lives " + ss.str(), 5.0f, 5.0f, 1.0f);
        ss.str("");
        ss.clear();
        ss << player1_->lives_;
        text_->render_text("Lives " + ss.str(), 5.0f, height_ - font_size_, 1.0f);
    }

    if (state_ == GameState::GAME_MENU)
    {
        text_->render_text("Press ENTER to start", 250.0f, height_ / 2, 1.0f);
        text_->render_text(
            "Press W or S to select level", 245.0f, height_ / 2 + 20.0f, 0.75f);
    }

    if (state_ == GameState::GAME_WIN)
    {
        std::string winner{};
        if (winner_ == Winner::Player1)
        {
            winner = "PLAYER 1";
        }
        else if (winner_ == Winner::Player2)
        {
            winner = "PLAYER 2";
        }

        text_->render_text(winner + " WON!",
                           320.0,
                           height_ / 2 - 20.0,
                           1.0,
                           glm::vec3(0.0, 1.0, 0.0));
        text_->render_text("Press ENTER to retry or ESC to quit",
                           130.0,
                           height_ / 2,
                           1.0,
                           glm::vec3(1.0, 1.0, 0.0));
    }
}

void gc::Game::do_collisions()
{
    for (GameObject& box : levels_[current_level_].bricks)
    {
        if (!box.destroyed_)
        {
            Collision collision{ check_collision(*ball_, box) };
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.is_solid_)
                {
                    box.destroyed_ = true;
                    spawn_powerups(box);
                    ma_engine_play_sound(&engine_, "res/audio/bleep.mp3", nullptr);
                }
                else
                { // if block is solid, enable shake effect
                    shake_time_       = 0.05f;
                    effects_->shake_ = true;
                    ma_engine_play_sound(&engine_, "res/audio/solid.mp3", nullptr);
                }

                // collision resolution
                Direction direction{ std::get<1>(collision) };
                glm::vec2 diff_vector{ std::get<2>(collision) };
                if (!(ball_->passthrough_ &&
                      !box.is_solid_)) // don't do collision resolution on non-solid
                                       // bricks if pass-through is activated
                {
                    if (direction == Direction::LEFT ||
                        direction == Direction::RIGHT) // horizontal collision
                    {
                        ball_->velocity_.x =
                            -ball_->velocity_.x;       // reverse horizontal velocity
                        // relocate
                        float penetration{ ball_->radius_ -
                                           std::abs(diff_vector.x) };
                        if (direction == Direction::LEFT)
                        {
                            ball_->pos_.x += penetration; // move ball_ to right
                        }
                        else
                        {
                            ball_->pos_.x -= penetration; // move ball_ to left;
                        }
                    }
                    else                                  // vertical collision
                    {
                        ball_->velocity_.y =
                            -ball_->velocity_.y; // reverse vertical velocity
                        // relocate
                        float penetration{ ball_->radius_ -
                                           std::abs(diff_vector.y) };
                        if (direction == Direction::UP)
                        {
                            ball_->pos_.y -= penetration; // move ball_ back up
                        }
                        else
                        {
                            ball_->pos_.y += penetration; // move ball_ back down
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
            if (check_collision(*player1_, powerup))
            {
                // collided with player1_, now active powerup
                active_powerup(powerup);
                powerup.destroyed_ = true;
                powerup.activated_ = true;
                ma_engine_play_sound(&engine_, "res/audio/powerup.wav", nullptr);
            }
        }
    }

    // and finally check collisions for player1_ pad (unless stuck)
    Collision result{ check_collision(*ball_, *player1_) };
    if (!ball_->stuck_ && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit
        // the board
        float center_board{ player1_->pos_.x + player1_->size_.x / 2.0f };
        float distance{ ball_->pos_.x + ball_->radius_ - center_board };
        float percentage{ distance / (player1_->size_.x / 2.0f) };

        // then move accordingly
        float strength{ 2.0f };
        glm::vec2 old_velocity{ ball_->velocity_ };
        ball_->velocity_.x = initial_ball_velocity_.x * percentage * strength;
        // ball_->velocity_.y = -ball_->velocity_.y;
        ball_->velocity_.y =
            -1.0f * abs(ball_->velocity_.y); // avoid sticky paddle issue
        ball_->velocity_ =
            glm::normalize(ball_->velocity_) * glm::length(old_velocity);

        ball_->stuck_ = ball_->sticky_;

        ma_engine_play_sound(&engine_, "res/audio/bleep.wav", nullptr);
    }

    // check collisions for player2_ pad
    result = check_collision(*ball_, *player2_);
    if (!ball_->stuck_ && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit
        // the board
        float center_board{ player2_->pos_.x + player2_->size_.x / 2.0f };
        float distance{ ball_->pos_.x + ball_->radius_ - center_board };
        float percentage{ distance / (player2_->size_.x / 2.0f) };

        // then move accordingly
        float strength{ 2.0f };
        glm::vec2 old_velocity{ ball_->velocity_ };
        ball_->velocity_.x = initial_ball_velocity_.x * percentage * strength;
        // ball_->velocity_.y = -ball_->velocity_.y;
        ball_->velocity_.y =
            -1.0f * abs(ball_->velocity_.y); // avoid sticky paddle issue
        ball_->velocity_ =
            glm::normalize(-ball_->velocity_) *
            glm::length(old_velocity); // Flip the velocity_ of the player2 pad to
                                       // shoot the ball downward

        ball_->stuck_ = ball_->sticky_;

        ma_engine_play_sound(&engine_, "res/audio/bleep.wav", nullptr);
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
        ball_->velocity_ *= 1.2;
    }
    else if (powerup.type_ == "sticky")
    {
        ball_->sticky_   = true;
        player1_->color_ = glm::vec3{ 1.0f, 0.5f, 1.0f };
    }
    else if (powerup.type_ == "pass-through")
    {
        ball_->passthrough_ = true;
        ball_->color_       = glm::vec3{ 1.0f, 0.5f, 0.5f };
    }
    else if (powerup.type_ == "pad-size-increase")
    {
        player1_->size_.x += 50;
    }
    else if (powerup.type_ == "confuse")
    {
        if (!effects_->chaos_)
        {
            effects_->confuse_ = true; // only active if chaos wasn't already active
        }
    }
    else if (powerup.type_ == "chaos")
    {
        if (!effects_->confuse_)
        {
            effects_->chaos_ = true;
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
                // deactive effects_
                if (powerup.type_ == "sticky")
                {
                    if (!is_other_powerup_active(powerups_, "sticky"))
                    { // only reset if no other PowerUp of type sticky is active
                        ball_->sticky_   = false;
                        player1_->color_ = glm::vec3{ 1.0f };
                    }
                }
                else if (powerup.type_ == "pass-through")
                {
                    if (!is_other_powerup_active(powerups_, "pass-through"))
                    {
                        ball_->passthrough_ = false;
                        ball_->color_       = glm::vec3{ 1.0f };
                    }
                }
                else if (powerup.type_ == "confuse")
                {
                    if (!is_other_powerup_active(powerups_, "confuse"))
                    {
                        ball_->passthrough_ = false;
                        ball_->color_       = glm::vec3{ 1.0f };
                    }
                }
                else if (powerup.type_ == "chaos")
                {
                    if (!is_other_powerup_active(powerups_, "chaos"))
                    {
                        ball_->passthrough_ = false;
                        ball_->color_       = glm::vec3{ 1.0f };
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

void gc::Game::shutdown() { sprite_renderer_.reset(); }

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void gc::Game::key_callback(GLFWwindow* window, int key, int scancode, int action,
                            int mode)
{
    // when a user presses the escape key, we set the WindowShouldClose property to
    // true, closing the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys_[key] = true;
        else if (action == GLFW_RELEASE)
        {
            keys_[key]           = false;
            keys_processed_[key] = false;
        }
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void gc::Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}