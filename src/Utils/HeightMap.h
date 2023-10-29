#pragma once

#include <FastNoiseLite/FastNoiseLite.h>
#include <filesystem>
#include <vector>

struct TerrainGenerationOptions
{
    float frequency = 0.15f;
    float amplitude = 315.0f;
    float lacunarity = 1.9f;
    int octaves = 6;
    float amplitude_dampen = 8.0f;

    float water_level = 64.0f;

    int seed = 523523;

    bool water_level_damper = true;
    bool generate_island = true;

    int bump_power = 3;

    bool gui();
};

struct HeightMap
{
    std::vector<float> heights;
    const int size;

    HeightMap(int size);

    float get_height(int x, int z) const;
    void set_height(int x, int z, float height);

    float set_base_height();

    float min_height() const;
    float max_height() const;

    void generate_terrain(const TerrainGenerationOptions& options);

    static HeightMap from_image(const std::filesystem::path& path);

  private:
    FastNoiseLite noise_gen_;
    float min_height_ = 0.0f;
    float max_height_ = 0.0f;
};