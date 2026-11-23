param([Parameter(Mandatory=$true)][string]$EngineRoot)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$project = Join-Path $projectRoot 'PredictionLab.uproject'
$build = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (!(Test-Path -LiteralPath $build) -or !(Test-Path -LiteralPath $editor)) { throw 'Invalid Unreal Engine root' }
& $build PredictionLabEditor Win64 Development $project -WaitMutex
if ($LASTEXITCODE -ne 0) { throw 'Unreal build failed' }
$arenaScript = Join-Path $PSScriptRoot 'create_arena.py'
& $editor $project '-unattended' '-nullrhi' "-ExecutePythonScript=$arenaScript" '-stdout' '-FullStdOutLogOutput'
if ($LASTEXITCODE -ne 0) { throw 'Arena generation failed; inspect Saved\Logs' }
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Content\Maps\PredictionArena.umap'))) { throw 'Arena map missing' }
Write-Output 'Build and map generation finished.'
