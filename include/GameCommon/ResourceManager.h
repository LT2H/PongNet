#pragma once

#include "Texture.h"
#include "Shader.h"

namespace gc
{
class ResourceManager
{
  public:
    // Resource storage
    static std::map<std::string_view, Shader> shaders;
    static std::map<std::string_view, Texture2D> textures;

    // loads (and generates) a shader program from file loading vertex, fragment (and
    // geometry) shader's source code. If gShaderFile is not "", it also loads a
    // geometry shader
    static Shader load_shader(std::string_view v_shader_file,
                              std::string_view f_shader_file,
                              std::string_view g_shader_file, std::string_view name);

    // retrieves a stored shader
    static Shader get_shader(std::string_view name);

    // loads (and generates) a texture from file
    static Texture2D load_texture(std::string_view file, bool alpha,
                                  std::string_view name);

    // retrieves a stored texture
    static Texture2D get_texture(std::string_view name);

    // properly de-allocates all loaded resources
    static void clear();

    static void print_all_textures();

  private:
    // private constructor, that is we do not want any actual resource manager
    // objects. Its members and functions should be publicly available (static).
    explicit ResourceManager() {}

    // loads and generates a shader from file
    static Shader load_shader_from_file(std::string_view v_shader_file,
                                        std::string_view f_shader_file,
                                        std::string_view g_shader_file = "");

    // loads a single texture from file
    static Texture2D load_texture_from_file(std::string_view file, bool alpha);
};

} // namespace gc