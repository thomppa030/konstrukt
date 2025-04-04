#include "Config.hpp"

#include <string>
#include <chrono>
#include <filesystem>
#include <set>

namespace kst::core {

// FileWatcher implementation
FileWatcher::FileWatcher(const std::string& filePath, Callback callback, uint32_t pollInterval_ms)
  : m_filePath(filePath)
  , m_callback(callback)
  , m_pollInterval_ms(pollInterval_ms)
  , m_running(false)
{
  // Get initial file write time
  if (std::filesystem::exists(filePath)) {
    m_lastWriteTime = std::filesystem::last_write_time(filePath);
  }
}

FileWatcher::~FileWatcher() {
  stop();
}

void FileWatcher::start() {
  if (m_running) {
    return;
  }

  m_running = true;
  m_watchThread = std::thread(&FileWatcher::watchThread, this);
}

void FileWatcher::stop() {
  if (!m_running) {
    return;
  }

  m_running = false;
  if (m_watchThread.joinable()) {
    m_watchThread.join();
  }
}

bool FileWatcher::isRunning() const {
  return m_running;
}

void FileWatcher::watchThread() {
  while (m_running) {
    try {
      // Check if file exists
      if (std::filesystem::exists(m_filePath)) {
        auto lastWriteTime = std::filesystem::last_write_time(m_filePath);
        
        // If file was modified
        if (lastWriteTime != m_lastWriteTime) {
          m_lastWriteTime = lastWriteTime;
          
          // Call the callback with the file path
          m_callback(m_filePath);
        }
      }
    } catch (const std::exception& e) {
      // Log error but continue watching
      Logger::error<const char*>("FileWatcher: Error watching file: {}", e.what());
    }

    // Sleep for the polling interval
    std::this_thread::sleep_for(std::chrono::milliseconds(m_pollInterval_ms));
  }
}

// Config implementation
nlohmann::json Config::s_configData;
nlohmann::json Config::s_previousConfigData;
bool Config::s_initialized = false;
std::string Config::s_configFilePath;
std::unique_ptr<FileWatcher> Config::s_fileWatcher;
bool Config::s_watchingEnabled = false;
std::vector<Config::CallbackInfo> Config::s_callbacks;
Config::CallbackHandle Config::s_nextCallbackHandle = 1;  // Start from 1, 0 can be used as invalid handle
std::mutex Config::s_callbackMutex;

auto Config::init(const std::string& configFilePath, bool watchForChanges) -> bool {
  s_configFilePath = configFilePath;
  
  // Load the configuration
  if (!loadFromFile(configFilePath)) {
    return false;
  }
  
  // Store initial state for change detection
  s_previousConfigData = s_configData;

  // Set up file watching if requested
  if (watchForChanges) {
    setWatchingEnabled(true);
  }
  
  return true;
}

auto Config::setWatchingEnabled(bool enable) -> void {
  // If already in the requested state, do nothing
  if (s_watchingEnabled == enable) {
    return;
  }
  
  s_watchingEnabled = enable;
  
  if (enable) {
    // Create and start file watcher
    if (!s_fileWatcher) {
      s_fileWatcher = std::make_unique<FileWatcher>(
        s_configFilePath,
        &Config::onConfigFileChanged,
        1000  // Check every second
      );
    }
    s_fileWatcher->start();
    Logger::info<const std::string&>("Config: Started watching for changes to {}", s_configFilePath);
  } else if (s_fileWatcher) {
    // Stop file watcher
    s_fileWatcher->stop();
    Logger::info<const std::string&>("Config: Stopped watching for changes to {}", s_configFilePath);
  }
}

auto Config::isWatchingEnabled() -> bool {
  return s_watchingEnabled;
}

auto Config::reload() -> bool {
  Logger::info<const std::string&>("Config: Reloading configuration from {}", s_configFilePath);
  
  // Store current config for change detection
  s_previousConfigData = s_configData;
  
  // Load the new config
  if (loadFromFile(s_configFilePath)) {
    // Find changed values and notify callbacks
    std::unordered_map<std::string, nlohmann::json> changedValues;
    flattenJSON(s_configData, "", changedValues);
    notifyCallbacks(changedValues);
    return true;
  }
  
  return false;
}

auto Config::loadFromFile(const std::string& configFilePath) -> bool {
  try {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
      Logger::error<const std::string&>("Config: Failed to open config file: {}", configFilePath);
      return false;
    }

    s_configData = nlohmann::json::parse(file);
    s_initialized = true;
    Logger::info<const std::string&>(
        "Config: Successfully loaded configuration from {}",
        configFilePath
    );
    return true;
  } catch (const nlohmann::json::parse_error& e) {
    Logger::error<const std::string&, const char*>(
        "Config: Failed to parse config file {}: {}",
        configFilePath,
        e.what()
    );
    return false;
  } catch (const std::exception& e) {
    Logger::error<const std::string&, const char*>(
        "Config: Error loading config file {}: {}",
        configFilePath,
        e.what()
    );
    return false;
  }
}

auto Config::onConfigFileChanged(const std::string& filePath) -> void {
  Logger::info<const std::string&>("Config: Detected change in configuration file: {}", filePath);
  reload();
}

auto Config::getJsonValue(const std::string& key) -> nlohmann::json {
  if (!s_initialized) {
    Logger::warn<>("Config: Attempting to access configuration before initialization");
    return {};
  }

  // Handle dot notation for nested configs
  std::string::size_type pos     = 0;
  std::string::size_type prevPos = 0;
  std::string keyPart;

  nlohmann::json* currentJson = &s_configData;

  while ((pos = key.find('.', prevPos)) != std::string::npos) {
    keyPart = key.substr(prevPos, pos - prevPos);

    if (!currentJson->contains(keyPart) || !(*currentJson)[keyPart].is_object()) {
      return {};
    }

    currentJson = &(*currentJson)[keyPart];
    prevPos     = pos + 1;
  }

  keyPart = key.substr(prevPos);

  if (!currentJson->contains(keyPart)) {
    return {};
  }

  return (*currentJson)[keyPart];
}

auto Config::getString(const std::string& key, const std::string& defaultValue) -> std::string {
  nlohmann::json value = getJsonValue(key);
  if (value.is_null() || !value.is_string()) {
    return defaultValue;
  }
  return value.get<std::string>();
}

auto Config::getInt(const std::string& key, int defaultValue) -> int {
  nlohmann::json value = getJsonValue(key);
  if (value.is_null() || !value.is_number_integer()) {
    return defaultValue;
  }
  return value.get<int>();
}

auto Config::getFloat(const std::string& key, float defaultValue) -> float {
  nlohmann::json value = getJsonValue(key);
  if (value.is_null() || !value.is_number_float()) {
    return defaultValue;
  }
  return value.get<float>();
}

auto Config::getBool(const std::string& key, bool defaultValue) -> bool {
  nlohmann::json value = getJsonValue(key);
  if (value.is_null() || !value.is_boolean()) {
    return defaultValue;
  }
  return value.get<bool>();
}

auto Config::hasKey(const std::string& key) -> bool {
  return !getJsonValue(key).is_null();
}

auto Config::onValueChanged(const std::string& key, ChangeCallback callback) -> CallbackHandle {
  std::lock_guard<std::mutex> lock(s_callbackMutex);
  
  CallbackInfo info{key, callback, s_nextCallbackHandle};
  s_callbacks.push_back(info);
  
  // Log the registration
  Logger::info<const std::string&, uint32_t>(
      "Config: Registered callback for key '{}' with handle {}", 
      key, 
      s_nextCallbackHandle
  );
  
  return s_nextCallbackHandle++;
}

auto Config::onAnyValueChanged(ChangeCallback callback) -> CallbackHandle {
  std::lock_guard<std::mutex> lock(s_callbackMutex);
  
  CallbackInfo info{"", callback, s_nextCallbackHandle};
  s_callbacks.push_back(info);
  
  // Log the registration
  Logger::info<uint32_t>(
      "Config: Registered callback for any changes with handle {}", 
      s_nextCallbackHandle
  );
  
  return s_nextCallbackHandle++;
}

auto Config::removeCallback(CallbackHandle handle) -> bool {
  std::lock_guard<std::mutex> lock(s_callbackMutex);
  
  auto it = std::find_if(s_callbacks.begin(), s_callbacks.end(),
                         [handle](const CallbackInfo& info) {
                           return info.handle == handle;
                         });
  
  if (it != s_callbacks.end()) {
    Logger::info<uint32_t>("Config: Removed callback with handle {}", handle);
    s_callbacks.erase(it);
    return true;
  }
  
  Logger::warn<uint32_t>("Config: Attempted to remove non-existent callback with handle {}", handle);
  return false;
}

auto Config::flattenJSON(const nlohmann::json& json, const std::string& prefix, 
                        std::unordered_map<std::string, nlohmann::json>& result) -> void {
  for (auto it = json.begin(); it != json.end(); ++it) {
    std::string newKey = prefix.empty() ? it.key() : prefix + "." + it.key();
    
    if (it.value().is_object()) {
      // Recurse into nested objects
      flattenJSON(it.value(), newKey, result);
    } else {
      // Add leaf key-value pair
      result[newKey] = it.value();
    }
  }
}

auto Config::notifyCallbacks(const std::unordered_map<std::string, nlohmann::json>& changedValues) -> void {
  if (changedValues.empty()) {
    return;
  }
  
  std::lock_guard<std::mutex> lock(s_callbackMutex);
  
  // Create a set of keys that have changed for quick lookup
  std::set<std::string> changedKeys;
  for (const auto& [key, value] : changedValues) {
    changedKeys.insert(key);
    
    // Process the callbacks for this specific key
    for (const auto& callbackInfo : s_callbacks) {
      if (callbackInfo.key == key) {
        try {
          // Execute the callback with key and new value
          callbackInfo.callback(key, value);
          Logger::debug<const std::string&, uint32_t>(
              "Config: Notified callback for key '{}' with handle {}", 
              key, 
              callbackInfo.handle
          );
        } catch (const std::exception& e) {
          Logger::error<const std::string&, const char*>(
              "Config: Error in callback for key '{}': {}", 
              key, 
              e.what()
          );
        }
      }
    }
  }
  
  // Then process the callbacks for any changes
  for (const auto& callbackInfo : s_callbacks) {
    if (callbackInfo.key.empty()) {  // This is an "any change" callback
      for (const auto& [key, value] : changedValues) {
        try {
          // Execute the callback with key and new value
          callbackInfo.callback(key, value);
          Logger::debug<const std::string&, uint32_t>(
              "Config: Notified 'any change' callback for key '{}' with handle {}", 
              key, 
              callbackInfo.handle
          );
        } catch (const std::exception& e) {
          Logger::error<const std::string&, const char*>(
              "Config: Error in 'any change' callback for key '{}': {}", 
              key, 
              e.what()
          );
        }
      }
    }
  }
}

auto Config::getChangedKeys(const nlohmann::json& oldData, const nlohmann::json& newData) -> std::vector<std::string> {
  std::unordered_map<std::string, nlohmann::json> oldFlat;
  std::unordered_map<std::string, nlohmann::json> newFlat;
  
  flattenJSON(oldData, "", oldFlat);
  flattenJSON(newData, "", newFlat);
  
  std::vector<std::string> changedKeys;
  
  // Check for keys in new data that are different from old data
  for (const auto& [key, newValue] : newFlat) {
    auto oldIt = oldFlat.find(key);
    if (oldIt == oldFlat.end() || oldIt->second != newValue) {
      changedKeys.push_back(key);
    }
  }
  
  // Check for keys that were removed in new data
  for (const auto& [key, oldValue] : oldFlat) {
    if (newFlat.find(key) == newFlat.end()) {
      changedKeys.push_back(key);
    }
  }
  
  return changedKeys;
}

} // namespace kst::core

