import ctypes
import sys
import os
import xml.etree.ElementTree as ET
from lib import cmd_utils

cmake_env_var = "CMAKE_PREFIX_PATH"


class EnvError(Exception):
    pass


def relaunch_as_admin(script):
    args = " ".join(sys.argv[1:])
    command = f"{script} --pause-on-exit {args}"
    print(f"Re-launching script as admin: {command}")
    ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, command, None, 1)


def is_admin():
    """Returns True if the current process has admin privileges."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except ctypes.WinError:
        return False


def set_env_var(name, value):
    """
    Sets or updates an environment variable. Appends the value if it doesn't already exist.

    Args:
    name (str): The name of the environment variable.
    value (str): The value of the environment variable.
    """

    current_value = os.getenv(name, "")

    if value not in current_value:
        new_value = f"{current_value}{os.pathsep}{value}" if current_value else value
        os.environ[name] = new_value
        print(f"Setting environment variable: {name}={value}")
        cmd_utils.run(["setx", name, new_value], check=True)


class WindowsChoco:
    """Chocolatey for Windows."""

    def install(self, command, ci_env):
        """Installs packages using Chocolatey."""
        if ci_env:
            # don't show noisy choco progress bars in ci env
            cmd_utils.run(f"{command} --no-progress")
        else:
            cmd_utils.run("winget install chocolatey", check=False)
            cmd_utils.run(command)

    def config_ci_cache(self):
        """Configures Chocolatey cache for CI."""

        runner_temp_key = "RUNNER_TEMP"
        runner_temp = os.environ.get(runner_temp_key)
        if runner_temp:
            # sets the choco cache dir, which should match the dir in the ci cache action.
            key_arg = '--name="cacheLocation"'
            value_arg = f'--value="{runner_temp}/choco"'
            cmd_utils.run(["choco", "config", "set", key_arg, value_arg])
        else:
            print(f"Warning: CI environment variable {runner_temp_key} not set")

    def remove_from_config(self, config_file, remove_packages):
        """Removes a package from the Chocolatey configuration."""

        tree = ET.parse(config_file)
        root = tree.getroot()
        for remove in remove_packages:
            for package in root.findall("package"):
                if package.get("id") == remove:
                    root.remove(package)
                    print(f"Removed package from choco config: {remove}")

        tree.write(config_file)


class WindowsQt:
    """Qt for Windows."""

    def __init__(self, config, config_file):
        self.config = config
        self.config_file = config_file

        self.version = os.environ.get("QT_VERSION")
        if not self.version:
            try:
                default_version = config["version"]
            except KeyError:
                raise EnvError(f"Qt version not set in {config_file}")

            print(f"QT_VERSION not set, using: {default_version}")
            self.version = default_version

        self.base_dir = os.environ.get("QT_BASE_DIR")
        if not self.base_dir:
            try:
                default_base_dir = config["install-dir"]
            except KeyError:
                raise EnvError(f"Qt install-dir not set in {config_file}")

            print(f"QT_BASE_DIR not set, using: {default_base_dir}")
            self.base_dir = default_base_dir

        self.install_dir = f"{self.base_dir}\\{self.version}"

    def get_install_dir(self):
        if os.path.isdir(self.install_dir):
            return self.install_dir

    def install(self):
        """Installs Qt on Windows."""

        cmd_utils.run(["pip", "install", "aqtinstall"])

        try:
            mirror_url = self.config["mirror"]
        except KeyError:
            raise EnvError(f"Qt mirror not set in {self.config_file}")

        args = ["python", "-m", "aqt", "install-qt"]
        args.extend(["--outputdir", self.base_dir])
        args.extend(["--base", mirror_url])
        args.extend(["windows", "desktop", self.version, "win64_msvc2019_64"])
        cmd_utils.run(args)

        install_dir = self.get_install_dir()
        if not install_dir:
            raise EnvError(f"Qt not installed, path not found: {install_dir}")

    def set_env_vars(self):
        set_env_var(cmake_env_var, f"{self.get_install_dir()}\\msvc2019_64")
