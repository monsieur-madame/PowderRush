param(
    [switch]$OpenEditor,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

function Write-Step {
    param([string]$Message)
    Write-Host "[PowderRush Build] $Message" -ForegroundColor Cyan
}

function Resolve-EngineInstallDir {
    param([string]$EngineAssociation)

    $registryCandidates = @(
        "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\$EngineAssociation",
        "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\EpicGames\Unreal Engine\$EngineAssociation"
    )

    foreach ($keyPath in $registryCandidates) {
        if (Test-Path $keyPath) {
            $installedDirectory = (Get-ItemProperty -Path $keyPath -ErrorAction SilentlyContinue).InstalledDirectory
            if ($installedDirectory -and (Test-Path $installedDirectory)) {
                return $installedDirectory
            }
        }
    }

    $userBuildsKey = 'Registry::HKEY_CURRENT_USER\Software\Epic Games\Unreal Engine\Builds'
    if (Test-Path $userBuildsKey) {
        $userBuilds = Get-ItemProperty -Path $userBuildsKey
        $property = $userBuilds.PSObject.Properties | Where-Object { $_.Name -eq $EngineAssociation } | Select-Object -First 1
        if ($property -and $property.Value -and (Test-Path $property.Value)) {
            return $property.Value
        }
    }

    $defaultInstallDir = Join-Path $env:ProgramFiles "Epic Games\UE_$EngineAssociation"
    if (Test-Path $defaultInstallDir) {
        return $defaultInstallDir
    }

    return $null
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
$uprojectPath = Join-Path $projectRoot 'PowderRush.uproject'

if (-not (Test-Path $uprojectPath)) {
    throw "Could not find PowderRush.uproject at: $uprojectPath"
}

$projectData = Get-Content $uprojectPath -Raw | ConvertFrom-Json
$engineAssociation = [string]$projectData.EngineAssociation
if ([string]::IsNullOrWhiteSpace($engineAssociation)) {
    throw 'PowderRush.uproject is missing EngineAssociation.'
}

Write-Step "Project: $uprojectPath"
Write-Step "Engine Association: $engineAssociation"

$engineInstallDir = Resolve-EngineInstallDir -EngineAssociation $engineAssociation
if (-not $engineInstallDir) {
    throw "Unable to find an Unreal Engine installation for EngineAssociation '$engineAssociation'. Install UE $engineAssociation with Epic Games Launcher, then run this script again."
}

$buildBat = Join-Path $engineInstallDir 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path $buildBat)) {
    throw "Found Unreal Engine at '$engineInstallDir', but Build.bat was not found at '$buildBat'."
}

$buildArgs = @(
    'PowderRushEditor',
    'Win64',
    'Development',
    "-Project=$uprojectPath",
    '-WaitMutex',
    '-NoHotReloadFromIDE'
)

Write-Step "Using Unreal Engine: $engineInstallDir"
Write-Step "Build target: PowderRushEditor Win64 Development"

if ($DryRun) {
    Write-Step 'Dry run requested. Build command:'
    Write-Host "`"$buildBat`" $($buildArgs -join ' ')" -ForegroundColor Yellow
    exit 0
}

$process = Start-Process -FilePath $buildBat -ArgumentList $buildArgs -WorkingDirectory $projectRoot -NoNewWindow -Wait -PassThru
if ($process.ExitCode -ne 0) {
    throw "Unreal build failed with exit code $($process.ExitCode)."
}

Write-Step 'Build finished successfully.'

if ($OpenEditor) {
    Write-Step 'Opening PowderRush.uproject...'
    Start-Process -FilePath $uprojectPath | Out-Null
}