# Initialize build environment
Write-Host 'Update dependencies...' -ForegroundColor Magenta
.\update_dependencies.ps1
.\Tools\nuget.exe restore .\Unpaint.sln

Write-Host 'Finding Visual Studio...' -ForegroundColor Magenta
$vsPath = .\Tools\vswhere.exe -latest -property installationPath
Write-Host $vsPath

Write-Host 'Importing environment variables...' -ForegroundColor Magenta
cmd.exe /c "call `"$vsPath\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"
Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
  if ($_ -match "^(.*?)=(.*)$") {
      Set-Content "env:\$($matches[1])" $matches[2]
  }
}

# Build projects
Write-Host 'Patching appx manifest...' -ForegroundColor Magenta
$version = if ($null -ne $env:APPVEYOR_BUILD_VERSION) { $env:APPVEYOR_BUILD_VERSION } else { "1.0.0.0" }
[XML]$appManifest = Get-Content "$PSScriptRoot/Unpaint/Package.appxmanifest"
$appManifest.Package.Identity.Version = $version
$appManifest.Save("$PSScriptRoot/Unpaint/Package.appxmanifest")

certutil -importpfx -p unpaint -f -user .\Unpaint\key.pfx NoRoot
$coreCount = (Get-CimInstance -class Win32_ComputerSystem).NumberOfLogicalProcessors
$configurations = "Release"
$platforms = "x64"

foreach ($platform in $platforms) {
  foreach ($config in $configurations) {
    Write-Host "Building $platform $config..." -ForegroundColor Magenta
    MSBuild.exe .\Unpaint.sln -p:Configuration=$config -p:Platform=$platform -m:$coreCount -v:m
  }
}

# Pack app
Write-Host 'Creating output directory...' -ForegroundColor Magenta
$packagePath = ".\Unpaint\bin\Release\x64\Unpaint_$($version)_x64.msix"
signtool.exe sign /fd SHA256 /td SHA256 /a /f "./Unpaint/key.pfx" /p unpaint /tr http://timestamp.digicert.com $packagePath