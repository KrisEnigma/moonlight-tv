# Build CTM-USBIP and copy Release/Debug output to dist/CTM-USBIP/
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$CtmRoot = Join-Path $ProjectRoot 'CTM-USBIP'

if (-not (Test-Path (Join-Path $CtmRoot 'build.ps1'))) {
    Write-Error "CTM-USBIP submodule missing. Run: git submodule update --init CTM-USBIP"
}

Push-Location $CtmRoot
try {
    & .\build.ps1 -Configuration $Configuration
} finally {
    Pop-Location
}

$out = Join-Path $CtmRoot "out\x64\$Configuration"
$dist = Join-Path $ProjectRoot 'dist\CTM-USBIP'
New-Item -ItemType Directory -Force -Path $dist | Out-Null
Copy-Item -Force -Path (Join-Path $out '*') -Destination $dist -Recurse
Write-Host "Copied CTM-USBIP build to $dist"
