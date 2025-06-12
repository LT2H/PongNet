#include <GameCommon/Common.h>
#include <GameCommon/Texture.h>
#include <GameCommon/PostProcessor.h>

gc::PostProcessor::PostProcessor(const Shader& shader, u32 width, u32 height)
    : post_processing_shader_{ shader }, texture_{}, width_{ width },
      height_{ height }, confuse_{ false }, chaos_{ false }, shake_{ false }
{
    // initialize renderbuffer/framebuffer object
    glGenFramebuffers(1, &MSFBO_);
    glGenFramebuffers(1, &FBO_);
    glGenRenderbuffers(1, &RBO_);
    // initialize renderbuffer storage with a multisampled color buffer (don't need a
    // depth/stencil buffer)
    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO_);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO_);
    glRenderbufferStorageMultisample(
        GL_RENDERBUFFER,
        4,
        GL_RGB,
        width,
        height); // allocare storage for render buffer object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER,
                              RBO_); // attach MS render buffer object to framebuffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::POSTPROCESSOR: Failed to initialize MSFBO\n";
    }
    // also initialize the FBO/texture to blit multisampled color-buffer to; used for
    // shader operations (for postprocessing effects)
    glBindFramebuffer(GL_FRAMEBUFFER, FBO_);
    texture_.generate(width_, height_, nullptr);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        texture_.id(),
        0); // attach texture to framebuffer as its color attachment
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::POSTPROCESSOR: Failed to initialize FBO\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // initialize render data and uniforms
    init_render_data();
    post_processing_shader_.set_integer("scene", 0, true);
    float offset{ 1.0f / 300.0f };
    std::array<std::array<float, 2>, 9> offsets = { {
        { { -offset, offset } },  // top-left
        { { 0.0f, offset } },     // top-center
        { { offset, offset } },   // top-right
        { { -offset, 0.0f } },    // center-left
        { { 0.0f, 0.0f } },       // center-center
        { { offset, 0.0f } },     // center-right
        { { -offset, -offset } }, // bottom-left
        { { 0.0f, -offset } },    // bottom-center
        { { offset, -offset } }   // bottom-right
    } };
    glUniform2fv(glGetUniformLocation(post_processing_shader_.id, "offsets"),
                 9,
                 reinterpret_cast<float*>(offsets.data()));

    std::array edge_kernel{ -1, -1, -1, -1, 8, -1, -1, -1, -1 };
    glUniform1iv(glGetUniformLocation(post_processing_shader_.id, "edge_kernel"),
                 9,
                 edge_kernel.data());

    std::array blur_kernel{ 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
                            2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
                            1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f };
    glUniform1fv(
        glGetUniformLocation(this->post_processing_shader_.id, "blur_kernel"),
        9,
        blur_kernel.data());
}
void gc::PostProcessor::begin_render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void gc::PostProcessor::end_render()
{
    // now resolve multisampled color-buffer into intermediate FBO to store to
    // texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, MSFBO_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO_);
    glBlitFramebuffer(0,
                      0,
                      width_,
                      height_,
                      0,
                      0,
                      width_,
                      height_,
                      GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // binds both READ and WRITE framebuffer to
                                          // default framebuffer
}

void gc::PostProcessor::render(float time)
{
    // set uniforms/options
    post_processing_shader_.use();
    post_processing_shader_.set_float("time", time);
    post_processing_shader_.set_integer("confuse", confuse_);
    post_processing_shader_.set_integer("chaos", chaos_);
    post_processing_shader_.set_integer("shake", shake_);
    // render texture quad
    glActiveTexture(GL_TEXTURE0);
    texture_.bind();
    glBindVertexArray(VAO_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void gc::PostProcessor::init_render_data()
{
    // configure VAO/VBO
    u32 VBO{};
    // pos and tex
    std::array vertices{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 1.0f,
                         1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,

                         -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, -1.0f,
                         1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f };
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    glBindVertexArray(VAO_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}