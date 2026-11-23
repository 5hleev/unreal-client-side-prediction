param([ValidateSet('Clean','Lag100','Loss175')][string]$Profile = 'Lag100')
$ErrorActionPreference = 'Stop'
$exe = Join-Path $PSScriptRoot 'PredictionLab.exe'
if (!(Test-Path -LiteralPath $exe)) { throw 'Keep this script beside PredictionLab.exe in the packaged Windows folder' }
$settings = switch ($Profile) {
    'Clean' { 'NetEmulation.PktLag 0,NetEmulation.PktLagVariance 0,NetEmulation.PktLoss 0' }
    'Lag100' { 'NetEmulation.PktLag 100,NetEmulation.PktLagVariance 0,NetEmulation.PktLoss 0' }
    'Loss175' { 'NetEmulation.PktLag 175,NetEmulation.PktLagVariance 25,NetEmulation.PktLoss 5' }
}
$exec = '-ExecCmds="' + $settings + ',p.NetShowCorrections 1,p.NetCorrectionLifetime 8"'
# Visible windows are intentional: these are the two interactive players.
Start-Process -FilePath $exe -ArgumentList "/Game/Maps/PredictionArena?listen -windowed -ResX=900 -ResY=650 -WinX=0 -WinY=0 -port=7777 $exec"
Start-Sleep -Seconds 8
Start-Process -FilePath $exe -ArgumentList "127.0.0.1:7777 -windowed -ResX=900 -ResY=650 -WinX=920 -WinY=0 $exec"
