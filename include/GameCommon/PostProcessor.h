#pragma once 

#include "Common.h"

#include "Texture.h"
#include "SpriteRenderer.h"
#include "Shader.h"

// PostProcessor hosts all PostProcessing effects for the Breakout
// Game. It renders the game on a textured quad after which one can
// enable specific effects by enabling either the Confuse, Chaos or
// Shake boolean.
// It is required to call BeginRender() before rendering the game
// and EndRender() after rendering the game for the class to work.
namespace gcom
{
class PostProcessor
{
  public:
    PostProcessor(const Shader& shader, u32 width, u32 height);

    // prepares the postprocessor/s framebuffer operations before rendering the game
    void begin_render();

    // should be called after rendering the game, so it stores all the rendered data
    // into a texture object
    void end_render();

    // renders the PostProcessor texture quad (as a screen-encompassing large sprite)
    void render(float time);

    // state
    Shader post_processing_shader_;
    Texture2D texture_;
    u32 width_;
    u32 height_;
    // options
    bool shake_;
    bool confuse_;
    bool chaos_;

  private:
    // render state
    u32 MSFBO_, FBO_; // MSFBP = Multisampled FBO. FBO is regular, used for blitting MS
                    // color-buffer to texture
    u32 RBO_;        // RBO is used for multisampled color buffer
    u32 VAO_;
    //initialize quad for rendering postprocessing texture
    void init_render_data();
};

} // namespace gcom