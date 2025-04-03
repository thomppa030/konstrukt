#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <fstream>

#include <nlohmann/json.hpp>

#include "core/log/Logger.hpp"

namespace kst::core {

/**
 * @class Config
 * @brief A configuration system for loading and accessing application settings from JSON files
 */
class Config {
public:
  /**
   * @brief Initialize the configuration system with a JSON file
   * @param configFilePath Path to the JSON configuration file
   * @return True if configuration was loaded successfully, false otherwise
   */
  static bool init(const std::string& configFilePath);

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

private:
  static nlohmann::json getJsonValue(const std::string& key);
  static nlohmann::json s_configData;
  static bool s_initialized;
};

} // namespace kst::core