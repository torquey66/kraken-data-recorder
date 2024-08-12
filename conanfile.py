from conan import ConanFile

class KrakpotRecipe(ConanFile):
    generators = "CMakeToolchain", "CMakeDeps"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("arrow/16.1.0")
        self.requires("boost/1.85.0", force=True)
        self.requires("date/3.0.1")
        self.requires("doctest/2.4.11")
        self.requires("openssl/3.2.0")
        self.requires("simdjson/3.6.1")
        self.requires("thrift/0.17.0")

    def configure(self):
        self.options["arrow/*"].parquet = True
        self.options["arrow/*"].with_boost = True
        self.options["arrow/*"].with_snappy = True
        self.options["arrow/*"].with_thrift = True
        self.options["boost/*"].with_cobolt = False

    def build_requirements(self):
        self.tool_requires("cmake/3.29.0")
