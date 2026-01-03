param (
    [string]$Arg1,
    [string]$Arg2
)

$ErrorActionPreference = "Stop"

# Paths
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDir   = Join-Path $ProjectRoot "GameOfLife"
$BuildDir    = Join-Path $ProjectRoot "builds"

# Helper: usage
function Show-Usage {
    Write-Host "Usage:" -ForegroundColor Cyan
    Write-Host "  .\run.ps1 mconsole [ExecutableName]"
    Write-Host "  .\run.ps1 mwindows [ExecutableName]"
    Write-Host "  .\run.ps1 ExecutableName"
}

# ----------------------------------------
# Case 1: No arguments
# ----------------------------------------
if (-not $Arg1) {
    Write-Host "No arguments provided." -ForegroundColor Yellow
    Show-Usage
    exit 1
}

# ----------------------------------------
# Case 2: Run-only mode (no mconsole/mwindows)
# ----------------------------------------
if ($Arg1 -notin @("mconsole", "mwindows")) {

    $RunExe = if ($Arg1 -notmatch '\.exe$') { "$Arg1.exe" } else { $Arg1 }
    $ExePath = Join-Path $BuildDir $RunExe

    if (Test-Path $ExePath) {
        Write-Host "Running $RunExe from builds..." -ForegroundColor Cyan
        & $ExePath
        exit 0
    }
    else {
        Write-Host "Executable '$RunExe' not found in builds." -ForegroundColor Red
        exit 1
    }
}

# ----------------------------------------
# Case 3: Compile mode
# ----------------------------------------
$Mode   = $Arg1
$RunExe = if ($Arg2 -and ($Arg2 -notmatch '\.exe$')) { "$Arg2.exe" } else { $Arg2 }
$SubsystemFlag = "-$Mode"

# Ensure builds directory exists
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Find source files
$SourceFiles = Get-ChildItem -Path $SourceDir -Recurse -Include *.c, *.cpp

if ($SourceFiles.Count -eq 0) {
    Write-Host "No C/C++ source files found in GameOfLife." -ForegroundColor Red
    exit 1
}

Write-Host "Compiling with $SubsystemFlag..." -ForegroundColor Cyan

foreach ($File in $SourceFiles) {
    $ExeName   = [System.IO.Path]::GetFileNameWithoutExtension($File.Name) + ".exe"
    $OutputExe = Join-Path $BuildDir $ExeName

    Write-Host " -> $($File.Name)" -ForegroundColor Yellow

    g++ $SubsystemFlag `
        "$($File.FullName)" `
        -I"$ProjectRoot\includes" `
        -o "$OutputExe"

    if ($LASTEXITCODE -ne 0) {
        Write-Host "Compilation failed for $($File.Name)" -ForegroundColor Red
        exit 1
    }
}

Write-Host "Compilation complete. Executables are in 'builds'." -ForegroundColor Green

# ----------------------------------------
# Optional run after compile
# ----------------------------------------
if ($RunExe) {
    $ExePath = Join-Path $BuildDir $RunExe

    if (Test-Path $ExePath) {
        Write-Host "Running $RunExe..." -ForegroundColor Cyan
        & $ExePath
    }
    else {
        Write-Host "Executable '$RunExe' not found in builds." -ForegroundColor Red
        exit 1
    }
}
