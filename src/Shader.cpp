#include <GameCommon/Shader.h>

#include <GameCommon/Common.h>

gm::Shader& gm::Shader::use()
{
    glUseProgram(this->id);
    return *this;
}

void gm::Shader::compile(std::string_view vertex_source,
                         std::string_view fragment_source,
                         std::string_view geometry_source)
{
    unsigned int vertex_shader{};
    unsigned int fragment_shader{};
    unsigned int geo_shader{};

    // Vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source.data(), nullptr);
    glCompileShader(fragment_shader);
}