# pylint: disable=line-too-long,missing-module-docstring,missing-function-docstring

import subprocess, argparse, os, sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

ALL_BUILD_TYPES = ["Debug", "Release", "RelWithDebInfo"]

AP = argparse.ArgumentParser(description="gta-reversed project setup script")
AP.add_argument(
    "--build", 
    action="store_true",
    help="build instead of setting up & configuring"
)
AP.add_argument(
    "--no-unity-build",
    action="store_true",
    help="disable unity build"
)
AP.add_argument(
    "--build-type",
    #default="Debug",
    choices=ALL_BUILD_TYPES,
    help="cmake compilation type",
)
AP.add_argument(
    "--standalone",
    default=False,
    action="store_true",
    help="Build standalone executable instead of .asi plugin (for debugging purposes)",
)
AP.add_argument(
    "--dump-hooks-only",
    default=False,
    action="store_true",
    help="Don't try to write to memory at all, just dump the hooks that would be applied (for debugging purposes) and exit - To provide a path use the `--dump-hooks-to` argument on the resulting executable",
)
AP.add_argument("--profile", default="conanprofile.txt", help="custom profile")
args = AP.parse_args()

def run_conan(build_type: str):
    conan_args = [
        "conan", "install",
        SCRIPT_DIR,
        "--build", "missing",
        "--profile", f"{SCRIPT_DIR}/{args.profile}",
        "-s", f"build_type={build_type}"
    ]
    subprocess.run(conan_args, shell=True, check=True)

def run_cmake() -> None:
    cmake_defines = {
        "GTASA_STANDALONE": args.standalone,
        "GTASA_DUMP_HOOKS_ONLY": args.dump_hooks_only,
        "GTASA_UNITY_BUILD": not args.no_unity_build,
        "GTASA_NO_EDIT_AND_CONTINUE": sys.platform == "linux",  # Disable Edit & Continue on Linux (Wine)
    }
    if args.build_type is not None:
        cmake_defines["CMAKE_BUILD_TYPE"] = args.build_type

    cmake_args = (['cmake', 'build'] if args.build else ['cmake']) + [
        "cmake",
        "--preset", "conan-default",
    ] + [
        f"-D{key}={'ON' if value else 'OFF'}" if isinstance(value, bool) else f"-D{key}={value}"
        for key, value in cmake_defines.items()
    ]
    if args.build:
        cmake_args.append("build")

    print(f'Running CMake with args: {cmake_args}')
    subprocess.run(cmake_args, shell=True, check=True)

def main() -> None:
    if args.build_type is not None:
        run_conan(args.build_type)
    else:
        for build_type in ALL_BUILD_TYPES:
            run_conan(build_type)

    run_cmake()
    
if __name__ == "__main__":
    if not args.standalone and args.dump_hooks_only:
        AP.error("The --dump-hooks-only option can only be used with the --standalone option")
    try:
        main()
    except subprocess.CalledProcessError as e:
        print(f"Installation failed with error code {e.returncode}!")
        sys.exit(e.returncode)
    else:
        print("Installation is done!")
