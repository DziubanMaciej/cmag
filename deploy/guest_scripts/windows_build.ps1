$cmag_commit=$args[0]
$expected_cmag_version=$args[1]

echo "cmag_commit=$cmag_commit"
echo "expected_cmag_version=$expected_cmag_version"

function Failure {
    param (
        [string]$ErrorMessage
    )
    Write-Error "ERROR: failed to $ErrorMessage"
    exit 1
}

function CheckFailure {
    param (
        [string]$ErrorMessage
    )

    if (-Not $?) {
        Failure $ErrorMessage
    }
}

# VirtualBox uses UNC path for shared folders. This may upset some software, so we have to make workarounds.
# For git, we have to add an exception for this directory to be able to initialize submodules.
# For cmd (used by CMake custom commands) we have to execute in a non-UNC working directory. Hence, we create a symlink
# and cd to it.
git config --global --add safe.directory '%(prefix)///VBOXSVR/workspace/cmag'
$desktopPath = [Environment]::GetFolderPath("Desktop")
New-Item -Path $desktopPath\workspace -Type SymbolicLink -Value "\\VBOXSVR\workspace" -Force >$null
cd $desktopPath\workspace

# Clone the repository
if (-Not (Test-Path -Path cmag)) {
    git clone https://github.com/DziubanMaciej/cmag
    CheckFailure "clone"
    cd cmag
} else {
    cd cmag
    git fetch
    CheckFailure "fetch"
}
git checkout $cmag_commit -q
CheckFailure "checkout"
git submodule update --init --recursive
CheckFailure "download submodules"

# Configure CMake
mkdir build -Force >$null
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAG_BUILD_TESTS=OFF -DCMAG_BUILD_BROWSER=ON
CheckFailure "run CMake"

# Compile
cmake --build . --config Release
CheckFailure "compile"

# Self run
bin/Release/cmag.exe cmake ..
CheckFailure "perform a self cmag run"

# Verify version
$actual_cmag_version = bin/Release/cmag.exe -v
if($actual_cmag_version -ne $expected_cmag_version) {
    echo "ERROR: version mismatch"
    exit 4
}
