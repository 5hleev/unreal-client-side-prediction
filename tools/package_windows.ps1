param([string]$EngineRoot = 'F:\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$project = Join-Path $projectRoot 'PredictionLab.uproject'
$uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$output = Join-Path $projectRoot 'dist'
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Content\Maps\PredictionArena.umap'))) { throw 'Run tools/build.ps1 first' }
& $uat BuildCookRun "-project=$project" -noP4 -platform=Win64 -clientconfig=Development -build -cook -map=/Game/Maps/PredictionArena -stage -pak -archive "-archivedirectory=$output" -prereqs -utf8output '-UbtArgs=-NoUBA -MaxParallelActions=4'
if ($LASTEXITCODE -ne 0) { throw 'Windows packaging failed' }
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'launch_portable.ps1') -Destination (Join-Path $output 'Windows\launch_portable.ps1')
foreach ($profile in @('Clean', 'Lag100', 'Loss175')) {
    $launcher = '@echo off' + "`r`n" + 'powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0launch_portable.ps1" -Profile ' + $profile + "`r`n" + 'pause' + "`r`n"
    Set-Content -LiteralPath (Join-Path $output "Windows\Start_$profile.bat") -Value $launcher -Encoding ASCII
}
Write-Output "Portable build: $output\Windows"
