from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake, cmake_layout

class BlahdioConanfile(ConanFile):
	name = "blahdio"
	version = "1.0"
	license = "MIT"
	settings = "os", "compiler", "build_type", "arch"
	implements = ["auto_shared_fpic"]
	exports_sources = "CMakeLists.txt", "include/*", "src/*", "tests/*"
	options = {
		"shared": [True, False],
		"fPIC": [True, False],
		"enable_flac": [True, False],
		"enable_mp3": [True, False],
		"enable_wav": [True, False],
		"enable_wavpack": [True, False],
		"build_tests": [True, False],
	}
	default_options = {
		"shared": False,
		"fPIC": True,
		"enable_flac": True,
		"enable_mp3": True,
		"enable_wav": True,
		"enable_wavpack": True,
		"build_tests": False,
	}

	def layout(self):
		cmake_layout(self)

	def requirements(self):
		needs_mackron = self.options.enable_flac or self.options.enable_mp3 or self.options.enable_wav
		needs_wavpack = self.options.enable_wavpack
		self.requires("tl-expected/20190710")
		self.requires("utfcpp/4.0.5")
		if needs_mackron:
			self.requires("dr_libs/cci.20230529")
			self.requires("miniaudio/0.11.21")
		if needs_wavpack:
			self.requires("wavpack/5.8.1")

	def build_requirements(self):
		self.tool_requires("cmake/[>=3.16]")

	def generate(self):
		deps = CMakeDeps(self)
		deps.generate()
		tc = CMakeToolchain(self)
		tc.variables["BLAHDIO_ENABLE_FLAC"]    = self.options.enable_flac
		tc.variables["BLAHDIO_ENABLE_MP3"]     = self.options.enable_mp3
		tc.variables["BLAHDIO_ENABLE_WAV"]     = self.options.enable_wav
		tc.variables["BLAHDIO_ENABLE_WAVPACK"] = self.options.enable_wavpack
		tc.variables["BLAHDIO_BUILD_TESTS"]    = self.options.build_tests
		tc.generate()

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.libs = ["blahdio"]
		self.cpp_info.includedirs = ["include"]
