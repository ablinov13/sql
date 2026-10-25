# Открывает всё для показа лаб: отчёты в Word, решение в Visual Studio, DB Browser for SQLite, шпаргалку.
# Запуск из папки sql: powershell -ExecutionPolicy Bypass -File tools\demo.ps1
$root = Resolve-Path "$PSScriptRoot\.."

Get-ChildItem (Join-Path $root "reports") -Filter "*.docx" | Where-Object { $_.Name -notlike "~*" } |
    Sort-Object Name | ForEach-Object { Start-Process $_.FullName; Start-Sleep -Seconds 2 }

$devenv = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe"
if (Test-Path $devenv) { Start-Process $devenv -ArgumentList "`"$(Join-Path $root 'DbLabs.sln')`"" }

$db4s = "C:\Program Files\DB Browser for SQLite\DB Browser for SQLite.exe"
if (Test-Path $db4s) { Start-Process $db4s -WorkingDirectory $root }

Start-Process notepad.exe -ArgumentList "`"$(Join-Path $root 'ДЕМО.md')`""
"Открыто: отчёты, Visual Studio, DB Browser, шпаргалка ДЕМО.md"
