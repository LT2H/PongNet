#include <GameCommon/Shader.h>

#include <GameCommon/Common.h>
#include <iostream>

gc::Shader& gc::Shader::use()
{
    glUseProgram(id);
    return *this;
}

void gc::Shader::compile(std::string_view vertex_source,
                         std::string_view fragment_source,
                         std::string_view geometry_source)
{
    unsigned int vertex_shader{};
    unsigned int fragment_shader{};
    unsigned int geo_shader{};

    // Vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* v_source{ vertex_source.data() };
    glShaderSource(vertex_shader, 1, &v_source, nullptr);
    glCompileShader(vertex_shader);
    check_compile_errors(vertex_shader, "VERTEX");

    // Fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f_source{ fragment_source.data() };
    glShaderSource(fragment_shader, 1, &f_source, nullptr);
    glCompileShader(fragment_shader);
    check_compile_errors(fragment_shader, "FRAGMENT");

    // If geometry shader source code is given, also compile geo shader
    if (!geometry_source.empty())
    {
        geo_shader = glCreateShader(GL_GEOMETRY_SHADER);
        const char* g_source{ geometry_source.data() };
        glShaderSource(geo_shader, 1, &g_source, nullptr);
        glCompileShader(geo_shader);
        check_compile_errors(geo_shader, "GEOMETRY");
    }

    // Shader program
    id = glCreateProgram();
    glAttachShader(id, vertex_shader);
    glAttachShader(id, fragment_shader);
    if (!geometry_source.empty())
    {
        glAttachShader(id, geo_shader);
    }

    glLinkProgram(id);
    check_compile_errors(id, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer
    // necessary
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    if (!geometry_source.empty())
    {
        glDeleteShader(geo_shader);
    }
}

void gc::Shader::set_float(std::string_view name, float value, bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform1f(glGetUniformLocation(id, name.data()), value);
}

void gc::Shader::set_integer(std::string_view name, int value, bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform1f(glGetUniformLocation(id, name.data()), value);
}

void gc::Shader::set_vector2f(std::string_view name, float x, float y,
                              bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform2f(glGetUniformLocation(id, name.data()), x, y);
}

void gc::Shader::set_vector2f(std::string_view name, const glm::vec2& value,
                              bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform2f(glGetUniformLocation(id, name.data()), value.x, value.y);
}

void gc::Shader::set_vector3f(std::string_view name, float x, float y, float z,
                              bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform3f(glGetUniformLocation(id, name.data()), x, y, z);
}

void gc::Shader::set_vector3f(std::string_view name, const glm::vec3& value,
                              bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform3f(glGetUniformLocation(id, name.data()), value.x, value.y, value.z);
}

void gc::Shader::set_vector4f(std::string_view name, float x, float y, float z,
                              float w, bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform4f(glGetUniformLocation(id, name.data()), x, y, z, w);
}

void gc::Shader::set_vector4f(std::string_view name, const glm::vec4& value,
                              bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniform4f(
        glGetUniformLocation(id, name.data()), value.x, value.y, value.z, value.w);
}
void gc::Shader::set_matrix4(std::string_view name, const glm::mat4& matrix,
                             bool use_shader)
{
    if (use_shader)
    {
        use();
    }
    glUniformMatrix4fv(
        glGetUniformLocation(id, name.data()), 1, false, glm::value_ptr(matrix));
}

void gc::Shader::check_compile_errors(unsigned int object, std::string_view type)
{
    int success{};
    std::array<char, 1024> info_log{};
    if (type != "PROGRAM")
    {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(object, 1024, nullptr, info_log.data());
            std::cerr
                << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                << info_log.data()
                << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
    else
    {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(object, 1024, nullptr, info_log.data());
            std::cerr
                << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                << info_log.data()
                << "\n -- --------------------------------------------------- -- "
                << std::endl;
        }
    }
}
