from conan import ConanFile
from conan.tools.files import get, copy
from conan.tools.layout import basic_layout
import os


class ImGuiFileBrowserConan(ConanFile):
    name = "imgui-filebrowser"
    # Map your version to an identifiable tag, or a "cci" format based on the date
    version = "1.0.0"

    # The commit hash to use, since this project doesn't have versio numbers
    _commit_hash = "47a1884"

    description = "A header-only file browser implementation for dear-imgui"
    license = "MIT"
    homepage = "https://github.com/AirGuanZ/imgui-filebrowser"
    topics = ("imgui", "filebrowser", "header-only")
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True

    def layout(self):
        basic_layout(self)

    def source(self):
        # GitHub generates source archives automatically for any commit hash
        get(
            self,
            url=f"https://github.com/haraldwer/ImGui-FileBrowser/archive/{self._commit_hash}.tar.gz",
            strip_root=True,
        )

    def package(self):
        copy(
            self,
            pattern="imfilebrowser.h",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "include"),
        )

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property(
            "cmake_target_name", "imgui-filebrowser::imgui-filebrowser"
        )
