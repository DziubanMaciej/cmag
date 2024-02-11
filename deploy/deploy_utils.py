import subprocess
import shlex
import platform
import enum

_current_os = None

class OperatingSystem(enum.Enum):
    Windows = "Windows"
    Linux = "Linux"

    @staticmethod
    def current():
        global _current_os
        if _current_os is None:
            system = platform.system()
            try:
                _current_os = OperatingSystem(system)
            except ValueError as e:
                raise NotImplementedError(f"Unsupported OS: {system}")
        return _current_os

    def is_windows(self):
        return self == OperatingSystem.Windows

    def is_linux(self):
        return self == OperatingSystem.Linux


class CommandError(Exception):
    def __init__(self, output):
        self.stdout = self.stderr = None
        if output is not None:
            if output[0] is not None:
                self.stdout = output[0].decode("utf-8")
            if output[1] is not None:
                self.stderr = output[1].decode("utf-8")

    def __str__(self):
        print(f"stdout: {self.stdout}\n\nstderr: {self.stderr}")


class Stdin:
    def __init__(self, popen_arg, communicate_arg):
        self.popen_arg = popen_arg
        self.communicate_arg = communicate_arg

    @staticmethod
    def empty():
        return Stdin(None, None)

    @staticmethod
    def file(file_handle):
        return Stdin(file_handle, None)

    @staticmethod
    def string(content):
        return Stdin(subprocess.PIPE, bytes(content, "utf-8"))


class Stdout:
    def __init__(self, popen_arg, should_return):
        self.popen_arg = popen_arg
        self.should_return = should_return

    @staticmethod
    def ignore():
        return Stdout(subprocess.PIPE, False) # We could use DEVNULL, but we need the outputs to throw errors

    @staticmethod
    def return_back():
        return Stdout(subprocess.PIPE, True)

    @staticmethod
    def print_to_console():
        return Stdout(None, False)

    @staticmethod
    def print_to_file(file_handle):
        return Stdout(file_handle, False)


def run_command(command, *, shell=False, stdin=Stdin.empty(), stdout=Stdout.print_to_console(), stderr=Stdout.print_to_console()):
    if not shell and not OperatingSystem.current().is_windows():
        command = shlex.split(command)

    process = subprocess.Popen(command, shell=shell, stdin=stdin.popen_arg, stdout=stdout.popen_arg, stderr=stderr.popen_arg)
    output = process.communicate(input=stdin.communicate_arg)
    return_value = process.wait()

    if return_value != 0:
        raise CommandError(output)

    result = []
    if stdout.should_return and output[0] is not None:
        result.append(output[0].decode("utf-8"))
    if stderr.should_return and output[1] is not None:
        result.append(output[1].decode("utf-8"))
    if len(result) == 2:
        return tuple(result)
    if len(result) == 1:
        return result[0]
    return None
