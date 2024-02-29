#!/bin/python
import os
import sys

from deploy_utils import run_command, Stdout, CommandError
from pathlib import Path
import shutil


class Vm:
    def __init__(self, cmag_commit, cmag_version, workspace_name, vm_name):
        self._cmag_commit = cmag_commit
        self._cmag_version = cmag_version
        self._workspace_path = Path(workspace_name)
        self._vm_name = vm_name
        self._success = False

    def _upload_file(self, file_name):
        src = Path("guest_scripts") / file_name
        dst = self._workspace_path / file_name

        if src.is_dir():
            shutil.copytree(src, dst, dirs_exist_ok=True)
        else:
            shutil.copy(src, dst)

    def start_vm(self):
        run_command(f"vagrant up {self._vm_name}")

    def stop_vm(self):
        run_command(f"vagrant halt {self._vm_name}")

    def set_success(self):
        self._success = True

    def get_success(self):
        return self._success

    def get_vm_name(self):
        return self._vm_name

    def compile(self):
        raise NotImplementedError()

    def upload_release(self):
        raise NotImplementedError()

    def get_release_asset(self):
        return None


class WindowsVm(Vm):
    def __init__(self, cmag_commit, cmag_version, workspace_name, chocolatey_key_path):
        super().__init__(cmag_commit, cmag_version, workspace_name, "windows10")
        self._chocolatey_key_path = chocolatey_key_path

        self._upload_file("windows_build.ps1")
        self._upload_file("windows_provision.ps1")

    def compile(self):
        run_command(f'vagrant winrm {self._vm_name} --shell powershell --command "cd //VBOXSVR/workspace; ./windows_build.ps1 {self._cmag_commit} {self._cmag_version}"')

    def upload_release(self):
        raise NotImplementedError()

    def get_release_asset(self):
        binary_dir = self._workspace_path / "cmag/build/bin/Release"
        return {
            "name": f"cmag-{self._cmag_version}-win64.zip",
            "files": [
                binary_dir / "cmag.exe",
                binary_dir / "cmag_browser.exe",
            ]
        }


class UbuntuVm(Vm):
    def __init__(self, cmag_commit, cmag_version, workspace_name, gpg_key_id):
        super().__init__(cmag_commit, cmag_version, workspace_name, "ubuntu2204")
        self._key_file_name = "key.gpg"

        run_command(f"gpg --output {self._workspace_path}/{self._key_file_name} --export-secret-key --armor --yes '{gpg_key_id}'")
        self._upload_file("ubuntu_provision.sh")
        self._upload_file("ubuntu_build.sh")
        self._upload_file("ubuntu_package.sh")
        self._upload_file("ubuntu_prepare_source_tarball.sh")
        self._upload_file("debian")

    def compile(self):
        run_command(f'vagrant ssh {self._vm_name} --command "cd ~/workspace; ./ubuntu_build.sh {self._cmag_commit} {self._cmag_version}"')

    def upload_release(self):
        run_command(f'vagrant ssh {self._vm_name} --command "gpg --import ~/workspace/{self._key_file_name}"')

        distros = "focal jammy"
        run_command(f'vagrant ssh {self._vm_name} --command "cd ~/workspace; ./ubuntu_package.sh {self._cmag_version} \'{distros}\'"')


# Parse arguments
try:
    version = sys.argv[1]
except IndexError:
    print("ERROR: Too few arguments!")
    print("Usage: deploy.py <CMAG_VERSION>")
    sys.exit(1)
try:
    commit_hash = run_command(f"git rev-parse v{version}", stdout=Stdout.return_back(), stderr=Stdout.ignore())
    commit_hash = commit_hash.strip()
except CommandError:
    print(f"ERROR: Failed to get git revision for version {version}")
    sys.exit(1)

# Prepare VMs
vms = [
    WindowsVm(commit_hash, version, "workspace_windows10", "/home/choco.key"),
    UbuntuVm(commit_hash, version, "workspace_ubuntu2204", 'dziuban.maciej@gmail.com'),
]

# Compile on all VMs and upload to package repositories.
for vm in vms:
    try:
        print(f"Starting VM {vm.get_vm_name()}")
        vm.start_vm()

        print(f"Building in VM {vm.get_vm_name()}")
        vm.compile()

        print(f"Packaging and deploying in VM {vm.get_vm_name()}")
        vm.upload_release()

        print(f"Stopping VM {vm.get_vm_name()}")
        vm.stop_vm()
        vm.set_success()
    except:
        print(f"Failed in {vm.get_vm_name()}")

# Gather files to upload to the GitHub release
for vm in vms:
    asset = vm.get_release_asset()
    if asset is None:
        continue
    binaries = ' '.join([str(x) for x in asset['files']])
    zip_command = f"zip -j assets/{asset['name']} {binaries}"  # TODO won't work on Windows
    run_command(zip_command)

# Report results
print(f"Executed deploy routine for {len(vms)} VMs.")
for vm in vms:
    success_string = "SUCCESS" if vm.get_success() else "FAIL"
    print(f"\t{vm.get_vm_name()}: {success_string}")
