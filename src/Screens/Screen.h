#pragma once

#include <SFML/System/Clock.hpp>
#include <memory>

class ScreenManager;

class Screen
{
  public:
    Screen(ScreenManager* screen_manager)
        : p_screen_manager_(screen_manager)
    {
    }

    virtual void on_update(sf::Time dt) = 0;
    virtual void on_render() = 0;

  protected:
    ScreenManager* p_screen_manager_;
};
