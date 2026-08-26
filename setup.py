from argparse import ArgumentParser
import subprocess

ap = ArgumentParser(description="Setup script for the project")
#ap.add_argument('-o', '--options', dest='options', default=[], nargs='+', help='Options passed to Conan (See `conanfile` for available options)')
#ap.add_argument('-s', '--settings', dest='settings', default=[], nargs='+', help='Settings passed to Conan (See `conanfile` for available settings)')
ap.add_argument('-p', '--profile', dest='profile', default='conanprofile.txt', help='Conan profile to use for the build')
ap.add_argument('-b', '--build', dest='build', action='store_true', help='Build the project after setup')

def main() -> None:
    args, conan_args = ap.parse_known_args()

    #options = [item for opt in args.options for item in ('-o', opt)] if args.options else []
    #settings = [item for setting in args.settings for item in ('-s', setting)] if args.settings else []

    subprocess.run(["conan", "install", ".", "--build=missing", "--profile", args.profile, *conan_args], check=True)
    if args.build:
        subprocess.run(["conan", "build", ".", "--profile", args.profile, *conan_args], check=True)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error: {e}")
        exit(1)
