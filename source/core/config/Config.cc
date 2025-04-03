#include "Config.hpp"

#include <string>

namespace kst::core {

  nlohmann::json Config::s_configData;
  bool Config::s_initialized = false;

  auto Config::init(const std::string& configFilePath) -> bool {
    try {
      std::ifstream file(configFilePath);
      if (!file.is_open()) {
        Logger::error<const std::string&>("Config: Failed to open config file: {}", configFilePath);
        return false;
      }

      s_configData  = nlohmann::json::parse(file);
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

} // namespace kst::core

