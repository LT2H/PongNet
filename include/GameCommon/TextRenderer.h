#pragma once
#include "Common.h"
#include "GameCommon/Shader.h"

namespace gcom
{
// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
    u32 texture_id;     // ID handle of the glyph texture
    glm::ivec2 size;    // size of glyph
    glm::ivec2 bearing; // offset from baseline to left/top of glyph
    u32 advance;        // horizontal offset to advance to next glyph
};


// A renderer class for rendering text displayed by a font loaded using the
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRender
{
  public:
    TextRender(u32 width, u32 height);

    // pre-compiles a list of characters from the given font
    void load(std::string_view font, u32 font_size);

    // renders a string of text using the precompiled list of characters
    void render_text(const std::string text, float x, float y, float scale,
                     const glm::vec3 color = glm::vec3(1.0f));

    // holds a list of pre-compiled Characters
    std::map<char, Character> characters_;
    // shader used for text rendering
    Shader shader_;

  private:
    // render state
    u32 VAO_;
    u32 VBO_;
};
} // namespace gcom
