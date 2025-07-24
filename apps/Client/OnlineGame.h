#pragma once

#include <GameCommon/Game.h>
#include <GameCommon/Player.h>
#include "Client.h"
#include "GLFW/glfw3.h"
#include "GameCommon/BallDesc.h"
#include "GameCommon/BallObject.h"
#include "NetCommon/NetMessage.h"
#include <GameCommon/ResourceManager.h>
#include <GameCommon/PlayerDesc.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

class OnlineGame : public gc::Game
{

  public:
    OnlineGame(u32 width, u32 height) : gc::Game(width, height) {}

    bool init() override
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
        window_ = glfwCreateWindow(
            screen_info_.width, screen_info_.height, "Pong Net", nullptr, nullptr);
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
        glViewport(0, 0, screen_info_.width, screen_info_.height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Load shaders
        gc::ResourceManager::load_shader(
            "res/shaders/sprite.vert", "res/shaders/sprite.frag", "", "sprite");
        gc::ResourceManager::load_shader("res/shaders/particle.vert",
                                         "res/shaders/particle.frag",
                                         "",
                                         "particle");
        gc::ResourceManager::load_shader("res/shaders/postprocessing.vert",
                                         "res/shaders/postprocessing.frag",
                                         "",
                                         "postprocessing");

        // configure shaders
        glm::mat4 projection{ glm::ortho(0.0f,
                                         static_cast<float>(screen_info_.width),
                                         static_cast<float>(screen_info_.height),
                                         0.0f,
                                         -1.0f,
                                         1.0f) };

        gc::ResourceManager::get_shader("sprite").use().set_integer("image", 0);
        gc::ResourceManager::get_shader("sprite").set_matrix4("projection",
                                                              projection);

        gc::ResourceManager::get_shader("particle").use().set_integer("sprite", 0);
        gc::ResourceManager::get_shader("particle")
            .set_matrix4("projection", projection);

        // Load textures
        gc::ResourceManager::load_texture(
            "res/textures/awesomeface.png", true, "face");
        gc::ResourceManager::load_texture(
            "res/textures/background.jpg", false, "background");
        gc::ResourceManager::load_texture("res/textures/block.png", false, "block");
        gc::ResourceManager::load_texture(
            "res/textures/indestructible_block.png", false, "indestructible_block");
        gc::ResourceManager::load_texture("res/textures/paddle.png", true, "paddle");

        gc::ResourceManager::load_texture(
            "res/textures/particle.png", true, "particle");

        gc::ResourceManager::load_texture(
            "res/textures/powerup_speed.png", true, "powerup_speed");
        gc::ResourceManager::load_texture(
            "res/textures/powerup_sticky.png", true, "powerup_sticky");
        gc::ResourceManager::load_texture(
            "res/textures/powerup_increase.png", true, "powerup_increase");
        gc::ResourceManager::load_texture(
            "res/textures/powerup_confuse.png", true, "powerup_confuse");
        gc::ResourceManager::load_texture(
            "res/textures/powerup_chaos.png", true, "powerup_chaos");
        gc::ResourceManager::load_texture(
            "res/textures/powerup_passthrough.png", true, "powerup_passthrough");

        // Set render-specific controls
        sprite_renderer_ = std::make_unique<gc::SpriteRenderer>(
            gc::ResourceManager::get_shader("sprite"));
        particles_ = std::make_unique<gc::ParticleGenerator>(
            gc::ResourceManager::get_shader("particle"),
            gc::ResourceManager::get_texture("particle"),
            500);
        effects_ = std::make_unique<gc::PostProcessor>(
            gc::ResourceManager::get_shader("postprocessing"),
            screen_info_.width,
            screen_info_.height);

        // Load levels
        gc::GameLevel one;
        one.load("res/levels/one.lvl", screen_info_.width, screen_info_.height / 2);
        gc::GameLevel two;
        two.load("res/levels/two.lvl", screen_info_.width, screen_info_.height / 2);
        gc::GameLevel three;
        three.load(
            "res/levels/three.lvl", screen_info_.width, screen_info_.height / 2);
        gc::GameLevel four;
        four.load(
            "res/levels/four.lvl", screen_info_.width, screen_info_.height / 2);

        levels_.push_back(one);
        levels_.push_back(two);
        levels_.push_back(three);
        levels_.push_back(four);

        // configure game objects
        // Player 1
        glm::vec2 player_pos{ glm::vec2{ screen_info_.width / 2.0f -
                                             player_size_.x / 2.0f,
                                         screen_info_.height - player_size_.y } };

        // player1_ = std::make_unique<gc::Player>(
        //     3, player_pos, player_size_,
        //     gc::ResourceManager::get_texture("paddle"));

        // glm::vec2 ball_pos{ player_pos +
        //                     glm::vec2{ player_size_.x / 2.0f - ball_radius_,
        //                                -ball_radius_ * 2.0f } };

        // // local_player2_ = std::make_unique<gc::Player>(
        // //     3, player_pos, player_size_,
        // //     gc::ResourceManager::get_texture("paddle"));

        // ball_ = std::make_shared<gc::BallObject>(
        //     ball_pos,
        //     ball_radius_,
        //     initial_ball_velocity_,
        //     gc::ResourceManager::get_texture("face"));

        glm::vec2 ball_pos{ player_pos +
                            glm::vec2{ player_size_.x / 2.0f - ball_radius_,
                                       -ball_radius_ * 2.0f } };

        ball_ = std::make_shared<gc::BallObject>(
            ball_pos,
            ball_radius_,
            initial_ball_velocity_,
            gc::ResourceManager::get_texture("face"));

        text_ = std::make_unique<gc::TextRender>(screen_info_.width,
                                                 screen_info_.height);
        text_->load("res/fonts/OCRAEXT.TTF", font_size_);

        // Main Menu theme
        result_ = ma_engine_init(nullptr, &engine_);
        if (result_ != MA_SUCCESS)
        {
            std::cerr << "Failed to initialize audio\n";
            return false;
        }
        ma_engine_play_sound(&engine_, "res/audio/breakout.mp3", nullptr);

        // UI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsClassic();
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 130"); // OpenGL 3.x

        return true;
    }

    void run() override
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
            process_input(delta_time);

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

    void render() override
    {
        if (state_ == gc::GameState::GAME_MAIN_MENU)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            sprite_renderer_->draw_sprite(
                gc::ResourceManager::get_texture("background"),
                glm::vec2(0.0f, 0.0f),
                glm::vec2(screen_info_.width, screen_info_.height),
                0.0f);

            ImGui::Begin("Connect to server");
            ImGui::Text("Enter server IP");
            ImGui::InputTextWithHint("##ServerIpInput",
                                     "e.g. 127.0.0.1",
                                     client_.ip_to_connect().data(),
                                     client_.ip_to_connect().size());

            if (ImGui::Button("Connect"))
            {
                // Connect to server
                if (client_.connect(client_.ip_to_connect().data(), 50000))
                {
                    state_ = gc::GameState::WAITING_TO_CONNECT;
                }
                else
                {
                    std::cerr << "Failed to connect to server" << std::endl;
                    return;
                }
            }
            ImGui::End();

            // Alert Server is full
            if (show_server_full_popup_)
            {
                ImGui::OpenPopup("ServerIsFullPopup");
            }
            if (ImGui::BeginPopupModal(
                    "ServerIsFullPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Server is full");
                if (ImGui::Button("OK"))
                {
                    ImGui::CloseCurrentPopup();
                    show_server_full_popup_ = false;
                }
                ImGui::EndPopup();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            return;
        }

        if (state_ == gc::GameState::WAITING_TO_CONNECT)
        {
            sprite_renderer_->draw_sprite(
                gc::ResourceManager::get_texture("background"),
                glm::vec2(0.0f, 0.0f),
                glm::vec2(screen_info_.width, screen_info_.height),
                0.0f);
            levels_[current_level_].draw(*sprite_renderer_);

            text_->render_text(
                "WAITING TO CONNECT...", 100.0f, screen_info_.height / 2, 2.0f);
            return;
        }

        if (state_ == gc::GameState::GAME_ACTIVE ||
            state_ == gc::GameState::WAITING_FOR_OTHER_PLAYER ||
            state_ == gc::GameState::GAME_READY ||
            state_ == gc::GameState::GAME_ENDS)
        {
            effects_->begin_render();
            // Draw background
            sprite_renderer_->draw_sprite(
                gc::ResourceManager::get_texture("background"),
                glm::vec2(0.0f, 0.0f),
                glm::vec2(screen_info_.width, screen_info_.height),
                0.0f);

            // Draw level
            levels_[current_level_].draw(*sprite_renderer_);

            // Draw World Objs
            for (auto& pair : map_players_)
            {
                if (pair.second)
                {
                    pair.second->draw(*sprite_renderer_);
                }
            }
            // local_player_->draw(*sprite_renderer_);
            // for (auto& pair : map_players_)
            // {
            //     pair.second->draw(*sprite_renderer_);
            // }
            // for (auto& powerup : powerups_)
            // {
            //     if (!powerup.destroyed_)
            //     {
            //         powerup.draw(*sprite_renderer_);
            //     }
            // }

            // particles_->draw();
            if (draw_ball_)
            {
                ball_->draw(*sprite_renderer_);
            }

            effects_->end_render();
            effects_->render(glfwGetTime());

            // Show lives
            std::stringstream ss{};
            for (const auto& player_desc : map_players_)
            {
                ss << player_desc.second->get_desc().lives;
                if (player_desc.second->player_number_ == PlayerNumber::One)
                {
                    text_->render_text("Lives " + ss.str(),
                                       5.0f,
                                       screen_info_.height - font_size_,
                                       1.0f);
                }
                if (player_desc.second->player_number_ == PlayerNumber::Two)
                {
                    text_->render_text("Lives " + ss.str(), 5.0f, 5.0f, 1.0f);
                }
                ss.str("");
                ss.clear();
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // In-game menu
            if (show_game_active_menu_popup)
            {
                ImGui::OpenPopup("GameActivePopup");
            }
            if (ImGui::BeginPopupModal(
                    "GameActivePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                    show_game_active_menu_popup = false;
                }

                if (ImGui::Button("Quit Game"))
                {
                    client_.disconnect();
                    map_players_.clear();
                    show_game_active_menu_popup = false;
                    state_                      = gc::GameState::GAME_MAIN_MENU;
                }
                ImGui::EndPopup();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        if (state_ == gc::GameState::GAME_READY)
        {
            text_->render_text(
                "Press ENTER to get ready", 250.0f, screen_info_.height / 2, 1.0f);
            // text_->render_text("Press W or S to select level",
            //                    245.0f,
            //                    screen_info_.height / 2 + 20.0f,
            //                    0.75f);
        }

        if (state_ == gc::GameState::WAITING_FOR_OTHER_PLAYER)
        {
            text_->render_text(
                "Waiting for other player", 250.0f, screen_info_.height / 2, 1.0f);
        }

        if (state_ == gc::GameState::GAME_ENDS)
        {
            std::string endgame_msg{};
            if (won_)
            {
                endgame_msg = "YOU WON";
            }
            else
            {
                endgame_msg = "YOU LOST";
            }

            text_->render_text(endgame_msg,
                               320.0,
                               screen_info_.height / 2 - 20.0,
                               1.0,
                               glm::vec3(0.0, 1.0, 0.0));
            text_->render_text("Press ENTER to return to main menu",
                               130.0,
                               screen_info_.height / 2,
                               1.0,
                               glm::vec3(1.0, 1.0, 0.0));
        }
    }

    void process_input(float dt)
    {
        // Control of Player object
        if (state_ == gc::GameState::GAME_READY)
        {
            if (keys_[GLFW_KEY_ENTER] && !keys_processed_[GLFW_KEY_ENTER] &&
                map_players_.contains(local_player_id_))
            {
                map_players_[local_player_id_]->is_ready = true;
                net::Message<GameMsgTypes> msg_is_ready{};
                msg_is_ready.header.id = GameMsgTypes::GamePlayerReady;
                client_.send(msg_is_ready);

                state_ = gc::GameState::WAITING_FOR_OTHER_PLAYER;

                keys_processed_[GLFW_KEY_ENTER] = true;
            }
            // if (keys_[GLFW_KEY_W] && !keys_processed_[GLFW_KEY_W])
            // {
            //     current_level_              = (current_level_ + 1) % 4;
            //     keys_processed_[GLFW_KEY_W] = true;
            // }
            // if (keys_[GLFW_KEY_S] && !keys_processed_[GLFW_KEY_S])
            // {
            //     if (current_level_ > 0)
            //     {
            //         --current_level_;
            //     }
            //     else
            //     {
            //         current_level_ = 3;
            //     }
            //     keys_processed_[GLFW_KEY_S] = true;
            // }
        }

        if (state_ == gc::GameState::GAME_ENDS)
        {
            // if (keys_[GLFW_KEY_ENTER])
            // {
            //     keys_processed_[GLFW_KEY_ENTER] = true;
            //     effects_->chaos_                = false;
            //     state_                          = gc::GameState::GAME_READY;
            // }

            if (keys_[GLFW_KEY_ENTER])
            {
                keys_processed_[GLFW_KEY_ENTER] = true;
                state_                          = gc::GameState::GAME_MAIN_MENU;
                net::Message<GameMsgTypes> msg_disconnect{};
                msg_disconnect.header.id = GameMsgTypes::ClientUnregisterWithServer;
                client_.send(msg_disconnect);
                client_.disconnect();
                map_players_.clear();
            }
        }

        if (state_ == gc::GameState::GAME_ACTIVE &&
            map_players_.contains(local_player_id_))
        {
            float velocity{ player_velocity_ * dt };
            // move player1_'s paddle
            if (keys_[GLFW_KEY_A])
            {
                if (map_players_[local_player_id_]->pos_.x >= 0.0f)
                {
                    map_players_[local_player_id_]->pos_.x -= velocity;
                    // if (ball_->stuck_)
                    // {
                    //     ball_->pos_.x -= velocity;
                    // }
                }
            }
            if (keys_[GLFW_KEY_D])
            {
                if (map_players_[local_player_id_]->pos_.x <=
                    screen_info_.width - map_players_[local_player_id_]->size_.x)
                {
                    map_players_[local_player_id_]->pos_.x += velocity;
                    // if (ball_->stuck_)
                    // {
                    //     ball_->pos_.x += velocity;
                    // }
                }
            }
            if (keys_[GLFW_KEY_SPACE] &&
                map_players_[local_player_id_]->player_number_ == PlayerNumber::One)
            {
                net::Message<GameMsgTypes> msg_launch_ball{};
                msg_launch_ball.header.id = GameMsgTypes::GamePlayerLaunchBall;
                client_.send(msg_launch_ball);
                // ball_->stuck_ = false;
            }

            if (keys_[GLFW_KEY_ESCAPE])
            {
                show_game_active_menu_popup = true;
            }
        }
    }

    void do_collisions() override
    {
        for (gc::GameObject& box : levels_[current_level_].bricks)
        {
            if (!box.destroyed_)
            {
                gc::Collision collision{ check_collision(*ball_, box) };
                if (std::get<0>(collision)) // if collision is true
                {
                    // destroy block if not solid
                    if (!box.is_solid_)
                    {
                        box.destroyed_ = true;
                        spawn_powerups(box);
                        ma_engine_play_sound(
                            &engine_, "res/audio/bleep.mp3", nullptr);
                    }
                    else
                    { // if block is solid, enable shake effect
                        shake_time_      = 0.05f;
                        effects_->shake_ = true;
                        ma_engine_play_sound(
                            &engine_, "res/audio/solid.mp3", nullptr);
                    }

                    // collision resolution
                    gc::Direction direction{ std::get<1>(collision) };
                    glm::vec2 diff_vector{ std::get<2>(collision) };
                    if (!(ball_->passthrough_ &&
                          !box.is_solid_)) // don't do collision resolution on
                                           // non-solid bricks if pass-through is
                                           // activated
                    {
                        if (direction == gc::Direction::LEFT ||
                            direction ==
                                gc::Direction::RIGHT) // horizontal collision
                        {
                            ball_->velocity_.x =
                                -ball_->velocity_.x;  // reverse horizontal velocity
                            // relocate
                            float penetration{ ball_->radius_ -
                                               std::abs(diff_vector.x) };
                            if (direction == gc::Direction::LEFT)
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
                            if (direction == gc::Direction::UP)
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
        for (gc::PowerUp& powerup : powerups_)
        {
            if (!powerup.destroyed_)
            {
                // first check if powerup passed bottom edge, if so: keep as inactive
                // and destroy
                if (powerup.pos_.y >= screen_info_.height)
                {
                    powerup.destroyed_ = true;
                }
                // if (check_collision(*local_player_, powerup))
                // {
                //     // collided with player1_, now active powerup
                //     active_powerup(powerup);
                //     powerup.destroyed_ = true;
                //     powerup.activated_ = true;
                //     ma_engine_play_sound(&engine_, "res/audio/powerup.wav",
                //     nullptr);
                // }
            }
        }

        // and finally check collisions for player1_ pad (unless stuck)
        // gc::Collision result{ check_collision(*ball_, *local_player_) };
        // if (!ball_->stuck_ && std::get<0>(result))
        // {
        //     // check where it hit the board, and change velocity based on where it
        //     // hit the board
        //     float center_board{ local_player_->pos_.x +
        //                         local_player_->size_.x / 2.0f };
        //     float distance{ ball_->pos_.x + ball_->radius_ - center_board };
        //     float percentage{ distance / (local_player_->size_.x / 2.0f) };

        //     // then move accordingly
        //     float strength{ 2.0f };
        //     glm::vec2 old_velocity{ ball_->velocity_ };
        //     ball_->velocity_.x = initial_ball_velocity_.x * percentage * strength;
        //     // ball_->velocity_.y = -ball_->velocity_.y;
        //     ball_->velocity_.y =
        //         -1.0f * abs(ball_->velocity_.y); // avoid sticky paddle issue
        //     ball_->velocity_ =
        //         glm::normalize(ball_->velocity_) * glm::length(old_velocity);

        //     ball_->stuck_ = ball_->sticky_;

        //     ma_engine_play_sound(&engine_, "res/audio/bleep.wav", nullptr);
        // }
    }

    bool update(float dt) override
    {
        // Check for incoming network messages
        if (client_.is_connected())
        {
            while (!client_.incoming().empty())
            {
                auto msg{ client_.incoming().pop_front().msg };

                switch (msg.header.id)
                {
                case GameMsgTypes::ClientAccepted:
                {
                    std::cout << "Server accepted client\n";
                    map_players_.clear();

                    net::Message<GameMsgTypes> sending_msg{};
                    sending_msg.header.id = GameMsgTypes::ClientRegisterWithServer;

                    // local_player_ = std::make_shared<gc::Player>(
                    //     3,
                    //     glm::vec2{ 100.0f, 100.0f },
                    //     player_size_,
                    //     gc::ResourceManager::get_texture("paddle"));
                    // sending_msg << local_player_->get_desc();

                    PlayerDesc local_player_desc{ .pos{ 100.0f, 100.0f },
                                                  .screen_info{ 800, 600 } };
                    sending_msg << local_player_desc;
                    client_.send(sending_msg);

                    break;
                }
                case GameMsgTypes::ClientAssignId:
                {
                    // Server is assigning us our Id
                    msg >> local_player_id_;
                    std::cout << "Assigned client Id = " << local_player_id_ << "\n";
                    break;
                }
                case GameMsgTypes::ServerIsFull:
                {
                    client_.disconnect();
                    map_players_.clear();
                    show_server_full_popup_ = true;
                    state_                  = gc::GameState::GAME_MAIN_MENU;
                    break;
                }
                case GameMsgTypes::GameAddPlayer:
                {
                    PlayerDesc player_desc{};
                    msg >> player_desc;
                    if (!map_players_.contains(player_desc.unique_id))
                    {
                        auto player{
                            std::make_shared<gc::Player>(
                                3,
                                glm::vec2{ 0.0f, 0.0f },
                                player_size_,
                                gc::ResourceManager::get_texture("paddle"),
                                screen_info_),
                        };
                        player->set_props(player_desc);
                        map_players_.insert_or_assign(player->unique_id_, player);
                    }

                    if (player_desc.unique_id == local_player_id_)
                    {
                        // Now we exist in game world
                        state_ = gc::GameState::GAME_READY;
                    }
                    break;
                }
                case GameMsgTypes::GameRemovePlayer:
                {
                    u32 removal_id{ 0 };
                    msg >> removal_id;
                    map_players_.erase(removal_id);
                    break;
                }
                case GameMsgTypes::GameActive:
                {
                    state_ = gc::GameState::GAME_ACTIVE;
                    break;
                }
                case GameMsgTypes::GameUpdatePlayer:
                {
                    PlayerDesc player_desc{};
                    msg >> player_desc;

                    auto player{ std::make_shared<gc::Player>(
                        3,
                        glm::vec2{ 0.0f, 0.0f },
                        player_size_,
                        gc::ResourceManager::get_texture("paddle"),
                        screen_info_) };
                    player->set_props(player_desc);

                    map_players_.insert_or_assign(player->unique_id_, player);
                    break;
                }
                case GameMsgTypes::GameReduceLives:
                {
                    PlayerDesc player_desc{};
                    msg >> player_desc;
                    auto it = map_players_.find(player_desc.unique_id);
                    if (it != map_players_.end())
                    {
                        it->second->lives_ = player_desc.lives;
                        if (local_player_id_ == player_desc.unique_id)
                        {
                            shake_time_      = 0.05f;
                            effects_->shake_ = true;
                            ma_engine_play_sound(
                                &engine_, "res/audio/solid.wav", nullptr);
                        }
                    }

                    break;
                }
                case GameMsgTypes::GameAddBall:
                {
                    draw_ball_ = true;
                    break;
                }
                case GameMsgTypes::GameUpdateBall:
                {
                    BallDesc ball_desc{};
                    msg >> ball_desc;
                    ball_->set_props(ball_desc);

                    break;
                }
                case GameMsgTypes::GamePlayPadSound:
                {
                    ma_engine_play_sound(&engine_, "res/audio/bleep.wav", nullptr);
                    break;
                }
                case GameMsgTypes::GameEnds:
                {
                    // This hacky check prevents the client from rendering the game
                    // over screen when reconnecting to a game that hasn't been reset
                    // on the server.
                    if (client_.is_connected())
                    {
                        PlayerNumber winner{};
                        msg >> winner;
                        if (map_players_.contains(local_player_id_) &&
                            winner == map_players_[local_player_id_]->player_number_)
                        {
                            won_ = true;
                        }
                        state_ = gc::GameState::GAME_ENDS;
                    }
                    break;
                }
                }
            }
        }

        // update objects locally
        // if (ball_)
        // {
        //     ball_->move(dt, screen_info_.width, screen_info_.height);
        // }

        // Check for collisions
        // do_collisions();

        // update particles_
        // particles_->update(dt, *ball_, 2, glm::vec2{ ball_->radius_ / 2.0f });

        // update_powerups(dt);

        // reduce shake time
        if (shake_time_ > 0.0f)
        {
            shake_time_ -= dt;
            if (shake_time_ <= 0.0f)
            {
                effects_->shake_ = false;
            }
        }

        // // reduce player 1 lives
        // if (ball_->pos_.y >= screen_info_.height - ball_->size_.y)
        // {
        //     --player1_->lives_;
        // }

        // // check win condition of player 1
        // if (state_ == gc::GameState::GAME_ACTIVE && player1_->lives_ == 0)
        // {
        //     reset_players();
        //     reset_level();
        //     effects_->chaos_ = true;
        //     state_           = gc::GameState::GAME_ENDS;
        //     winner_          = gc::Winner::Player1;
        // }

        // Send our player desc
        if (map_players_.contains(local_player_id_))
        {
            net::Message<GameMsgTypes> msg{};
            msg.header.id = GameMsgTypes::GameUpdatePlayer;
            msg << map_players_[local_player_id_]->get_desc();
            client_.send(msg);
        }

        return true;
    }

  private:
    std::unordered_map<u32, std::shared_ptr<gc::Player>> map_players_{};
    std::shared_ptr<gc::BallObject> ball_;
    Client client_{};
    u32 local_player_id_;
    gc::GameState state_{ gc::GameState::GAME_MAIN_MENU };

    bool draw_ball_{ false };
    bool show_server_full_popup_{ false };
    bool show_game_active_menu_popup{ false };
    bool won_{ false };
};