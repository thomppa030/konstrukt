#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

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