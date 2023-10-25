#pragma once

#include <SFML/Window/Window.hpp>

#include "Settings.h"

class DebugRenderer;


namespace GUI
{

    bool init(sf::Window* window);

    void begin_frame();
    void end_frame();

    void shutdown();

    void event(const sf::Window& window, sf::Event& e);


    void debug_window(const glm::vec3& camera_position,
                      const glm::vec3& camera_rotation, Settings& settings);

    void debug_renderer_window(DebugRenderer& debug_renderer, Settings& setting);

    void text_vec3(const std::string& text, const glm::vec3& vect);

} // namespace GUI