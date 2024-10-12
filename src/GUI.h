#pragma once

#include <unordered_map>

#include <SFML/Window/Window.hpp>
#include <imgui.h>

#include "Settings.h"

namespace GUI
{

    bool init(sf::Window* window);

    void begin_frame();
    void end_frame();

    void shutdown();

    void event(const sf::Window& window, sf::Event& e);

    void debug_window(const glm::vec3& camera_position, const glm::vec3& camera_rotation,
                      Settings& settings);
    void text_vec3(const std::string& text, const glm::vec3& vect);

    template <typename Value, typename OnSelect>
    void radio_button_group(int* selection,
                            const std::unordered_map<std::string, Value>& button_map,
                            OnSelect on_select, int per_line = 2)
    {
        // clang-format off
        int i = 0;
        for (auto& [name, value] : button_map)
        {
            if (ImGui::RadioButton(name.c_str(), selection, value))  on_select(value);
            if (i++ % per_line == 0)  ImGui::SameLine();
        }
        // clang-format on
    }

} // namespace GUI