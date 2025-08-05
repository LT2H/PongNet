#include <GameCommon/Common.h>
#include <GameCommon/TextRenderer.h>
#include <GameCommon/ResourceManager.h>

gcom::TextRender::TextRender(u32 width, u32 height)
{
    // load and configure shader
    shader_ =
        ResourceManager::load_shader("res/shaders/text_2d.vert", "res/shaders/text_2d.frag", "", "text");

    shader_.set_matrix4(
        "projection",
        glm::ortho(
            0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f),
        true);

    shader_.set_integer("text", 0);
    // configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void gcom::TextRender::load(std::string_view font, u32 font_size)
{
    // first clear the previously loaded Characters
    characters_.clear();
    // then initialize and load the FreeType library
    FT_Library ft{};
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library\n";
    }

    // load font face
    FT_Face face{};
    if (FT_New_Face(ft, font.data(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font\n";
    }

    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, font_size);
    // disable byte-aligment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // then for the 1st 128 ASCII characters, pre-load/compile their characters and
    // store the
    for (GLubyte c{ 0 }; c < 128; ++c)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYTPE: Failed to load Glyph\n";
            continue;
        }

        // generate texture
        u32 texture{};
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character character{
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<u32>(face->glyph->advance.x)
        };

        characters_.insert(std::pair<char, Character>{ c, character });
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void gcom::TextRender::render_text(const std::string text, float x, float y,
                                 float scale, const glm::vec3 color)
{
    // active corresponding render state
    shader_.use();
    shader_.set_vector3f("text_color", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO_);

    // iterate throung all characters
    std::string::const_iterator cit;
    for (cit = text.begin(); cit != text.end(); ++cit)
    {
        Character ch = characters_[*cit];

        float xpos{ x + ch.bearing.x * scale };
        float ypos{ y + (characters_['H'].bearing.y - ch.bearing.y) * scale };

        float w{ ch.size.y * scale };
        float h{ ch.size.y * scale };
        // update VBO for each character
        std::array<std::array<float, 4>, 6> vertices = {
            { { { xpos, ypos + h, 0.0f, 1.0f } },
              { { xpos + w, ypos, 1.0f, 0.0f } },
              { { xpos, ypos, 0.0f, 0.0f } },
              { { xpos, ypos + h, 0.0f, 1.0f } },
              { { xpos + w, ypos + h, 1.0f, 1.0f } },
              { { xpos + w, ypos, 1.0f, 0.0f } } }
        };

        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            0,
            sizeof(vertices),
            vertices.data()); // be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for the next glyph
        x += (ch.advance >> 6) *
             scale; // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
