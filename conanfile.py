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
        self.requires("zstd/1.5.7", override=True)

        # Display
        self.requires("glfw/3.4")

        # Math
        self.requires("glm/1.0.1")

        self.requires("boost/1.87.0")

        # Asset Loading
        self.requires("assimp/5.4.3")
        self.requires("ktx/4.3.2")
        self.requires("meshoptimizer/0.22")

        # Unit Tests
        self.requires("gtest/1.16.0")
