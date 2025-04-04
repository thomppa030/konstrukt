#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>

#include "core/config/Config.hpp"

namespace fs = std::filesystem;

class ConfigTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Create a test config file
    const std::string testConfig = R"({
      "app": {
        "name": "Konstrukt",
        "version": "1.0.0",
        "debug": true
      },
      "renderer": {
        "width": 1280,
        "height": 720,
        "vsync": true,
        "msaa": 4,
        "gamma": 2.2,
        "backend": "vulkan"
      },
      "performance": {
        "maxFPS": 120,
        "enableProfiling": false
      }
    })";
    
    testConfigPath = fs::temp_directory_path() / "konstrukt_test_config.json";
    std::ofstream file(testConfigPath);
    file << testConfig;
    file.close();
    
    ASSERT_TRUE(kst::core::Config::init(testConfigPath.string()));
  }

  void TearDown() override {
    // Disable file watching if it's enabled
    if (kst::core::Config::isWatchingEnabled()) {
      kst::core::Config::setWatchingEnabled(false);
    }
    
    // Delete test config file
    if (fs::exists(testConfigPath)) {
      fs::remove(testConfigPath);
    }
  }

  fs::path testConfigPath;
};

TEST_F(ConfigTest, GetString) {
  EXPECT_EQ(kst::core::Config::getString("app.name"), "Konstrukt");
  EXPECT_EQ(kst::core::Config::getString("renderer.backend"), "vulkan");
  EXPECT_EQ(kst::core::Config::getString("nonexistent", "default"), "default");
}

TEST_F(ConfigTest, GetInt) {
  EXPECT_EQ(kst::core::Config::getInt("renderer.width"), 1280);
  EXPECT_EQ(kst::core::Config::getInt("renderer.height"), 720);
  EXPECT_EQ(kst::core::Config::getInt("renderer.msaa"), 4);
  EXPECT_EQ(kst::core::Config::getInt("nonexistent", 42), 42);
}

TEST_F(ConfigTest, GetFloat) {
  EXPECT_FLOAT_EQ(kst::core::Config::getFloat("renderer.gamma"), 2.2f);
  EXPECT_FLOAT_EQ(kst::core::Config::getFloat("nonexistent", 3.14f), 3.14f);
}

TEST_F(ConfigTest, GetBool) {
  EXPECT_TRUE(kst::core::Config::getBool("app.debug"));
  EXPECT_TRUE(kst::core::Config::getBool("renderer.vsync"));
  EXPECT_FALSE(kst::core::Config::getBool("performance.enableProfiling"));
  EXPECT_TRUE(kst::core::Config::getBool("nonexistent", true));
}

TEST_F(ConfigTest, HasKey) {
  EXPECT_TRUE(kst::core::Config::hasKey("app.name"));
  EXPECT_TRUE(kst::core::Config::hasKey("renderer.width"));
  EXPECT_FALSE(kst::core::Config::hasKey("nonexistent"));
}

TEST_F(ConfigTest, Reload) {
  // Initially, get the value to verify
  EXPECT_EQ(kst::core::Config::getString("app.name"), "Konstrukt");
  
  // Update the config file with new content
  const std::string updatedConfig = R"({
    "app": {
      "name": "UpdatedName",
      "version": "1.0.0",
      "debug": true
    },
    "renderer": {
      "width": 1280,
      "height": 720,
      "vsync": true,
      "msaa": 4,
      "gamma": 2.2,
      "backend": "vulkan"
    }
  })";
  
  // Write the updated config
  std::ofstream file(testConfigPath);
  file << updatedConfig;
  file.close();
  
  // Manually reload the config
  EXPECT_TRUE(kst::core::Config::reload());
  
  // Check if the value was updated
  EXPECT_EQ(kst::core::Config::getString("app.name"), "UpdatedName");
}

TEST_F(ConfigTest, FileWatcher) {
  // Enable watching with explicit call
  kst::core::Config::setWatchingEnabled(true);
  EXPECT_TRUE(kst::core::Config::isWatchingEnabled());
  
  // Initially, get the value to verify
  EXPECT_EQ(kst::core::Config::getString("app.name"), "Konstrukt");
  
  // Update the config file with new content
  const std::string updatedConfig = R"({
    "app": {
      "name": "WatchedUpdate",
      "version": "2.0.0",
      "debug": false
    },
    "renderer": {
      "width": 1920,
      "height": 1080,
      "vsync": false,
      "msaa": 8,
      "gamma": 2.4,
      "backend": "vulkan"
    }
  })";
  
  // Write the updated config
  std::ofstream file(testConfigPath);
  file << updatedConfig;
  file.close();
  
  // Wait a bit for the file watcher to detect the change
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  
  // Check if the value was automatically updated
  EXPECT_EQ(kst::core::Config::getString("app.name"), "WatchedUpdate");
  EXPECT_EQ(kst::core::Config::getString("app.version"), "2.0.0");
  EXPECT_FALSE(kst::core::Config::getBool("app.debug"));
  EXPECT_EQ(kst::core::Config::getInt("renderer.width"), 1920);
  
  // Disable watching
  kst::core::Config::setWatchingEnabled(false);
  EXPECT_FALSE(kst::core::Config::isWatchingEnabled());
}

TEST_F(ConfigTest, InitWithWatching) {
  // First disable any existing watching
  kst::core::Config::setWatchingEnabled(false);
  
  // Create a separate config file for this test
  fs::path watchedConfigPath = fs::temp_directory_path() / "konstrukt_watched_config.json";
  
  const std::string configContent = R"({
    "test": {
      "value": "initial"
    }
  })";
  
  // Write the config
  std::ofstream file(watchedConfigPath);
  file << configContent;
  file.close();
  
  // Initialize with watching enabled
  EXPECT_TRUE(kst::core::Config::init(watchedConfigPath.string(), true));
  EXPECT_TRUE(kst::core::Config::isWatchingEnabled());
  
  // Check initial value
  EXPECT_EQ(kst::core::Config::getString("test.value"), "initial");
  
  // Update the config file
  const std::string updatedConfig = R"({
    "test": {
      "value": "updated"
    }
  })";
  
  // Write the updated config
  std::ofstream updatedFile(watchedConfigPath);
  updatedFile << updatedConfig;
  updatedFile.close();
  
  // Wait a bit for the file watcher to detect the change
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  
  // Check if the value was updated
  EXPECT_EQ(kst::core::Config::getString("test.value"), "updated");
  
  // Clean up
  kst::core::Config::setWatchingEnabled(false);
  if (fs::exists(watchedConfigPath)) {
    fs::remove(watchedConfigPath);
  }
}

TEST_F(ConfigTest, SpecificKeyCallback) {
  // Create variables to track callback invocation
  std::atomic<bool> callbackInvoked = false;
  std::string callbackKey;
  std::string newNameValue;
  
  // Register a callback for the app.name key
  auto handle = kst::core::Config::onValueChanged("app.name", 
    [&callbackInvoked, &callbackKey, &newNameValue](const std::string& key, const nlohmann::json& value) {
      callbackInvoked = true;
      callbackKey = key;
      newNameValue = value.get<std::string>();
    }
  );
  
  // Ensure the callback handle is valid
  EXPECT_GT(handle, 0);
  
  // Update the config file with a new name
  const std::string updatedConfig = R"({
    "app": {
      "name": "CallbackTest",
      "version": "1.0.0",
      "debug": true
    },
    "renderer": {
      "width": 1280,
      "height": 720,
      "vsync": true,
      "msaa": 4,
      "gamma": 2.2,
      "backend": "vulkan"
    }
  })";
  
  // Write the updated config
  std::ofstream file(testConfigPath);
  file << updatedConfig;
  file.close();
  
  // Manually reload to trigger the callback
  EXPECT_TRUE(kst::core::Config::reload());
  
  // Check if callback was invoked with correct values
  EXPECT_TRUE(callbackInvoked);
  EXPECT_EQ(callbackKey, "app.name");
  EXPECT_EQ(newNameValue, "CallbackTest");
  
  // Remove the callback
  EXPECT_TRUE(kst::core::Config::removeCallback(handle));
}

TEST_F(ConfigTest, AnyChangeCallback) {
  // Create variables to track callback invocation
  std::atomic<int> callbackCount = 0;
  std::vector<std::string> changedKeys;
  
  // Register a callback for any changes
  auto handle = kst::core::Config::onAnyValueChanged(
    [&callbackCount, &changedKeys](const std::string& key, const nlohmann::json&) {
      callbackCount++;
      changedKeys.push_back(key);
    }
  );
  
  // Ensure the callback handle is valid
  EXPECT_GT(handle, 0);
  
  // Update the config file with multiple changes
  const std::string updatedConfig = R"({
    "app": {
      "name": "GlobalCallbackTest",
      "version": "2.0.0",
      "debug": false
    },
    "renderer": {
      "width": 1920,
      "height": 1080,
      "vsync": false,
      "msaa": 8
    }
  })";
  
  // Write the updated config
  std::ofstream file(testConfigPath);
  file << updatedConfig;
  file.close();
  
  // Manually reload to trigger the callback
  EXPECT_TRUE(kst::core::Config::reload());
  
  // Check that callback was invoked multiple times for different keys
  EXPECT_GT(callbackCount, 0);
  
  // Check that the callback received notifications about key changes
  EXPECT_TRUE(std::find(changedKeys.begin(), changedKeys.end(), "app.name") != changedKeys.end());
  EXPECT_TRUE(std::find(changedKeys.begin(), changedKeys.end(), "app.debug") != changedKeys.end());
  EXPECT_TRUE(std::find(changedKeys.begin(), changedKeys.end(), "renderer.width") != changedKeys.end());
  
  // Remove the callback
  EXPECT_TRUE(kst::core::Config::removeCallback(handle));
}

TEST_F(ConfigTest, MultipleCallbacks) {
  // Create variables to track callback invocations
  std::atomic<bool> nameCallbackInvoked = false;
  std::atomic<bool> widthCallbackInvoked = false;
  std::atomic<int> anyChangeCallbackCount = 0;
  
  // Register callbacks for specific keys
  auto nameHandle = kst::core::Config::onValueChanged("app.name", 
    [&nameCallbackInvoked](const std::string&, const nlohmann::json&) {
      nameCallbackInvoked = true;
    }
  );
  
  auto widthHandle = kst::core::Config::onValueChanged("renderer.width", 
    [&widthCallbackInvoked](const std::string&, const nlohmann::json&) {
      widthCallbackInvoked = true;
    }
  );
  
  auto anyHandle = kst::core::Config::onAnyValueChanged(
    [&anyChangeCallbackCount](const std::string&, const nlohmann::json&) {
      anyChangeCallbackCount++;
    }
  );
  
  // Update the config file with changes to both watched keys
  const std::string updatedConfig = R"({
    "app": {
      "name": "MultiCallbackTest",
      "version": "1.0.0",
      "debug": true
    },
    "renderer": {
      "width": 3840,
      "height": 2160,
      "vsync": true,
      "msaa": 4,
      "gamma": 2.2,
      "backend": "vulkan"
    }
  })";
  
  // Write the updated config
  std::ofstream file(testConfigPath);
  file << updatedConfig;
  file.close();
  
  // Manually reload to trigger the callbacks
  EXPECT_TRUE(kst::core::Config::reload());
  
  // Check that all callbacks were invoked appropriately
  EXPECT_TRUE(nameCallbackInvoked);
  EXPECT_TRUE(widthCallbackInvoked);
  EXPECT_GT(anyChangeCallbackCount, 0);
  
  // Remove the callbacks
  EXPECT_TRUE(kst::core::Config::removeCallback(nameHandle));
  EXPECT_TRUE(kst::core::Config::removeCallback(widthHandle));
  EXPECT_TRUE(kst::core::Config::removeCallback(anyHandle));
}

TEST_F(ConfigTest, RemoveCallback) {
  // Variables to track callback invocation
  std::atomic<bool> callbackInvoked = false;
  
  // Register a callback
  auto handle = kst::core::Config::onValueChanged("app.name", 
    [&callbackInvoked](const std::string&, const nlohmann::json&) {
      callbackInvoked = true;
    }
  );
  
  // Remove the callback immediately
  EXPECT_TRUE(kst::core::Config::removeCallback(handle));
  
  // Update the config
  const std::string updatedConfig = R"({
    "app": {
      "name": "RemoveCallbackTest",
      "version": "1.0.0",
      "debug": true
    },
    "renderer": {
      "width": 1280,
      "height": 720,
      "vsync": true,
      "msaa": 4,
      "gamma": 2.2,
      "backend": "vulkan"
    }
  })";
  
  // Write the updated config
  std::ofstream file(testConfigPath);
  file << updatedConfig;
  file.close();
  
  // Reload to see if the callback is triggered
  EXPECT_TRUE(kst::core::Config::reload());
  
  // Ensure the callback was not invoked after being removed
  EXPECT_FALSE(callbackInvoked);
  
  // Try removing a non-existent callback
  EXPECT_FALSE(kst::core::Config::removeCallback(9999));
}