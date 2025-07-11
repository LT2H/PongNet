#pragma once

#include "GameLevel.h"
#include "Common.h"
#include "GameObject.h"
#include "BallObject.h"
#include "PowerUp.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "ScreenInfo.h"
#include "TextRenderer.h"

namespace gc
{

class Player;

enum class GameState
{
    GAME_ACTIVE,
    GAME_MENU,
    GAME_CONNECT_TO_SERVER,
    GAME_WIN,
};

enum class Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

enum class Winner
{
    NoOne,
    Player1,
    Player2,
};

using Collision = std::tuple<bool, Direction, glm::vec2>;

class Game
{
  public:
    Game(u32 width, u32 height);

    virtual ~Game();

    virtual bool init();
    virtual void run();

  private:
    void process_player1_input(float dt);
    void process_player2_input(float dt);

  protected:
    virtual bool update(float dt);

    void reset_level();

    void reset_players();

    virtual void render();

    virtual void do_collisions();

    void spawn_powerups(GameObject& block);
    void active_powerup(const PowerUp& powerup);
    void update_powerups(float dt);

    // Reset the sprite renderer before cleaning up other resources
    void shutdown();

    GameState state_;

    static std::array<bool, 1024> keys_;
    static std::array<bool, 1024> keys_processed_;

    std::vector<GameLevel> levels_{};
    u32 current_level_{ 0 };

    std::vector<PowerUp> powerups_{};

  protected:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                             int mode);

    GLFWwindow* window_;

    bool check_collision(const GameObject& one, const GameObject& two);

    Collision check_collision(const BallObject& one, const GameObject& two);

    Direction vector_direction(const glm::vec2& target);

    // u32 width_;
    // u32 height_;
    ScreenInfo screen_info_;

    std::unique_ptr<BallObject> ball_;

  private:
    std::unique_ptr<Player> player1_;
    std::unique_ptr<Player> player2_;

  protected:
    Winner winner_{ Winner::NoOne };

    std::unique_ptr<SpriteRenderer> sprite_renderer_;

    std::unique_ptr<ParticleGenerator> particles_;

    std::unique_ptr<PostProcessor> effects_;

    std::unique_ptr<TextRender> text_;

    // Audio
    ma_result result_{};
    ma_engine engine_{};

  protected:
    float shake_time_{ 0.0f };

    const glm::vec2 player_size_{ 100.0f, 20.0f };
    const float player_velocity_{ 500.0f };

    // Initial velocity of the Ball
    const glm::vec2 initial_ball_velocity_{ 100.0f, -350.0f };

    // Radius of the ball_ object
    const float ball_radius_{ 12.5f };

    const u32 font_size_{ 24 };
};
} // namespace gc