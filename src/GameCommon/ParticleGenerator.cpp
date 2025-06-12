#include "GameCommon/Common.h"
#include "GameCommon/Shader.h"
#include <GameCommon/ParticleGenerator.h>

gc::ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture,
                                         u32 amount)
    : shader_{ shader }, texture_{ texture }, amount_{ amount }
{
    init();
}

void gc::ParticleGenerator::update(float dt, GameObject& object, u32 new_particles,
                                   glm::vec2 offset)
{
    // add new particles
    for (u32 i = 0; i < new_particles; ++i)
    {
        int unused_particle = this->first_unused_particle();
        this->respawn_particle(particles_[unused_particle], object, offset);
    }
    // update all particles
    for (u32 i = 0; i < amount_; ++i)
    {
        Particle& p{ particles_[i] };
        p.life -= dt; // reduce life
        if (p.life > 0.0f)
        {
            // particle is still alive, thus update
            p.pos -= p.velocity * dt;
            p.color.a -= dt * 2.5f;
        }
    }
}

// render all particles
void gc::ParticleGenerator::draw()
{
    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    shader_.use();
    for (auto& particle : particles_)
    {
        if (particle.life > 0.0f)
        {
            shader_.set_vector2f("offset", particle.pos);
            shader_.set_vector4f("color", particle.color);
            texture_.bind();
            glBindVertexArray(VAO_);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// render all particles
void gc::ParticleGenerator::init()
{
    // set up mesh and attribute properties
    u32 VBO;
    std::array particle_quad{ 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
                              1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

                              0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                              1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f };

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO_);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(particle_quad),
                 particle_quad.data(),
                 GL_STATIC_DRAW);
    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glBindVertexArray(0);

    // create this->amount default particle instances
    for (u32 i = 0; i < amount_; ++i)
        particles_.emplace_back(Particle{});
}

// stores the index of the last particle used (for quick access to next dead
// particle)
u32 last_used_particle = 0;

u32 gc::ParticleGenerator::first_unused_particle()
{
    // first search from last used particle, this will usually return almost
    // instantly
    for (u32 i{ last_used_particle }; i < amount_; ++i)
    {
        if (particles_[i].life <= 0.0f)
        {
            last_used_particle = i;
            return i;
        }
    }

    // otherwise, do a linear search
    for (u32 i{ 0 }; i < last_used_particle; ++i)
    {
        if (particles_[i].life <= 0.0f)
        {
            last_used_particle = i;
            return i;
        }
    }
    // all particles are taken, override the first one (note that if it repeatedly
    // hits this case, more particles should be reserved)
    last_used_particle = 0;
    return 0;
}

void gc::ParticleGenerator::respawn_particle(Particle& particle, GameObject& object,
                                             const glm::vec2& offset)
{
    float random{ ((std::rand() % 100) - 50) / 10.0f };
    float random_color{ 0.5f + ((std::rand() % 100) / 100.0f) };
    particle.pos      = object.pos_ + random + offset;
    particle.color    = glm::vec4(random_color, random_color, random_color, 1.0f);
    particle.life     = 1.0f;
    particle.velocity = object.velocity_ * 0.1f;
}
