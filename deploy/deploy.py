#!/bin/python
import os
import sys

from deploy_utils import run_command
from pathlib import Path
import shutil


class Vm:
    def __init__(self, cmag_commit, cmag_version, workspace_name, vm_name, archive_name_suffix):
        self._cmag_commit = cmag_commit
        self._cmag_version = cmag_version
        self._workspace_path = Path(workspace_name)
        self._vm_name = vm_name
        self._archive_name_suffix = archive_name_suffix

    def _upload_script(self, script_name):
        src = Path("guest_scripts") / script_name
        dst = self._workspace_path / script_name
        shutil.copy(src, dst)

    def start_vm(self):
        run_command(f"vagrant up {self._vm_name}")

    def stop_vm(self):
        run_command(f"vagrant halt {self._vm_name}")

    def get_vm_name(self):
        return self._vm_name

    def get_archive_name(self):
        return f"cmag-{self._cmag_version}-{self._archive_name_suffix}.zip"

    def compile(self):
        raise NotImplementedError()

    def get_binaries_paths(self):
        raise NotImplementedError()

    def upload_release(self):
        raise NotImplementedError()


class WindowsVm(Vm):
    def __init__(self, cmag_commit, cmag_version, workspace_name, chocolatey_key_path):
        super().__init__(cmag_commit, cmag_version, workspace_name, "windows10", "win64")
        self._chocolatey_key_path = chocolatey_key_path

    def compile(self):
        self._upload_script("windows_build.ps1")
        self._upload_script("windows_provision.ps1")
        run_command(f'vagrant winrm {self._vm_name} --shell powershell --command "cd //VBOXSVR/workspace; ./windows_build.ps1 {self._cmag_commit} {self._cmag_version}"')
        pass

    def get_binaries_paths(self):
        binary_dir = self._workspace_path / "cmag/build/bin/Release"
        return [
            binary_dir / "cmag.exe",
            binary_dir / "cmag_browser.exe",
        ]

    def upload_release(self):
        pass


class UbuntuVm(Vm):
    def __init__(self, cmag_commit, cmag_version, workspace_name):
        super().__init__(cmag_commit, cmag_version, workspace_name, "ubuntu2204", "ubuntu2204")

    def compile(self):
        self._upload_script("ubuntu2204_build.sh")
        self._upload_script("ubuntu2204_provision.sh")
        run_command(f'vagrant ssh {self._vm_name} --command "cd ~/workspace; ./ubuntu2204_build.sh {self._cmag_commit} {self._cmag_version}"')
        pass

    def get_binaries_paths(self):
        binary_dir = self._workspace_path / "cmag/build/bin"
        return [
            binary_dir / "cmag",
            binary_dir / "cmag_browser",
            ]

    def upload_release(self):
        pass


try:
    commit_hash = sys.argv[1]
    version = sys.argv[2]
except IndexError:
    print("ERROR: Too few arguments!")
    print("Usage: deploy.py <COMMIT_HASH> <CMAG_VERSION>")
    sys.exit(1)

vms = [
    WindowsVm(commit_hash, version, "workspace_windows10", "/home/choco.key"),
    UbuntuVm(commit_hash, version, "workspace_ubuntu2204"),
]

for vm in vms:
    try:
        print(f"Compiling {vm.get_vm_name()}")
        vm.start_vm()
        vm.compile()
        vm.stop_vm()
    except:
        print(f"Failed compiling {vm.get_vm_name()}")
        sys.exit(1)


for vm in vms:
    binaries = vm.get_binaries_paths()
    binaries = ' '.join([str(x) for x in binaries])
    zip_command = f"zip -j artifacts/{vm.get_archive_name()} {binaries}"
    run_command(zip_command)
