#pragma once

#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

namespace fs = std::filesystem;

std::string read_file_to_string(const std::filesystem::path& file_path);

template <typename N, typename T>
sf::Vector2<N> cast_vector(const sf::Vector2<T>& vec)
{
    return sf::Vector2<N>{static_cast<N>(vec.x), static_cast<N>(vec.y)};
}

inline auto to_btvec3(const glm::vec3& vec)
{
    return btVector3{vec.x, vec.y, vec.z};
}

struct FPSCounter final
{
    sf::Clock timer;
    float frameTime = 0;
    float frameCount = 0;

    void update()
    {
        frameCount++;
        if (timer.getElapsedTime() > sf::seconds(0.25))
        {
            auto time = timer.getElapsedTime();
            frameTime = time.asMilliseconds() / frameCount;
            timer.restart();
            frameCount = 0;
        }
    }
};
