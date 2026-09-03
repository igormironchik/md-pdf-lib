from conan import ConanFile

class DependenciesRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "PkgConfigDeps"

    def requirements(self):
        self.requires("zlib/1.3.1")

    def configure(self):
        self.options["zlib"].shared = False

    def imports(self):
        self.copy("*.dll", "bin", "bin")
        self.copy("*.dylib", "lib", "lib")
