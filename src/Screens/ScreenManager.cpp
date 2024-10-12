#include "ScreenManager.h"

void ScreenManager::push_screen(std::unique_ptr<Screen> screen)
{
    screen_stack_.push(std::move(screen));
}

bool ScreenManager::pop_screen()
{
    if (!empty())
    {
        screen_stack_.pop();
        return true;
    }
    return false;
}

Screen* ScreenManager::peek_screen()
{
    if (!empty())
    {
        return screen_stack_.top().get();
    }
    return nullptr;
}

bool ScreenManager::empty() const
{
    return screen_stack_.empty();
}
