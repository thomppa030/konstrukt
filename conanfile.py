from conan import ConanFile
import os


class Konstrukt(ConanFile):
    name = "Konstrukt"
    version = "0.1.0"
    settings = {
        "os",
        "compiler",
        "arch",
        "build_type",
    }
    generators = "CMakeToolchain", "CMakeDeps"

    def layout(self):
        self.folders.generators = os.path.join(
            "build", str(self.settings.build_type), "generators")
        self.folders.build = os.path.join(
            "build", str(self.settings.build_type))

    def requirements(self):
        # Vulkan requirements
        self.requires("vulkan-loader/1.4.309.0")
        self.requires("vulkan-headers/1.4.309.0")

        # Display
        self.requires("glfw/3.4")

        # Math
        self.requires("glm/1.0.1")

        # Logging and parsing
        self.requires("fmt/11.1.3")
        self.requires("spdlog/1.15.1", override=True)

        # UI
        self.requires("imgui/1.90.5-docking", override=True)
        self.requires("imguizmo/1.83")

        # Asset Loading
        self.requires("assimp/5.4.3")
        self.requires("ktx/4.3.2")
        self.requires("meshoptimizer/0.22")

        # Unit Tests
        self.requires("gtest/1.16.0")

        # performance profiling
        self.requires("tracy/0.11.1")
