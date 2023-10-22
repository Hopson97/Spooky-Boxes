#pragma once

#include <filesystem>
#include <vector>

struct TerrainGenerationOptions
{
    float roughness = 0.7;
    float smoothness = 350.0f;
    float amplitude = 80.0f;

    int octaves = 5;
    float offset = -45;

    int seed = 523523;
};

struct HeightMap
{
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