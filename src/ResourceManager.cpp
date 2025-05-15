#include <GameCommon/ResourceManager.h>
#include <vector>

#include "GameCommon/Common.h"
#include "GameCommon/Shader.h"
#include "GameCommon/Texture.h"

std::map<std::string_view, gm::Shader> gm::ResourceManager::shaders{};
std::map<std::string_view, gm::Texture2D> gm::ResourceManager::textures{};

gm::Shader gm::ResourceManager::load_shader(std::string_view v_shader_file,
                                            std::string_view f_shader_file,
                                            std::string_view g_shader_file,
                                            std::string_view name)
{
    shaders[name] =
        load_shader_from_file(v_shader_file, f_shader_file, g_shader_file);
    return shaders[name];
}

gm::Shader gm::ResourceManager::get_shader(std::string_view name)
{
    return shaders[name];
}

gm::Texture2D gm::ResourceManager::load_texture(std::string_view file, bool alpha,
                                                std::string_view name)
{
    textures[name] = load_texture_from_file(file, alpha);
    return textures[name];
}

void gm::ResourceManager::clear()
{
    for (auto& shader : shaders)
    {
        glDeleteProgram(shader.second.id);
    }
    for (auto& texture : textures)
    {
        unsigned int id{ texture.second.id() };
        glDeleteTextures(1, &id);
    }
}

gm::Shader gm::ResourceManager::load_shader_from_file(std::string_view v_shader_file,
                                                      std::string_view f_shader_file,
                                                      std::string_view g_shader_file)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertex_code;
    std::string fragment_code;
    std::string geo_code;

    try
    {
        // open files
        std::ifstream vertex_shader_file{ v_shader_file.data() };
        std::ifstream fragment_shader_file{ f_shader_file.data() };

        std::stringstream v_shader_stream{};
        std::stringstream f_shader_stream{};

        // read file's buffer contents into streams
        v_shader_stream << vertex_shader_file.rdbuf();
        f_shader_stream << fragment_shader_file.rdbuf();

        // Convert stream into string
        vertex_code   = v_shader_stream.str();
        fragment_code = f_shader_stream.str();

        if (!g_shader_file.empty())
        {
            std::ifstream geo_shader_file{ g_shader_file.data() };
            std::stringstream g_shader_stream{};

            g_shader_stream << geo_shader_file.rdbuf();

            geo_code = g_shader_stream.str();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }

    // 2. now create shader object from source code
    Shader shader{};
    shader.compile(vertex_code, fragment_code, !geo_code.empty() ? geo_code : "");
    return shader;
}

gm::Texture2D gm::ResourceManager::load_texture_from_file(std::string_view file,
                                                          bool alpha)
{ // Create texture object
    Texture2D texture{};
    if (alpha)
    {
        texture.set_image_format(GL_RGBA);
        texture.set_internal_format(GL_RGBA);
    }

    // Load image
    int width{};
    int height{};
    int nr_channels{};

    unsigned char* data{ stbi_load(file.data(), &width, &height, &nr_channels, 0) };

    // now generate texture
    texture.generate(width, height, data);

    stbi_image_free(data);

    return texture;
}
