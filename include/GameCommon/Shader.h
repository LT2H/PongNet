#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Common.h"

namespace gm
{
class Shader
{
  public:
    Shader() {}

    // Sets the current shader as active
    Shader& use();
    // compiles the shader from given source code
    void compile(std::string_view vertex_source, std::string_view fragment_source,
                 std::string_view geometry_source =
                     nullptr); // note: geometry source code is optional

    // utils
    void set_float(const char* name, float value, bool use_shader = false);
    void set_integer(const char* name, int value, bool use_shader = false);
    void set_vector2f(const char* name, float x, float y, bool use_shader = false);
    void set_vector2f(const char* name, const glm::vec2& value,
                      bool use_shader = false);
    void set_vector3f(const char* name, float x, float y, float z,
                      bool use_shader = false);
    void set_vector3f(const char* name, const glm::vec3& value,
                      bool use_shader = false);
    void set_vector4f(const char* name, float x, float y, float z, float w,
                      bool use_shader = false);
    void set_vector4f(const char* name, const glm::vec4& value,
                      bool use_shader = false);
    void set_matrix4(const char* name, const glm::mat4& matrix,
                     bool use_shader = false);

    unsigned int id{};

  private:
    // checks if compilation or linking failed and if so, print the error logs
    void check_compile_errors(unsigned int object, std::string& type);
};
} // namespace gm