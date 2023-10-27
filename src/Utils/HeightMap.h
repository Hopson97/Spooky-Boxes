#pragma once

#include <FastNoiseLite/FastNoiseLite.h>
#include <filesystem>
#include <vector>

struct TerrainGenerationOptions
{
    float frequency = 0.12f;
    float amplitude = 256.0f;
    float lacunarity = 2.0f;
    int octaves = 8;

    float offset = -45.0f;

    int seed = 523523;

    bool gui();
};

struct HeightMap
{
    FastNoiseLite noise_gen_;
    std::vector<float> heights;
    const int size;

    HeightMap(int size);

    float get_height(int x, int z) const;
    void set_height(int x, int z, float height);

    void set_base_height();

    float min_height() const;
    float max_height() const;

    void generate_terrain(const TerrainGenerationOptions& options);

    static HeightMap from_image(const std::filesystem::path& path);
};