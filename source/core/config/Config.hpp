#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <fstream>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <filesystem>
#include <vector>
#include <mutex>
#include <any>

#include <nlohmann/json.hpp>

#include "core/log/Logger.hpp"

namespace kst::core {

/**
 * @class FileWatcher
 * @brief Watches a file for changes and calls a callback when it changes
 */
class FileWatcher {
public:
  using Callback = std::function<void(const std::string&)>;

  /**
   * @brief Construct a new FileWatcher 
   * @param filePath Path to the file to watch
   * @param callback Function to call when the file changes
   * @param pollInterval_ms How often to check for changes in milliseconds
   */
  FileWatcher(const std::string& filePath, Callback callback, uint32_t pollInterval_ms = 1000);
  
  /**
   * @brief Destructor - stops the watching thread
   */
  ~FileWatcher();

  /**
   * @brief Start watching the file
   */
  void start();

  /**
   * @brief Stop watching the file
   */
  void stop();

  /**
   * @brief Check if the watcher is currently running
   * @return True if the watcher is running
   */
  bool isRunning() const;

private:
  void watchThread();

  std::string m_filePath;
  Callback m_callback;
  uint32_t m_pollInterval_ms;
  std::filesystem::file_time_type m_lastWriteTime;
  std::thread m_watchThread;
  std::atomic<bool> m_running;
};

/**
 * @class Config
 * @brief A configuration system for loading and accessing application settings from JSON files
 */
class Config {
public:
  /**
   * @brief Type for change callbacks 
   * @param key The config key that changed
   * @param newValue The new value (as an nlohmann::json object)
   */
  using ChangeCallback = std::function<void(const std::string&, const nlohmann::json&)>;
  
  /**
   * @brief Handle for a registered callback
   */
  using CallbackHandle = uint32_t;

  /**
   * @brief Initialize the configuration system with a JSON file
   * @param configFilePath Path to the JSON configuration file
   * @param watchForChanges Whether to watch the config file for changes (defaults to false)
   * @return True if configuration was loaded successfully, false otherwise
   */
  static bool init(const std::string& configFilePath, bool watchForChanges = false);

  /**
   * @brief Enable or disable file watching
   * @param enable True to enable watching, false to disable
   */
  static void setWatchingEnabled(bool enable);

  /**
   * @brief Check if file watching is enabled
   * @return True if file watching is enabled
   */
  static bool isWatchingEnabled();

  /**
   * @brief Reload the configuration from the file
   * @return True if reloaded successfully, false otherwise
   */
  static bool reload();

  /**
   * @brief Get a string value from the configuration
   * @param key The configuration key (supports dot notation for nested configs)
   * @param defaultValue Value to return if key is not found
   * @return The string value or defaultValue if not found
   */
  static std::string getString(const std::string& key, const std::string& defaultValue = "");

  /**
   * @brief Get an integer value from the configuration
   * @param key The configuration key (supports dot notation for nested configs)
   * @param defaultValue Value to return if key is not found
   * @return The integer value or defaultValue if not found
   */
  static int getInt(const std::string& key, int defaultValue = 0);

  /**
   * @brief Get a floating point value from the configuration
   * @param key The configuration key (supports dot notation for nested configs)
   * @param defaultValue Value to return if key is not found
   * @return The float value or defaultValue if not found
   */
  static float getFloat(const std::string& key, float defaultValue = 0.0f);

  /**
   * @brief Get a boolean value from the configuration
   * @param key The configuration key (supports dot notation for nested configs)
   * @param defaultValue Value to return if key is not found
   * @return The boolean value or defaultValue if not found
   */
  static bool getBool(const std::string& key, bool defaultValue = false);

  /**
   * @brief Check if a configuration key exists
   * @param key The configuration key to check
   * @return True if the key exists in the configuration
   */
  static bool hasKey(const std::string& key);

  /**
   * @brief Register a callback to be called when a specific config value changes
   * @param key The configuration key to watch (supports dot notation)
   * @param callback The function to call when the value changes
   * @return A handle that can be used to unregister the callback
   */
  static CallbackHandle onValueChanged(const std::string& key, ChangeCallback callback);

  /**
   * @brief Register a callback to be called when any config value changes
   * @param callback The function to call when any value changes
   * @return A handle that can be used to unregister the callback
   */
  static CallbackHandle onAnyValueChanged(ChangeCallback callback);

  /**
   * @brief Unregister a previously registered callback
   * @param handle The handle returned by onValueChanged or onAnyValueChanged
   * @return True if the callback was found and removed, false otherwise
   */
  static bool removeCallback(CallbackHandle handle);

private:
  struct CallbackInfo {
    std::string key;            // Empty string for "any" callbacks
    ChangeCallback callback;
    CallbackHandle handle;
  };

  static nlohmann::json getJsonValue(const std::string& key);
  static bool loadFromFile(const std::string& configFilePath);
  static void onConfigFileChanged(const std::string& filePath);
  static void notifyCallbacks(const std::unordered_map<std::string, nlohmann::json>& changedValues);
  static std::vector<std::string> getChangedKeys(const nlohmann::json& oldData, const nlohmann::json& newData);
  static void flattenJSON(const nlohmann::json& json, const std::string& prefix, 
                         std::unordered_map<std::string, nlohmann::json>& result);

  static nlohmann::json s_configData;
  static nlohmann::json s_previousConfigData;
  static bool s_initialized;
  static std::string s_configFilePath;
  static std::unique_ptr<FileWatcher> s_fileWatcher;
  static bool s_watchingEnabled;
  
  static std::vector<CallbackInfo> s_callbacks;
  static CallbackHandle s_nextCallbackHandle;
  static std::mutex s_callbackMutex;
};

} // namespace kst::core