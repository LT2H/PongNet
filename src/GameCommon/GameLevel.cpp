#include "GameCommon/GameObject.h"
#include "GameCommon/ResourceManager.h"
#include <GameCommon/GameLevel.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void gc::GameLevel::load(std::string_view file, u32 level_width, u32 level_height)
{
    // Clear old data
    bricks.clear();

    // Load from file
    u32 tile_code{};
    std::string line{};
    std::ifstream fstream{ file.data() };
    std::vector<std::vector<u32>> tile_data{};

    if (fstream)
    {
        while (std::getline(fstream, line)) // read each line from level file
        {
            std::istringstream sstream{ line };
            std::vector<u32> row;
            while (sstream >> tile_code) // read each word separated by spaces
            {
                row.push_back(tile_code);
            }
            tile_data.push_back(row);
        }

        if (tile_data.size() > 0)
        {
            init(tile_data, level_width, level_height);
        }
    }
}

void gc::GameLevel::draw(SpriteRenderer& renderer)
{
    for (GameObject& brick : bricks)
    {
        if (!brick.destroyed())
        {
            brick.draw(renderer);
        }
    }
}

bool gc::GameLevel::is_completed()
{
    for (GameObject& brick : bricks)
    {
        if (!brick.is_solid() && !brick.destroyed())
        {
            return false;
        }
    }
    return true;
}

void gc::GameLevel::init(const std::vector<std::vector<u32>>& tile_data,
                         u32 level_width, u32 level_height)
{
    // Calculate dimensions
    std::size_t width{ tile_data[0].size() };
    std::size_t height{ tile_data.size() };
    float unit_width{ level_width / static_cast<float>(width) };
    float unit_height{ level_height / static_cast<float>(height) };

    // Init level tiles based on tile_data
    for (u32 y{ 0 }; y < height; ++y)
    {
        for (u32 x{ 0 }; x < width; ++x)
        {
            // Check block type from level data (2D level array)
            if (tile_data[y][x] == 1) // indestructible
            {
                glm::vec2 pos{ unit_width * x, unit_height * y };
                glm::vec2 size{ unit_width, unit_height };
                GameObject obj{ pos,
                                size,
                                ResourceManager::get_texture("indestructible_block"),
                                glm::vec3{ 0.8, 0.8f, 0.7f } };

                obj.set_is_solid(true);
                bricks.push_back(obj);
            }
            else if (tile_data[y][x] > 1) // destructible
            {
                glm::vec3 color{ glm::vec3{ 1.0f } };
                if (tile_data[y][x] == 2)
                {
                    color = glm::vec3{ 0.2f, 0.6f, 1.0f };
                }
                else if (tile_data[y][x] == 3)
                {
                    color = glm::vec3{ 0.0f, 0.7f, 0.0f };
                }
                else if (tile_data[y][y] == 4)
                {
                    color = glm::vec3{ 0.8f, 0.8f, 0.4f };
                }
                else if (tile_data[y][x] == 5)
                {
                    color = glm::vec3{ 1.0f, 0.5f, 0.0f };
                }

                glm::vec2 pos{ unit_width * x, unit_height * y };
                glm::vec2 size{ unit_width, unit_height };
                bricks.push_back(GameObject{
                    pos, size, ResourceManager::get_texture("block"), color });
            }
        }
    }
}
