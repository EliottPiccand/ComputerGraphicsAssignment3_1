#pragma once

#include <unordered_map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Window.h"

class Input
{
  public:
    enum class Action
    {
        SpeedUp,
        SpeedDown,
        TurnLeft,
        TurnRight,
        Fire,
        CancelFire,
        ToggleFullScreen,
        UIClick,
    };

    enum class State
    {
        JustPressed,
        HeldPressed,
        JustReleased,
        HeldReleased,
    };

  private:
    static inline GLFWwindow *window_handle = nullptr;
    static inline std::unordered_map<Action, unsigned int> binds;
    static inline std::unordered_map<Action, State> states;

  public:
    static void initialize(const Window &window);

    static void bindKey(Action action, unsigned int key);
    static void bindMouseButton(Action action, unsigned int mouseButton);
    static void update();

    [[nodiscard]] static State getState(Action action);
    [[nodiscard]] static bool isPressed(Action action);
    [[nodiscard]] static bool isMouseInWindow();
    [[nodiscard]] static glm::vec2 getMousePos();
};
