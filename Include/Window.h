#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Input;

class Window
{
  private:
    friend Input;

    GLFWwindow *handle;
 
    int nonFullscreenPositionX;
    int nonFullscreenPositionY;
    int nonFullscreenWidth;
    int nonFullscreenHeight;
    bool isFullScreen;

  public:
    static constexpr const char *DEFAULT_TITLE = "Computer Graphics Assignment #3";
    static constexpr const uint16_t DEFAULT_WIDTH = 1280;
    static constexpr const uint16_t DEFAULT_HEIGHT = 720;

    Window();
    ~Window();

    [[nodiscard]] bool shouldClose() const;
    void endFrame() const;

    void setTitle(std::string title) const;
    void toggleFullscreen();

    [[nodiscard]] std::pair<uint32_t, uint32_t> getFramebufferSize() const;

    void close();
};
