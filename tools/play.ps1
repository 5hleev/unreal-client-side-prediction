param(
    [Parameter(Mandatory=$true)][string]$EngineRoot,
    [ValidateSet('Clean','Lag100','Loss175')][string]$Profile = 'Lag100'
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$project = Join-Path $projectRoot 'PredictionLab.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (!(Test-Path -LiteralPath $editor)) { throw 'UnrealEditor.exe missing' }
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Content\Maps\PredictionArena.umap'))) { throw 'Run tools\build.ps1 first' }
# Outbound delay on BOTH processes: each network direction is delayed once.
$network = switch ($Profile) {
    'Clean' { 'NetEmulation.PktLag 0,NetEmulation.PktLagVariance 0,NetEmulation.PktLoss 0' }
    'Lag100' { 'NetEmulation.PktLag 100,NetEmulation.PktLagVariance 0,NetEmulation.PktLoss 0' }
    'Loss175' { 'NetEmulation.PktLag 175,NetEmulation.PktLagVariance 25,NetEmulation.PktLoss 5' }
}
$run = Join-Path $projectRoot ('Saved\Runs\' + (Get-Date -Format 'yyyyMMdd_HHmmss') + '_' + $Profile)
New-Item -ItemType Directory -Force $run | Out-Null
$quotedProject = '"' + $project + '"'
$shared = '-game -NoScreenMessages -windowed -ResX=960 -ResY=720 -nosound '
$exec = '-ExecCmds="' + $network + ',p.NetShowCorrections 1,p.NetCorrectionLifetime 8,log LogNetPlayerMovement Verbose"'
# These are interactive game windows, intentionally visible for the demonstration.
$hostArgs = "$quotedProject /Game/Maps/PredictionArena?listen $shared -port=7777 -WinX=0 -WinY=0 $exec -abslog=`"$run\host.log`""
$clientArgs = "$quotedProject 127.0.0.1:7777 $shared -WinX=970 -WinY=0 $exec -abslog=`"$run\client.log`""
Start-Process -FilePath $editor -ArgumentList $hostArgs
Start-Sleep -Seconds 8
Start-Process -FilePath $editor -ArgumentList $clientArgs
Write-Output "Logs: $run"
