param(
    [string]$JdkHome = 'C:\Program Files\Eclipse Adoptium\jdk-17.0.19.10-hotspot',
    [string]$AgentName = 'lunar_unlock_agent.dll'
)

$ErrorActionPreference = 'Stop'
$vs = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
if (!(Test-Path $vs)) { throw "VsDevCmd.bat not found: $vs" }
if (!(Test-Path (Join-Path $JdkHome 'include\jni.h'))) {
    throw "JDK headers not found: $JdkHome"
}

$root = $PSScriptRoot
$bin = Join-Path $root 'bin'
$dist = Join-Path $root 'dist'
New-Item -ItemType Directory -Force -Path $bin | Out-Null
New-Item -ItemType Directory -Force -Path $dist | Out-Null

$command = @(
    "call `"$vs`" -arch=x64"
    "pushd `"$root`""
    "cl /nologo /utf-8 /std:c++20 /EHsc /LD agent.cpp /I`"$JdkHome\include`" /I`"$JdkHome\include\win32`" /link /OUT:bin\$AgentName"
    "cl /nologo /utf-8 /std:c++20 /EHsc injector.cpp /link /SUBSYSTEM:CONSOLE user32.lib psapi.lib /OUT:bin\LunarUnlockInjector.exe"
    "rc /nologo /fo bin\injector_ui.res injector_ui.rc"
    "cl /nologo /utf-8 /std:c++20 /EHsc injector_ui.cpp bin\injector_ui.res /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib psapi.lib shell32.lib dwmapi.lib /OUT:bin\LunarUnlockUI.exe"
    "popd"
) -join ' && '

cmd /c $command
if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }

Write-Host "Built $bin\$AgentName"
Write-Host "Built $bin\LunarUnlockInjector.exe"
Write-Host "Built $bin\LunarUnlockUI.exe"

if ($AgentName -eq 'lunar_unlock_agent.dll') {
    Copy-Item -LiteralPath (Join-Path $bin 'LunarUnlockUI.exe') `
        -Destination (Join-Path $dist 'LunarUnlockUI.exe') -Force
    Write-Host "Packaged $dist\LunarUnlockUI.exe"
}
