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

    def _upload_file(self, file_name):
        src = Path("guest_scripts") / file_name
        dst = self._workspace_path / file_name
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

    def upload_release(self):
        raise NotImplementedError()


class WindowsVm(Vm):
    def __init__(self, cmag_commit, cmag_version, workspace_name, chocolatey_key_path):
        super().__init__(cmag_commit, cmag_version, workspace_name, "windows10", "win64")
        self._chocolatey_key_path = chocolatey_key_path

        # Copy scripts to the VM
        self._upload_file("windows_build.ps1")
        self._upload_file("windows_provision.ps1")

    def compile(self):
        run_command(f'vagrant winrm {self._vm_name} --shell powershell --command "cd //VBOXSVR/workspace; ./windows_build.ps1 {self._cmag_commit} {self._cmag_version}"')

    def upload_release(self):
        raise NotImplementedError()


class UbuntuVm(Vm):
    def __init__(self, cmag_commit, cmag_version, workspace_name, gpg_key_id):
        super().__init__(cmag_commit, cmag_version, workspace_name, "ubuntu2204", "ubuntu2204")

        # Setup GPG key inside VM
        key_file_name = "key.gpg"
        run_command(f"gpg --output {self._workspace_path}/{key_file_name} --export-secret-key --armor '{gpg_key_id}'")
        run_command(f'vagrant ssh {self._vm_name} --command "gpg --import ~/workspace/{key_file_name}"')

        # Copy scripts and debian package metadata to the VM
        self._upload_file("ubuntu2204_provision.sh")
        self._upload_file("ubuntu2204_build.sh")
        self._upload_file("ubuntu_package.sh")
        self._upload_file("ubuntu_prepare_source_tarball.sh")
        self._upload_file("debian")

    def compile(self):
        run_command(f'vagrant ssh {self._vm_name} --command "cd ~/workspace; ./ubuntu2204_build.sh {self._cmag_commit} {self._cmag_version}"')

    def upload_release(self):
        distros = "focal jammy"
        run_command(f'vagrant ssh {self._vm_name} --command "cd ~/workspace; ./ubuntu2204_package.sh {self._cmag_version} \"{distros}\""')


try:
    commit_hash = sys.argv[1]
    version = sys.argv[2]
except IndexError:
    print("ERROR: Too few arguments!")
    print("Usage: deploy.py <COMMIT_HASH> <CMAG_VERSION>")
    sys.exit(1)

vms = [
    WindowsVm(commit_hash, version, "workspace_windows10", "/home/choco.key"),
    UbuntuVm(commit_hash, version, "workspace_ubuntu2204", 'dziuban.maciej@gmail.com'),
]

for vm in vms:
    try:
        print(f"Starting VM {vm.get_vm_name()}")
        vm.start_vm()

        print(f"Building in VM {vm.get_vm_name()}")
        vm.compile()

        print(f"Packaging and deploing in VM {vm.get_vm_name()}")
        vm.upload_release()

        print(f"Stopping VM {vm.get_vm_name()}")
        vm.stop_vm()
    except:
        print(f"Failed in {vm.get_vm_name()}")
        sys.exit(1)
