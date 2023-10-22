#include "HeightMap.h"

#include <glm/gtc/noise.hpp>

namespace
{
    float get_height_at(const glm::ivec2& position,
                        const TerrainGenerationOptions& options)
    {
        float value = 0;
        float acc = 0;
        for (int i = 0; i < options.octaves; i++)
        {
            float frequency = glm::pow(2.0f, i);
            float amplitude = glm::pow(options.roughness, i);

            float x = position.x * frequency / options.smoothness;
            float z = position.y * frequency / options.smoothness;

            float noiseValue = glm::simplex(glm::vec3{x, z, options.seed});
            noiseValue = (noiseValue + 1.0f) / 2.0f;
            value += noiseValue * amplitude;
            acc += amplitude;
        }
        return value / acc * options.amplitude + options.offset;
    }
} // namespace

HeightMap::HeightMap(int size)
    : heights(size * size)
    , size(size)
{
    std::fill(heights.begin(), heights.end(), 0.0f);
}

float HeightMap::get_height(int x, int z) const
{
    return heights[z * size + x];
}

void HeightMap::set_height(int x, int z, float height)
{
    heights[z * size + x] = height;
}

void HeightMap::set_base_height()
{
    float base = 0;

    float min = *std::min_element(heights.begin(), heights.end());
    float min_diff = min - base;

    for (auto& h : heights)
    {
        h -= min_diff;
    }
}

void HeightMap::generate_terrain(const TerrainGenerationOptions& options)
{
    for (int z = 0; z < size; z++)
    {
        for (int x = 0; x < size; x++)
        {
            float height = get_height_at({x, z}, options);
            set_height(x, z, height);
        }
    }
}