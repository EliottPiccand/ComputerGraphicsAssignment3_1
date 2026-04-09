#include "Input.h"

#include <cassert>

constexpr const size_t MASK_OFFSET = sizeof(unsigned int) * 8 / 2;
constexpr const unsigned int MASK = (1 << MASK_OFFSET) - 1;

void Input::initialize(const Window &window)
{
    window_handle = window.handle;
    
    // disable mouse acceleration
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window.handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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

    // mouse motion
    lastMousePosition = mousePosition;
    mousePosition = fetchMousePosition();
    mouseDelta = lastMousePosition - mousePosition;

    // keys
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

glm::vec2 Input::fetchMousePosition()
{
    assert(window_handle != nullptr && "calling Input::fetchMousePosition before initializing");

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

glm::vec2 Input::getMousePosition() {
    return mousePosition;
}

glm::vec2 Input::getMouseDelta() {
    return mouseDelta;
}
