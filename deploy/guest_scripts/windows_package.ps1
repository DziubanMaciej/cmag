$cmagVersion=$args[0]
$apiKeyPath=$args[1]

echo "cmagVersion=$cmagVersion"
echo "apiKeyPath=$apiKeyPath"

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

# Prepare paths
$desktopPath = [Environment]::GetFolderPath("Desktop")
$workspacePath = "$desktopPath\workspace"
$cmagBinariesPath = "$workspacePath\cmag\build\bin\Release"
if (-Not (Test-Path -Path $cmagBinariesPath)) {
    Failure "detect folder with compiled cmag binaries"
}
$packagePath = "$workspacePath\chocolatey"
if (-Not (Test-Path -Path $packagePath)) {
    Failure "detect folder with chocolatey package"
}
cd $workspacePath
CheckFailure "cd to workspace"

# Copy binaries to package directory
Copy-Item -Path "$cmagBinariesPath\cmag.exe" -Destination "$packagePath\tools"
CheckFailure "copy cmag.exe"
Copy-Item -Path "$cmagBinariesPath\cmag_browser.exe" -Destination "$packagePath\tools"
CheckFailure "copy cmag_browser.exe"

# Insert version to .nuspec file
$nuspecPath = "$packagePath\cmag.nuspec"
$nuspecXmlContent = [xml](Get-Content $nuspecPath)
CheckFailure "read .nuspec file"
$nuspecXmlContent.package.metadata.version = $cmagVersion
$nuspecXmlContent.Save($nuspecPath)
CheckFailure "write .nuspec file"

# Insert hashes to VERIFICATION.txt file
function insert_hash_to_verification() {
    param ([string]$fileName)

    $hash = (Get-FileHash "$cmagBinariesPath\$fileName" -Algorithm SHA256).Hash
    CheckFailure "calculate hash for $fileName"

    $oldLine = "^" + [regex]::Escape($fileName) + ":.*"
    $newLine = "$fileName`: $hash"
    $verificationPath = "$packagePath\tools\VERIFICATION.txt"
    (Get-Content -Path $verificationPath) -replace $oldLine, $newLine | Set-Content -Path $verificationPath
    CheckFailure "insert hash for $fileName"
}
insert_hash_to_verification "cmag.exe"
insert_hash_to_verification "cmag_browser.exe"

# Build package
rm *.nupkg
CheckFailure "cleanup before building .nupkg package"
choco pack $nuspecPath
CheckFailure "build .nupkg package"
$nupkgPath = "cmag.$cmagVersion.nupkg"
if (-Not (Test-Path -Path $cmagBinariesPath)) {
    Failure "detect build .nupkg package file"
}

# Upload package
$apiKey = Get-Content -Path "$apiKeyPath" -Raw
choco push $nupkgPath --source="https://push.chocolatey.org/" --api-key=$apiKey
CheckFailure "push .nupkg package to Chocolatey repository"
