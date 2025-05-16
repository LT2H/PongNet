#include "Shader.h"
#include "Common.h"
#include "Texture.h"
#include "glm/fwd.hpp"

namespace gc
{
class SpriteRenderer
{
  public:
    SpriteRenderer(const Shader& shader);
    ~SpriteRenderer();

    void draw_sprite(const Texture2D& texture, const glm::vec2& position,
                     const glm::vec2& size  = glm::vec2{ 10.0f, 10.0f },
                     float rotate           = 0.0f,
                     const glm::vec3& color = glm::vec3{ 1.0f });

  private:
    Shader shader_;
    unsigned int quad_VAO_;

    void init_render_data();
};
} // namespace gc