#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

namespace kst::core::application {

  struct WindowSize {
    int width;
    int height;
  };

  class Window {
  public:
    Window()                                   = default;
    ~Window()                                  = default;
    Window(const Window&)                      = delete;
    auto operator=(const Window&) -> Window&   = delete;
    Window(const Window&&)                     = delete;
    auto operator=(const Window&&) -> Window&& = delete;

    auto create(const std::string& title, int width, int height) -> bool;
    void destroy();

    void pollEvents();
    auto shouldClose() -> bool;

    auto getNativeWindow() const -> void* { return m_nativeWindow; }
    auto getWidth() const -> int { return m_size.width; }
    auto getHeight() const -> int { return m_size.height; }

    using ResizeCallback = std::function<void(int, int)>;
    void setResizeCallback(ResizeCallback callback);

    // Accessor for resize callback
    auto getResizeCallback() const -> const ResizeCallback& { return m_resizeCallback; }

  private:
    void* m_nativeWindow = nullptr;
    WindowSize m_size{.width = 0, .height = 0};
    bool m_vsync = false;

    std::string m_title;

    ResizeCallback m_resizeCallback;
    std::function<void(double, double)> m_mouseCallback;
  };
} // namespace kst::core::application
