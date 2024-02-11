# Install VS workloads required for C++ development
# Workload IDs were taken from here https://learn.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-community?view=vs-2022&preserve-view=true
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe" modify     `
    --passive --norestart                                                         `
    --installPath "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community" `
    --add "Microsoft.VisualStudio.Component.Windows10SDK.20348"                   `
    --add "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"                     `
    --add "Microsoft.VisualStudio.Workload.NativeDesktop;includeOptional;includeRecommended"


# Install Chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
Import-Module "$env:ChocolateyInstall\helpers\chocolateyProfile.psm1"
choco feature enable -n=useRememberedArgumentsForUpgrades
Update-SessionEnvironment

# Install dependencies
choco install git -y
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=User' -y
