param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$projectFile = Join-Path $projectRoot 'Gastro.uproject'
$enginePath = (Resolve-Path -LiteralPath $EngineRoot).Path
$buildTool = Join-Path $enginePath 'Engine/Build/BatchFiles/Build.bat'
$editorCmd = Join-Path $enginePath 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
if (!(Test-Path -LiteralPath $buildTool) -or !(Test-Path -LiteralPath $editorCmd)) {
    throw 'EngineRoot must name the UE installation directory, not its Engine subdirectory.'
}

$runDirectory = Join-Path $projectRoot ('Saved/FrameworkValidation/runs/' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
New-Item -ItemType Directory -Path $runDirectory -Force | Out-Null
$reportDirectory = Join-Path $runDirectory 'Reports'

Push-Location $projectRoot
try {
    if (!$SkipBuild) {
        $buildLog = Join-Path $runDirectory 'build.log'
        & $buildTool GastroEditor Win64 Development $projectFile -WaitMutex -NoHotReloadFromIDE -WarningsAsErrors -NoUBA 2>&1 |
            Tee-Object -FilePath $buildLog
        if ($LASTEXITCODE -ne 0) { throw "Development Editor build failed; see $buildLog" }
        if (Select-String -LiteralPath $buildLog -Pattern '\bwarning (C\d+|LNK\d+)|\bwarning:') {
            throw "Build emitted warnings; see $buildLog"
        }
    }

    $arguments = @(
        $projectFile, '-unattended', '-NullRHI', '-nosound', '-nop4', '-nosplash',
        '-ExecCmds=Automation RunTests GastroFramework.', '-TestExit=Automation Test Queue Empty',
        "-ReportExportPath=$reportDirectory", "-abslog=$(Join-Path $runDirectory 'automation.log')",
        '-stdout', '-FullStdOutLogOutput'
    )
    & $editorCmd @arguments 2>&1 | Tee-Object -FilePath (Join-Path $runDirectory 'automation-console.log')
    if ($LASTEXITCODE -ne 0) { throw "Automation process failed; see $runDirectory" }

    $reportFile = Join-Path $reportDirectory 'index.json'
    if (!(Test-Path -LiteralPath $reportFile)) { throw 'Automation produced no JSON report.' }
    $report = Get-Content -LiteralPath $reportFile -Raw -Encoding UTF8 | ConvertFrom-Json
    $testSources = Get-ChildItem -LiteralPath (Join-Path $projectRoot 'Plugins/GastroFramework/Source/GastroFrameworkTests/Private') -Filter '*.cpp'
    $expected = @($testSources | Select-String -Pattern '^\s*It\("').Count
    if ($expected -eq 0 -or $report.tests.Count -ne $expected -or $report.succeeded -ne $expected -or
        $report.failed -ne 0 -or $report.notRun -ne 0 -or $report.succeededWithWarnings -ne 0) {
        throw "Incomplete or failed suite: expected=$expected passed=$($report.succeeded) failed=$($report.failed) notRun=$($report.notRun). See $reportFile"
    }
    if (@($report.tests | Where-Object { $_.state -ne 'Success' }).Count -ne 0) {
        throw "Unexpected test state in $reportFile"
    }
    Write-Host "All $expected GastroFramework specs passed. Report: $reportFile"
}
finally {
    Pop-Location
}
