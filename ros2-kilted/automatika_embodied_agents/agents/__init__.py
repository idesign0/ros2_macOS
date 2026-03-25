import sys
import os
from importlib.metadata import version, PackageNotFoundError
from packaging import version as pkg_version

# Define the minimum required version of sugarcoat
MIN_SUGARCOAT_VERSION = "0.5.0"


def _print_sugarcoat_error(current_version=None):
    """
    Helper to print the standard error message for missing/incompatible sugarcoat.
    """
    package_name = "automatika-ros-sugar"

    print("\n" + "=" * 60)
    if current_version:
        print(f"❌ CRITICAL ERROR: Incompatible '{package_name}' version found.")
        print("=" * 60)
        print(f"EmbodiedAgents requires {package_name} >= {MIN_SUGARCOAT_VERSION}")
        print(f"Found version: {current_version}")
    else:
        print(f"❌ CRITICAL ERROR: '{package_name}' not found.")
        print("=" * 60)
        print(f"EmbodiedAgents depends on {package_name} (Sugarcoat).")

    print("\nPLEASE INSTALL THE CORRECT VERSION:")
    print("-" * 60)
    print("OPTION 1: Using your Package Manager (Recommended for Ubuntu):")
    print("  sudo apt install ros-$ROS_DISTRO-automatika-ros-sugar")
    print("-" * 60)
    print("OPTION 2: Build from Source (If unsure, use this):")
    print("  mkdir -p ros-sugar-ws/src")
    print("  cd ros-sugar-ws/src")
    print("  git clone https://github.com/automatika-robotics/sugarcoat && cd ..")
    print("  # Install dependencies (ensure attrs>=23.2.0 is included)")
    print(
        "  pip install numpy opencv-python-headless 'attrs>=23.2.0' jinja2 msgpack msgpack-numpy setproctitle pyyaml toml"
    )
    print("  colcon build")
    print("  source install/setup.bash")
    print("-" * 60)
    print(
        "Check the installation docs: https://automatika-robotics.github.io/sugarcoat/installation.html"
    )
    print("=" * 60 + "\n")

    # Force exit to prevent runtime errors
    sys.exit(1)


def check_sugarcoat_version():
    """
    Verifies that the installed version of automatika-ros-sugar (sugarcoat)
    meets the minimum requirement for this release of EmbodiedAgents.
    """
    package_name = "automatika-ros-sugar"

    try:
        installed_ver_str = version(package_name)

        # Check if installed version is less than minimum required
        if pkg_version.parse(installed_ver_str) < pkg_version.parse(
            MIN_SUGARCOAT_VERSION
        ):
            _print_sugarcoat_error(current_version=installed_ver_str)

    except PackageNotFoundError:
        _print_sugarcoat_error(current_version=None)


# Check if we are running inside a documentation build
_IS_DOCS_BUILD = os.environ.get("AGENTS_DOCS_BUILD") == "1"

if not _IS_DOCS_BUILD:
    check_sugarcoat_version()
