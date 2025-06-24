#pragma once 

#include "Common.h"
#include "GameObject.h"
#include "Texture.h"
#include "Shader.h"

namespace gc
{
// Represents a single particle and its state
struct Particle
{
    glm::vec2 pos{ 0.0f };
    glm::vec2 velocity{ 0.0f };
    glm::vec4 color{ 1.0f };
    float life{ 0.0f };
};

// ParticleGenerator acts as a container for rendering a large number of
// particles by repeatedly spawning and updating particles and killing
// them after a given amount of time.
class ParticleGenerator
{
  public:
    ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);

    // update all particles
    void update(float dt, GameObject& object, u32 new_particles,
                glm::vec2 offset = glm::vec2{ 0.0f, 0.0f });

    // render all particles
    void draw();

  private:
    // state
    std::vector<Particle> particles_;
    u32 amount_;
    // render state
    Shader shader_;
    Texture2D texture_;
    u32 VAO_;
    // initializes buffer and vertext attributes
    void init();
    // returns the 1st Particle index that's currently unused e.g. life <= 0.0f or 0
    // if no particle is currently inactive
    u32 first_unused_particle();
    // respawn particle
    void respawn_particle(Particle& particle, GameObject& object,
                          const glm::vec2& offset = glm::vec2{ 0.0f, 0.0f });
};

} // namespace gc