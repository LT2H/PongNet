#include <GameCommon/ResourceManager.h>
#include <GameCommon/Game.h>
#include <GameCommon/SpriteRenderer.h>

gc::SpriteRenderer* renderer;

gc::Game::Game(unsigned int width, unsigned int height)
    : state{ GameState::GAME_ACTIVE }, keys{}, width_{ width }, height_{ height }
{
}

gc::Game::~Game() { delete renderer; }

void gc::Game::init()
{
    // Load shaders
    gc::ResourceManager::load_shader(
        "res/shaders/shader.vert",
        "res/shaders/shader.frag",
        "",
        "sprite");

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
    gc::ResourceManager::load_texture(
        "res/textures/awesomeface.png", true, "face");
}

void gc::Game::process_input(float dt) {}

void gc::Game::update(float dt) {}

void gc::Game::render()
{
    renderer->draw_sprite(gc::ResourceManager::get_texture("face"),
                          glm::vec2(200.0f, 200.0f),
                          glm::vec2(300.0f, 400.0f),
                          45.0f,
                          glm::vec3(0.0f, 1.0f, 0.0f));
}
