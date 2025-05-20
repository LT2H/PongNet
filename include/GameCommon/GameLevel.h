#include "Common.h"
#include "GameObject.h"

namespace gc
{
class GameLevel
{
  public:
    // Level state
    std::vector<GameObject> bricks;

    // Constructor
    explicit GameLevel() {}

    // Load level from file
    void load(std::string_view file, u32 level_width, u32 level_height);

    // Check if the level is completed (all non-solid tiles are destroyed)
    bool is_completed();

    void draw(SpriteRenderer& renderer);

  private:
    // Initialize level from tile data
    void init(const std::vector<std::vector<u32>>& tile_data, u32 level_width,
              u32 level_height);
};
} // namespace gc