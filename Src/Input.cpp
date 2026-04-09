#include "Input.h"

#include <cassert>

constexpr const size_t MASK_OFFSET = sizeof(unsigned int) * 8 / 2;
constexpr const unsigned int MASK = (1 << MASK_OFFSET) - 1;

void Input::initialize(const Window &window)
{
    window_handle = window.handle;
}

void Input::bindKey(Action action, unsigned int key)
{
    key += 1;
    assert((key & MASK) == key && "`key` must be a `GLFW_KEY_...` macro");

    binds[action] = key;
    states[action] = State::HeldReleased;
}

void Input::bindMouseButton(Action action, unsigned int mouseButton)
{
    mouseButton += 1;
    assert((mouseButton & MASK) == mouseButton && "`mouseButton` must be a `GLFW_MOUSE_BUTTON_...` macro");

    binds[action] = mouseButton << MASK_OFFSET;
    states[action] = State::HeldReleased;
}

void Input::update()
{
    assert(window_handle != nullptr && "calling Input::update before initializing");

    for (const auto &[action, key] : binds)
    {
        const auto glfw_state = ((key & (MASK << MASK_OFFSET)) == 0)
                                    ? glfwGetKey(window_handle, key - 1)
                                    : glfwGetMouseButton(window_handle, (key >> MASK_OFFSET) - 1);

        if (glfw_state == GLFW_PRESS)
        {
            switch (states[action])
            {

            case State::JustPressed:
            case State::HeldPressed:
                states[action] = State::HeldPressed;
                break;
            case State::JustReleased:
            case State::HeldReleased:
                states[action] = State::JustPressed;
                break;
            }
        }
        else // glfw_state == GLFW_RELEASE
        {
            switch (states[action])
            {

            case State::JustPressed:
            case State::HeldPressed:
                states[action] = State::JustReleased;
                break;
            case State::JustReleased:
            case State::HeldReleased:
                states[action] = State::HeldReleased;
                break;
            }
        }
    }
}

Input::State Input::getState(Action action)
{
    return states.at(action);
}

bool Input::isPressed(Action action)
{
    return states.at(action) == State::JustPressed || states.at(action) == State::HeldPressed;
}

bool Input::isMouseInWindow()
{
    assert(window_handle != nullptr && "calling Input::isMouseInWindow before initializing");

    double xpos = 0.0;
    double ypos = 0.0;
    glfwGetCursorPos(window_handle, &xpos, &ypos);

    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window_handle, &windowWidth, &windowHeight);

    return 0.0 <= xpos && xpos < static_cast<double>(windowWidth) && 0.0 <= ypos &&
           ypos < static_cast<double>(windowHeight);
}

glm::vec2 Input::getMousePos()
{
    assert(window_handle != nullptr && "calling Input::getMousePos before initializing");

    double xpos, ypos;
    glfwGetCursorPos(window_handle, &xpos, &ypos);

    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowSize(window_handle, &windowWidth, &windowHeight);

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window_handle, &framebufferWidth, &framebufferHeight);

    const float scaleX =
        windowWidth > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth) : 1.0f;
    const float scaleY =
        windowHeight > 0 ? static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight) : 1.0f;

    return {static_cast<float>(xpos) * scaleX, static_cast<float>(ypos) * scaleY};
}
