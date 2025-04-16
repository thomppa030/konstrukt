#include <exception>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/Logger.hpp"
#include "renderer/RHI/GraphicsContext.hpp"

auto main() -> int {
  kst::core::Logger::init();

  try {
    if (!glfwInit()) {
      KST_ERROR("Failed to initialize GLFW");
      return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const uint32_t WIDTH  = 800;
    const uint32_t HEIGHT = 600;
    GLFWwindow* window    = glfwCreateWindow(WIDTH, HEIGHT, "Konstrukt Engine", nullptr, nullptr);

    if (!window) {
      KST_ERROR("Failed to create GLFW window");
      glfwTerminate();
      return -1;
    }

    // Configure and create rendering context with window handle
    kst::renderer::ContextOptions options;
    options.enableValidation  = true;
    options.printEnumerations = true;
    options.window            = window;
    options.width             = WIDTH;
    options.height            = HEIGHT;

    auto context = kst::renderer::GraphicsContext::create("vulkan", options);

    if (!context) {
      KST_ERROR("Failed to create rendering context");
      glfwDestroyWindow(window);
      glfwTerminate();
      return -1;
    }

    kst::renderer::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nativeWindowHandle = window;
    surfaceDesc.width              = WIDTH;
    surfaceDesc.height             = HEIGHT;

    if (!context->createSurface(surfaceDesc)) {
      KST_ERROR("Failed to create surface and swapchain");
      glfwDestroyWindow(window);
      glfwTerminate();
      return -1;
    }

    KST_INFO("Created {} rendering context with a surface", context->getImplementationName());

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }

    context->waitIdle();
    glfwDestroyWindow(window);
    glfwTerminate();

  } catch (const std::exception& e) {
    KST_ERROR("Unhandled exception: {}", e.what());
    glfwTerminate();
  }
  return 0;
}
