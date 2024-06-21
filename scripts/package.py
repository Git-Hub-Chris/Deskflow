#!/usr/bin/env python3

import platform
from lib import env

# important: load venv before loading modules that install deps.
env.ensure_in_venv(__file__)

env_file = ".env"
package_filename_product = "synergy"


def main():
    env.ensure_module("dotenv", "python-dotenv")
    from dotenv import load_dotenv  # type: ignore

    load_dotenv(dotenv_path=env_file)

    major, minor, patch, stage, _build = env.get_version_info()
    version = f"{major}.{minor}.{patch}-{stage}"
    filename_base = get_filename_base(version)
    print(f"Package filename base: {filename_base}")

    if env.is_windows():
        windows_package(filename_base)
    elif env.is_mac():
        mac_package(filename_base)
    elif env.is_linux():
        linux_package(filename_base)
    else:
        raise RuntimeError(f"Unsupported platform: {env.get_os()}")


def get_filename_base(version):
    os = env.get_os()
    machine = platform.machine().lower()
    return f"{package_filename_product}-{version}-{os}-{machine}"


def windows_package(filename_base):
    """TODO: Windows packaging"""
    pass


def mac_package(filename_base):
    from lib import mac

    mac.package(filename_base)


def linux_package(filename_base):
    """TODO: Linux packaging"""
    pass


if __name__ == "__main__":
    main()
