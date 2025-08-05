#include "GameCommon/Shader.h"
#include <GameCommon/SpriteRenderer.h>

#include <GameCommon/Common.h>

gcom::SpriteRenderer::SpriteRenderer(const Shader& shader) : shader_{ shader }
{
    init_render_data();
}

gcom::SpriteRenderer::~SpriteRenderer() {
    glDeleteVertexArrays(1, &quad_VAO_);
}

void gcom::SpriteRenderer::draw_sprite(const Texture2D& texture, const glm::vec2& position,
                                     const glm::vec2& size, float rotate,
                                     const glm::vec3& color)
{
    // Prepare transformations
    shader_.use();
    glm::mat4 model{ glm::mat4( 1.0f ) };
    model = glm::translate(model, glm::vec3{ position, 0.0f });  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

    model = glm::translate(model, glm::vec3{ 0.5f * size.x, 0.5f * size.y, 0.0f }); // move origin of rotation to center of quad
    model = glm::rotate(model, glm::radians(rotate), glm::vec3{ 0.0f, 0.0f, 1.0f }); // then rotate
    model = glm::translate(model, glm::vec3{ -0.5f * size.x, -0.5f * size.y, 0.0f }); // move origin back

    model = glm::scale(model, glm::vec3{ size, 1.0f });

    shader_.set_matrix4("model", model);
    shader_.set_vector3f("sprite_color", color);

    glActiveTexture(GL_TEXTURE0);
    texture.bind();

    glBindVertexArray(quad_VAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void gcom::SpriteRenderer::init_render_data()
{
    // configure VAO/VBO
    u32 VBO{};
    constexpr std::array vertices{
        // pos              // tex
        0.0f, 1.0f, 0.0f, 1.0f, // bottom-left
        1.0f, 0.0f, 1.0f, 0.0f, // top-right
        0.0f, 0.0f, 0.0f, 0.0f, // top-left
    
        0.0f, 1.0f, 0.0f, 1.0f, // bottom-left
        1.0f, 1.0f, 1.0f, 1.0f, // bottom-right
        1.0f, 0.0f, 1.0f, 0.0f  // top-right
    };
    // Create the VAO and VBO
    glGenVertexArrays(1, &quad_VAO_);
    glGenBuffers(1, &VBO);

    // Upload vertex data to the VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    // Tell OpenGL how to interpret the vertex data (via VAO)
    glBindVertexArray(quad_VAO_);
    glEnableVertexAttribArray(0);

    // Describe what 1 vertex looks like
    glVertexAttribPointer(
        0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}