param(
    [string]$EngineRoot = 'F:\Epic Games\UE_5.8',
    [string[]]$Profiles = @('Clean','Lag100','Loss175'),
    [switch]$Render,
    [switch]$Portable
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$project = Join-Path $projectRoot 'PredictionLab.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$projectArgument = '"' + $project + '" '
if ($Portable) {
    $editor = Join-Path $projectRoot 'dist\Windows\PredictionLab\Binaries\Win64\PredictionLab.exe'
    $projectArgument = ''
}
if (!(Test-Path -LiteralPath $editor)) { throw "Executable missing: $editor" }
$runRoot = Join-Path $projectRoot ('docs\media\run_' + (Get-Date -Format 'yyyyMMdd_HHmmss'))
New-Item -ItemType Directory -Force $runRoot | Out-Null
foreach ($profile in $Profiles) {
    $network = switch ($profile) {
        'Clean' { 'NetEmulation.PktLag 0,NetEmulation.PktLagVariance 0,NetEmulation.PktLoss 0' }
        'Lag100' { 'NetEmulation.PktLag 100,NetEmulation.PktLagVariance 0,NetEmulation.PktLoss 0' }
        'Loss175' { 'NetEmulation.PktLag 175,NetEmulation.PktLagVariance 25,NetEmulation.PktLoss 5' }
        default { throw "Unknown profile: $profile" }
    }
    $run = Join-Path $runRoot $profile
    New-Item -ItemType Directory -Force $run | Out-Null
    $shared = '-game -unattended -nosound -nosplash -NoLoadingScreen -NoScreenMessages -ResX=1280 -ResY=720 '
    $exec = '-ExecCmds="' + $network + ',p.NetShowCorrections 1,p.NetCorrectionLifetime 8,log LogNetPlayerMovement Verbose,t.MaxFPS 60"'
    $hostArgs = $projectArgument + '/Game/Maps/PredictionArena?listen ' + $shared + ' -nullrhi -port=7777 ' + $exec + ' -abslog="' + $run + '\host.log"'
    $clientRhi = if ($Render) { '-RenderOffscreen' } else { '-nullrhi' }
    $clientArgs = $projectArgument + '127.0.0.1:7777 ' + $shared + ' ' + $clientRhi + ' -LabAuto -LabProfile=' + $profile + ' ' + $exec + ' -LabMedia="' + $run + '" -abslog="' + $run + '\client.log"'
    $hostProcess = $null
    $clientProcess = $null
    try {
        Write-Output "Starting $profile (two real Unreal processes, Render=$Render)"
        $hostProcess = Start-Process -FilePath $editor -ArgumentList $hostArgs -WindowStyle Hidden -PassThru
        Start-Sleep -Seconds 12
        if ($hostProcess.HasExited) { throw "Host exited: $run\host.log" }
        $clientProcess = Start-Process -FilePath $editor -ArgumentList $clientArgs -WindowStyle Hidden -PassThru
        if (!$clientProcess.WaitForExit(240000)) { throw "Client timeout: $run\client.log" }
        if ($clientProcess.ExitCode -ne 0) { throw "Client failed: $run\client.log" }
        $log = Get-Content -LiteralPath (Join-Path $run 'client.log') -Raw
        if ($log -notmatch 'LAB event=auto_complete') { throw "Automatic scenario did not complete: $run" }
        Write-Output "Completed $profile : $run"
    } finally {
        foreach ($process in @($clientProcess, $hostProcess)) {
            if ($null -ne $process -and !$process.HasExited) { Stop-Process -Id $process.Id }
        }
    }
}
Write-Output "Raw experiment logs and screenshots: $runRoot"
