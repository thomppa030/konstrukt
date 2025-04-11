#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace kst::core::application {

  /**
   * @struct WindowSize
   * @brief Simple structure to hold window dimensions
   */
  struct WindowSize {
    int width;
    int height;
  };

  /**
   * @class Window
   * @brief Abstraction over the platform window system
   * 
   * This class handles window creation, event processing, and window state management.
   * It uses GLFW internally but provides an abstracted interface to decouple the engine
   * from the specific windowing library. This allows for potential future changes to
   * the windowing system without impacting other engine components.
   * 
   * The class also implements deferred state changes for thread safety, since GLFW
   * requires certain functions to be called from the main thread only.
   */
  class Window {
  public:
    /**
     * @brief Default constructor
     * 
     * Only creates the object, does not initialize GLFW or create a window.
     * Call create() to actually create the window.
     */
    Window() {
      // Default constructor - does not create a window yet
    }

    ~Window();
    
    // Windows should not be copied or moved
    // This is because GLFW windows have a single identity and contain 
    // internal user pointer references that would be invalidated
    Window(const Window&)                      = delete;
    auto operator=(const Window&) -> Window&   = delete;
    Window(const Window&&)                     = delete;
    auto operator=(const Window&&) -> Window&& = delete;

    /**
     * @brief Create and initialize the window
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     * @param fullscreen Whether to create in fullscreen mode
     * @param resizable Whether the window can be resized by the user
     * @return True if window creation succeeded
     * 
     * This initializes GLFW and creates the actual window. The window is created
     * without a specific graphics API to allow the renderer to choose the appropriate API.
     */
    auto create(
        const std::string& title,
        int width,
        int height,
        bool fullscreen = false,
        bool resizable  = true
    ) -> bool;
    
    /**
     * @brief Destroy the window and terminate GLFW
     * 
     * This should be called before application exit to ensure proper cleanup.
     */
    void destroy();

    /**
     * @brief Process any pending window events
     * 
     * Also applies any deferred window state changes that were requested from another thread.
     * This must be called regularly (usually once per frame) to handle input and window events.
     */
    void pollEvents();
    
    /**
     * @brief Check if the window has been requested to close
     * @return True if the window should close
     * 
     * This can be triggered by the user clicking the close button or by programmatically
     * setting the close flag with glfwSetWindowShouldClose.
     */
    auto shouldClose() -> bool;

    /**
     * @brief Get the native window handle for renderer initialization
     * @return Void pointer to the GLFWwindow (cast to void* to maintain API abstraction)
     */
    auto getNativeWindow() const -> void* { return m_nativeWindow; }
    
    /**
     * @brief Get the current window width
     * @return Window width in pixels
     */
    auto getWidth() const -> int { return m_size.width; }
    
    /**
     * @brief Get the current window height
     * @return Window height in pixels
     */
    auto getHeight() const -> int { return m_size.height; }

    /**
     * @brief Type alias for the window resize callback function
     * 
     * The callback receives the new width and height in pixels.
     */
    using ResizeCallback = std::function<void(int, int)>;
    
    /**
     * @brief Set the callback for window resize events
     * @param callback Function to call when window is resized
     * 
     * This is typically used by the renderer to adjust viewport and recreate swapchain.
     */
    void setResizeCallback(ResizeCallback callback);

    /**
     * @brief Get the current resize callback
     * @return Reference to the current resize callback
     * 
     * Used internally by the GLFW callback wrapper function.
     */
    auto getResizeCallback() const -> const ResizeCallback& { return m_resizeCallback; }

    /**
     * @brief Change the window title
     * @param title New window title
     * 
     * This change is deferred to the next pollEvents() call for thread safety.
     */
    void setWindowTitle(const std::string& title);
    
    /**
     * @brief Change the window size
     * @param width New width in pixels
     * @param height New height in pixels
     * 
     * This change is deferred to the next pollEvents() call for thread safety.
     * Has no effect in fullscreen mode.
     */
    void setWindowSize(int width, int height);
    
    /**
     * @brief Enable or disable vsync
     * @param enabled True to enable vsync, false to disable
     * 
     * The actual implementation of vsync happens in the renderer when presenting frames.
     */
    void setVSync(bool enabled);
    
    /**
     * @brief Switch between fullscreen and windowed mode
     * @param enabled True for fullscreen, false for windowed
     * 
     * This change is deferred to the next pollEvents() call for thread safety.
     */
    void setFullscreen(bool enabled);

  private:
    // Native window handle (GLFWwindow* cast to void* for abstraction)
    void* m_nativeWindow = nullptr;
    
    // Current window dimensions
    WindowSize m_size{.width = 0, .height = 0};
    
    // Window state flags
    bool m_vsync      = false;
    bool m_fullscreen = false;

    // Deferred window state changes for thread safety
    // GLFW functions must be called from the main thread, so we queue changes
    // and apply them in pollEvents() which is called from the main thread
    bool m_fullscreenChangeRequested = false;
    bool m_pendingFullscreenState    = false;
    bool m_sizeChangeRequested       = false;
    int m_pendingWidth               = 0;
    int m_pendingHeight              = 0;
    bool m_titleChangeRequested      = false;
    std::string m_pendingTitle;

    // Helper methods to apply deferred changes
    void applyFullscreenChange();
    void applySizeChange();
    void applyTitleChange();

    // Current window title
    std::string m_title;

    // Register config callbacks to respond to config file changes
    void setupConfigCallbacks();
    
    // Handles for registered config callbacks (used for cleanup)
    std::vector<uint32_t> m_configCallbackHandles;

    // Callback for window resize events
    ResizeCallback m_resizeCallback;
    
    // Callback for mouse events (for future use)
    std::function<void(double, double)> m_mouseCallback;
  };
} // namespace kst::core::application
