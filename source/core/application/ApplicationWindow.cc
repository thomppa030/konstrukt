#include "ApplicationWindow.hpp"

#include <string>

#include <GLFW/glfw3.h>

#include "core/config/Config.hpp"
#include "core/log/Logger.hpp"

namespace kst::core::application {

  namespace {
    /**
     * @brief GLFW callback function for window resize events
     * @param window The GLFW window that was resized
     * @param width New window width in pixels
     * @param height New window height in pixels
     * 
     * This wrapper retrieves the Window instance from GLFW's user pointer and forwards
     * the resize event to the registered callback. This approach allows us to:
     * 1. Use C++ member functions for callbacks, even though GLFW requires C-style functions
     * 2. Maintain encapsulation by hiding GLFW-specific code in this anonymous namespace
     */
    void glfwResizeCallback(GLFWwindow* window, int width, int height) {
      auto* appWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
      if ((appWindow != nullptr) && appWindow->getResizeCallback()) {
        appWindow->getResizeCallback()(width, height);
      }
    }
  } // namespace

  Window::~Window() {
    // Unregister config callbacks
    for (auto handle : m_configCallbackHandles) {
      kst::core::Config::removeCallback(handle);
    }

    destroy();
  }

  auto
  Window::create(const std::string& title, int width, int height, bool fullscreen, bool resizable)
      -> bool {
    if (glfwInit() == 0) {
      kst::core::Logger::error("Failed to initialize GLFW");
      return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

    // Store fullscreen state
    m_fullscreen = fullscreen;

    // Get monitor for fullscreen if needed
    GLFWmonitor* monitor = nullptr;
    if (m_fullscreen) {
      monitor = glfwGetPrimaryMonitor();
      if (monitor != nullptr) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode != nullptr) {
          width  = mode->width;
          height = mode->height;
        }
      }
    }

    // Create the window
    m_nativeWindow = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
    if (m_nativeWindow == nullptr) {
      kst::core::Logger::error("Failed to create GLFW window");
      glfwTerminate();
      return false;
    }

    // Store window size
    m_size.width  = width;
    m_size.height = height;
    m_title       = title;

    // Setup window user pointer and callbacks
    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_nativeWindow), this);
    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(m_nativeWindow), glfwResizeCallback);

    kst::core::Logger::info<const std::string, int, int>(
        "Created window: {} ({}x{})",
        title,
        width,
        height
    );

    // Setup config callbacks
    setupConfigCallbacks();

    return true;
  }

  void Window::destroy() {
    if (m_nativeWindow != nullptr) {
      glfwDestroyWindow(static_cast<GLFWwindow*>(m_nativeWindow));
      m_nativeWindow = nullptr;
      glfwTerminate();
      kst::core::Logger::info("Window destroyed");
    }
  }

  void Window::pollEvents() {
    // Poll for events
    glfwPollEvents();

    // Apply any pending window state changes
    // This is called from the main thread, so it's safe to modify the window
    if (m_titleChangeRequested) {
      applyTitleChange();
    }

    if (m_sizeChangeRequested) {
      applySizeChange();
    }

    if (m_fullscreenChangeRequested) {
      applyFullscreenChange();
    }
  }

  auto Window::shouldClose() -> bool {
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(m_nativeWindow)) != 0;
  }

  void Window::setResizeCallback(ResizeCallback callback) {
    m_resizeCallback = std::move(callback);
  }

  void Window::setWindowTitle(const std::string& title) {
    if ((m_nativeWindow != nullptr) && title != m_title) {
      // Schedule the title change for the next poll cycle (main thread)
      kst::core::Logger::info<const std::string&>("Scheduling window title change to: {}", title);

      m_titleChangeRequested = true;
      m_pendingTitle         = title;
    }
  }

  void Window::applyTitleChange() {
    if (!m_titleChangeRequested || (m_nativeWindow == nullptr)) {
      return;
    }

    // Reset the request flag
    m_titleChangeRequested = false;

    try {
      m_title = m_pendingTitle;
      glfwSetWindowTitle(static_cast<GLFWwindow*>(m_nativeWindow), m_title.c_str());
      kst::core::Logger::info<const std::string&>("Window title changed to: {}", m_title);
    } catch (const std::exception& e) {
      kst::core::Logger::error<const char*>("Error changing window title: {}", e.what());
    }
  }

  void Window::setWindowSize(int width, int height) {
    if ((m_nativeWindow != nullptr) && (width != m_size.width || height != m_size.height)) {
      // Schedule the size change for the next poll cycle (main thread)
      kst::core::Logger::info<int, int>("Scheduling window size change to: {}x{}", width, height);

      m_sizeChangeRequested = true;
      m_pendingWidth        = width;
      m_pendingHeight       = height;
    }
  }

  void Window::applySizeChange() {
    if (!m_sizeChangeRequested || (m_nativeWindow == nullptr)) {
      return;
    }

    // Reset the request flag
    m_sizeChangeRequested = false;

    // Only change size if in windowed mode
    if (!m_fullscreen) {
      try {
        glfwSetWindowSize(
            static_cast<GLFWwindow*>(m_nativeWindow),
            m_pendingWidth,
            m_pendingHeight
        );
        m_size.width  = m_pendingWidth;
        m_size.height = m_pendingHeight;
        kst::core::Logger::info<int, int>(
            "Window size changed to: {}x{}",
            m_pendingWidth,
            m_pendingHeight
        );
      } catch (const std::exception& e) {
        kst::core::Logger::error<const char*>("Error changing window size: {}", e.what());
      }
    } else {
      kst::core::Logger::warn<>("Cannot resize window in fullscreen mode");
    }
  }

  void Window::setVSync(bool enabled) {
    if (m_vsync != enabled) {
      m_vsync = enabled;
      kst::core::Logger::info<bool>("VSync {}", enabled ? "enabled" : "disabled");
      // Note: Actual VSync implementation happens in the renderer when swapchain is presented
    }
  }

  void Window::setFullscreen(bool enabled) {
    // IMPORTANT: GLFW functions must be called from the main thread
    // For fullscreen toggling via config changes, we need to defer the actual change
    // to the next time pollEvents is called (which is on the main thread)

    if ((m_nativeWindow != nullptr) && m_fullscreen != enabled) {
      kst::core::Logger::info<bool>(
          "Scheduling fullscreen mode change to: {}",
          enabled ? "fullscreen" : "windowed"
      );

      // Just mark that we want to change fullscreen state
      // The actual change will happen in pollEvents
      m_fullscreenChangeRequested = true;
      m_pendingFullscreenState    = enabled;
    }
  }

  void Window::applyFullscreenChange() {
    if (!m_fullscreenChangeRequested || (m_nativeWindow == nullptr)) {
      return;
    }

    // Reset the request flag
    m_fullscreenChangeRequested = false;

    // Apply the actual fullscreen change
    GLFWmonitor* monitor = nullptr;
    int xpos             = 0;
    int ypos             = 0;
    int width            = m_size.width;
    int height           = m_size.height;

    if (m_pendingFullscreenState) {
      // Get current window position and size before going fullscreen
      glfwGetWindowPos(static_cast<GLFWwindow*>(m_nativeWindow), &xpos, &ypos);

      // Switch to fullscreen
      monitor = glfwGetPrimaryMonitor();
      if (monitor != nullptr) {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode != nullptr) {
          width  = mode->width;
          height = mode->height;
        }
      }
    }

    try {
      // Set window monitor (NULL for windowed mode)
      glfwSetWindowMonitor(
          static_cast<GLFWwindow*>(m_nativeWindow),
          monitor,
          m_pendingFullscreenState ? 0 : xpos,
          m_pendingFullscreenState ? 0 : ypos,
          width,
          height,
          GLFW_DONT_CARE
      );

      m_fullscreen  = m_pendingFullscreenState;
      m_size.width  = width;
      m_size.height = height;

      kst::core::Logger::info<bool, int, int>(
          "Window switched to {} mode: {}x{}",
          m_fullscreen ? "fullscreen" : "windowed",
          width,
          height
      );
    } catch (const std::exception& e) {
      kst::core::Logger::error<const char*>("Error changing fullscreen mode: {}", e.what());
    }
  }

  void Window::setupConfigCallbacks() {
    // Register for window title changes
    auto titleHandle = kst::core::Config::onValueChanged(
        "window.title",
        [this](const std::string&, const nlohmann::json& value) {
          this->setWindowTitle(value.get<std::string>());
        }
    );
    m_configCallbackHandles.push_back(titleHandle);

    // Register for window size changes
    auto widthHandle = kst::core::Config::onValueChanged(
        "window.width",
        [this](const std::string&, const nlohmann::json& value) {
          // Only update width; setWindowSize will be called once
          int newWidth = value.get<int>();
          this->setWindowSize(newWidth, m_size.height);
        }
    );
    m_configCallbackHandles.push_back(widthHandle);

    auto heightHandle = kst::core::Config::onValueChanged(
        "window.height",
        [this](const std::string&, const nlohmann::json& value) {
          // Only update height; setWindowSize will be called once
          int newHeight = value.get<int>();
          this->setWindowSize(m_size.width, newHeight);
        }
    );
    m_configCallbackHandles.push_back(heightHandle);

    // Register for vsync changes
    auto vsyncHandle = kst::core::Config::onValueChanged(
        "window.vsync",
        [this](const std::string&, const nlohmann::json& value) {
          this->setVSync(value.get<bool>());
        }
    );
    m_configCallbackHandles.push_back(vsyncHandle);

    // Register for fullscreen changes
    auto fullscreenHandle = kst::core::Config::onValueChanged(
        "window.fullscreen",
        [this](const std::string&, const nlohmann::json& value) {
          this->setFullscreen(value.get<bool>());
        }
    );
    m_configCallbackHandles.push_back(fullscreenHandle);

    kst::core::Logger::info<int>(
        "Window registered {} config callbacks",
        static_cast<int>(m_configCallbackHandles.size())
    );
  }

} // namespace kst::core::application
