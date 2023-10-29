#include "HeightMap.h"

#include <SFML/Graphics/Image.hpp>
#include <cassert>
#include <imgui.h>
#include <iostream>

namespace
{
    float island(float t, int power)
    {
        return std::max(0.0, 1.0 - std::pow(t, power * 2));
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

float HeightMap::set_base_height()
{
    float base = 0;

    float min_diff = min_height() - base;

    for (auto& h : heights)
    {
        h -= min_diff;
    }

    return min_diff;
}

float HeightMap::min_height() const
{
    return min_height_;
}

float HeightMap::max_height() const
{
    return max_height_;
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
            float height =
                noise * options.amplitude - (options.amplitude / options.amplitude_dampen);

            float noise2 = noise_gen_.GetNoise(x * 0.06f, z * 0.06f);
            noise2 = (noise2 + 1.0f) / 2.0f;
            height += noise2 * options.amplitude / options.amplitude_dampen;

            // Make the terrain less noisy below water level if (height < waterLevel)

            float water_level = options.water_level;
            if (options.water_level_damper)
            {
                if (height < water_level)
                {
                    height += (water_level - height) / 1.25;
                }
                else
                {
                    float aboveWater = height - water_level;
                    float factor = 1.0f - aboveWater / (options.amplitude - water_level);
                    height += (water_level - height) * factor;
                }
            }

            if (options.generate_island)
            {
                float bump_x = (static_cast<float>(x) / static_cast<float>(size)) * 2.0f - 1.0f;
                float bump_z = (static_cast<float>(z) / static_cast<float>(size)) * 2.0f - 1.0f;
                float bump =
                    island(bump_x, options.bump_power) * island(bump_z, options.bump_power);
                height *= bump;
            }

            set_height(x, z, height);
        }
    }
    min_height_ = *std::min_element(heights.begin(), heights.end());
    max_height_ = *std::max_element(heights.begin(), heights.end());
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
        // clang-format off
        if (ImGui::SliderFloat  ("Frequency",        &frequency,         0.01f, 0.5f))      update = true;
        if (ImGui::SliderFloat  ("Amplitude",        &amplitude,         1.0f,  2000.0f))   update = true;
        if (ImGui::SliderFloat  ("Amplitude Factor", &amplitude_dampen,  1.0f,  64.0f))   update = true;
        if (ImGui::SliderFloat  ("Lacunarity",       &lacunarity,        0.01f, 2.5f))      update = true;
        if (ImGui::SliderInt    ("Octaves",          &octaves,           1,     10))        update = true;
        if (ImGui::SliderFloat  ("Water Level",      &water_level,       1,     500))       update = true;
        if (ImGui::SliderInt    ("Seed",             &seed,              1,     1000000))   update = true;

        if (ImGui::Checkbox     ("Generate Island",     &generate_island))      update = true;
        if (ImGui::Checkbox     ("Water Level Dampen",  &water_level_damper))   update = true;

        if (generate_island &&
            ImGui::SliderInt ("Island Factor", &bump_power, 0, 16)) update = true;

        // clang-format on
    }
    ImGui::End();
    return update;
}
