#include "ApplicationWindow.hpp"

#include <string>

#include <GLFW/glfw3.h>

#include "core/log/Logger.hpp"

namespace kst::core::application {

  // Static callback for GLFW resize events
  void glfwResizeCallback(GLFWwindow* window, int width, int height) {
    auto* appWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if ((appWindow != nullptr) && appWindow->getResizeCallback()) {
      appWindow->getResizeCallback()(width, height);
    }
  }

  auto Window::create(const std::string& title, int width, int height) -> bool {
    // Initialize GLFW if not already done
    if (glfwInit() == 0) {
      kst::core::Logger::error("Failed to initialize GLFW");
      return false;
    }

    // Configure GLFW for Vulkan
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create the window
    m_nativeWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
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
    glfwPollEvents();
  }

  auto Window::shouldClose() -> bool {
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(m_nativeWindow)) != 0;
  }

  void Window::setResizeCallback(ResizeCallback callback) {
    m_resizeCallback = std::move(callback);
  }

} // namespace kst::core::application
