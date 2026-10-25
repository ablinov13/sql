# Сборка решения из командной строки: powershell -File tools\build.ps1 [-Config Debug|Release] [-Target lab2]
param([string]$Config = "Debug", [string]$Target = "")
$msbuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
$args = @("$PSScriptRoot\..\DbLabs.sln", "/m", "/nologo", "/v:minimal", "/p:Configuration=$Config", "/p:Platform=x64")
if ($Target) { $args += "/t:$Target" }
& $msbuild @args
exit $LASTEXITCODE
