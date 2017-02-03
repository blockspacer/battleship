#include <cassert>
#include <iterator>
#include "message_window.hpp"


MessageWindow::MessageWindow(int starty, int startx, int height, int width)
    : NCursesWindow(starty, startx, height, width)
{
    assert(starty >= 0);
    assert(startx >= 0);
    assert(height >= 0);
    assert(width >= 3); // used for "..."
}

MessageWindow::~MessageWindow()
{
}


void MessageWindow::post_message(std::string message)
{
    if (message.size() > static_cast<size_t>(width_))
    {
        if (message.size() < 3)
        {
            message.replace(message.begin(), message.end(), message.size(), '.');

        }
        else
        {
            message.replace(std::prev(message.end(), 3), message.end(), "...");
        }
    }
    else
    {
        message.append(static_cast<size_t>(width_) - message.size(), ' ');
    }

    messages_.push_back(message);
    if (messages_.size() > static_cast<size_t>(height_))
    {
        messages_.pop_front();
    }
    update();
}

void MessageWindow::update()
{
    auto end = messages_.begin();
    std::advance(end, std::min(static_cast<size_t>(height_), messages_.size()));
    for (auto it = messages_.begin(); it != end; ++it)
    {
        mvwaddstr(window_, it - messages_.begin(), 0, it->c_str());
    }
    wrefresh(window_);
}
