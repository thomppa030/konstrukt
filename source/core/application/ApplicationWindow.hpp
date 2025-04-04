#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace kst::core::application {

  struct WindowSize {
    int width;
    int height;
  };

  class Window {
  public:
    Window() {
      // Default constructor
    }

    ~Window();
    Window(const Window&)                      = delete;
    auto operator=(const Window&) -> Window&   = delete;
    Window(const Window&&)                     = delete;
    auto operator=(const Window&&) -> Window&& = delete;

    auto create(
        const std::string& title,
        int width,
        int height,
        bool fullscreen = false,
        bool resizable  = true
    ) -> bool;
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

    // Change window properties at runtime
    void setWindowTitle(const std::string& title);
    void setWindowSize(int width, int height);
    void setVSync(bool enabled);
    void setFullscreen(bool enabled);

  private:
    void* m_nativeWindow = nullptr;
    WindowSize m_size{.width = 0, .height = 0};
    bool m_vsync      = false;
    bool m_fullscreen = false;

    // Deferred window state changes for thread safety
    bool m_fullscreenChangeRequested = false;
    bool m_pendingFullscreenState    = false;
    bool m_sizeChangeRequested       = false;
    int m_pendingWidth               = 0;
    int m_pendingHeight              = 0;
    bool m_titleChangeRequested      = false;
    std::string m_pendingTitle;

    void applyFullscreenChange();
    void applySizeChange();
    void applyTitleChange();

    std::string m_title;

    // Setup config change callbacks
    void setupConfigCallbacks();
    std::vector<uint32_t> m_configCallbackHandles;

    ResizeCallback m_resizeCallback;
    std::function<void(double, double)> m_mouseCallback;
  };
} // namespace kst::core::application
