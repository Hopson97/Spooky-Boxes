#include "HeightMap.h"

#include <SFML/Graphics/Image.hpp>
#include <cassert>
#include <imgui.h>

namespace
{
    float island(float t)
    {
        return std::max(0.0, 1.0 - std::pow(t, 6.0));
    }

} // namespace

HeightMap::HeightMap(int size)
    : heights(size * size)
    , size(size)
{
    std::fill(heights.begin(), heights.end(), 0.0f);
    noise_gen_.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise_gen_.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);
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

    float min_diff = min_height() - base;

    for (auto& h : heights)
    {
        h -= min_diff;
    }
}

float HeightMap::min_height() const
{
    return *std::min_element(heights.begin(), heights.end());
}

float HeightMap::max_height() const
{
    return *std::max_element(heights.begin(), heights.end());
}

void HeightMap::generate_terrain(const TerrainGenerationOptions& options)
{
    noise_gen_.SetFrequency(options.frequency);
    noise_gen_.SetFractalOctaves(options.octaves);
    noise_gen_.SetFractalLacunarity(options.lacunarity);
    noise_gen_.SetSeed(options.seed);

    for (int z = 0; z < size; z++)
    {
        for (int x = 0; x < size; x++)
        {
            float noise = noise_gen_.GetNoise(x * 0.01f, z * 0.01f);
            noise = (noise + 1.0f) / 2.0f;
            float height = noise * options.amplitude - (options.amplitude / 8.0f);

            float noise2 = noise_gen_.GetNoise(x * 0.06f, z * 0.06f);
            noise2 = (noise2 + 1.0f) / 2.0f;
            height += noise2 * options.amplitude / 8;

            float x0 = (x / size) * 2.0f - 1.0f;
            float z0 = (z / size) * 2.0f - 1.0f;

            set_height(x, z, height);
        }
    }
}

HeightMap HeightMap::from_image(const std::filesystem::path& path)
{
    sf::Image img;
    img.loadFromFile(path.string());

    assert(img.getSize().x == img.getSize().y);

    HeightMap height_map{static_cast<int>(img.getSize().x)};

    for (int z = 0; z < height_map.size; z++)
    {
        for (int x = 0; x < height_map.size; x++)
        {
            auto height = static_cast<float>(img.getPixel(x, z).r);
            height_map.set_height(x, z, height);
        }
    }
    return height_map;
}

bool TerrainGenerationOptions::gui()
{
    bool update = false;
    if (ImGui::Begin("Height Generation"))
    {
        if (ImGui::SliderFloat("Frequency", &frequency, 0.01f, 0.5f))
        {
            update = true;
        }
        if (ImGui::SliderFloat("Amplitude", &amplitude, 1.0f, 2000.0f))
        {
            update = true;
        }

        if (ImGui::SliderFloat("Lacunarity", &lacunarity, 0.01f, 2.5f))
        {
            update = true;
        }

        if (ImGui::SliderInt("Octaves", &octaves, 1, 10))
        {
            update = true;
        }
    }
    ImGui::End();
    return update;
}
