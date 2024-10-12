#pragma once

#include <memory>
#include <stack>

#include "Screen.h"

class ScreenManager
{
  public:
    void push_screen(std::unique_ptr<Screen> screen);
    bool pop_screen();
    Screen* peek_screen();

    bool empty() const;

  private:
    std::stack<std::unique_ptr<Screen>> screen_stack_;
};