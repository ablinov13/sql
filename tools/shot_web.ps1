# Скриншот страницы сервера лабы 5 в окне Chrome (с адресной строкой).
# Сервер запускается в своём окне консоли, Chrome — с отдельным временным профилем, после снимка оба закрываются.
# Пример: powershell -File tools\shot_web.ps1 -Server "bin\Debug\lab5_3.exe" -Out screenshots\lab5_3.png [-ConsoleOut screenshots\lab5_3_console.png]
param(
    [Parameter(Mandatory = $true)][string]$Server,     # команда запуска сервера (exe или "node lab5\...js")
    [Parameter(Mandatory = $true)][string]$Out,
    [string]$ConsoleOut = "",
    [string]$Url = "http://localhost:3000/",
    [int]$Width = 1000,
    [int]$Height = 700,
    [double]$Scale = 1.0
)
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public class Win {
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h, int x, int y, int w, int hh, bool repaint);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  public struct RECT { public int Left, Top, Right, Bottom; }
}
"@
[Win]::SetProcessDPIAware() | Out-Null
$root = Resolve-Path "$PSScriptRoot\.."

function Save-Window([IntPtr]$h, [string]$path) {
    [Win]::SetForegroundWindow($h) | Out-Null
    Start-Sleep -Milliseconds 600
    $r = New-Object Win+RECT
    [Win]::GetWindowRect($h, [ref]$r) | Out-Null
    $w = $r.Right - $r.Left; $hgt = $r.Bottom - $r.Top
    $bmp = New-Object System.Drawing.Bitmap $w, $hgt
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    [Win]::PrintWindow($h, $hdc, 2) | Out-Null
    $g.ReleaseHdc($hdc); $g.Dispose()
    $crop = 7
    $cut = $bmp.Clone((New-Object System.Drawing.Rectangle $crop, 0, ($w - 2 * $crop), ($hgt - $crop)), $bmp.PixelFormat)
    New-Item -ItemType Directory -Force (Split-Path (Join-Path $root $path)) | Out-Null
    $cut.Save((Join-Path $root $path), [System.Drawing.Imaging.ImageFormat]::Png)
    $cut.Dispose(); $bmp.Dispose()
    "saved $path (${w}x${hgt})"
}

function Find-Window([string]$titleLike) {
    for ($i = 0; $i -lt 30; $i++) {
        $p = Get-Process | Where-Object { $_.MainWindowTitle -like $titleLike } | Select-Object -First 1
        if ($p) { return $p.MainWindowHandle }
        Start-Sleep -Milliseconds 300
    }
    throw "window not found: $titleLike"
}

# 1. сервер в своём окне консоли
$title = Join-Path $root ($Server -split " ")[0]
if ($Server -like "node *") { $title = $Server }
$inner = "chcp 65001 >nul & mode con: cols=80 lines=12 & title $title & cd /d `"$root`" & cls & $Server"
$con = Start-Process conhost.exe -ArgumentList "cmd.exe /k $inner" -PassThru
Start-Sleep -Seconds 2
$conWin = Find-Window "$title*"

# 2. Chrome с временным профилем (не трогает профиль пользователя)
$profile = Join-Path $root "obj\chrome-profile"
$chromeExe = "C:\Program Files\Google\Chrome\Application\chrome.exe"
$edge = Start-Process $chromeExe -ArgumentList "--user-data-dir=`"$profile`" --no-first-run --no-default-browser-check --disable-sync --force-device-scale-factor=$Scale --new-window $Url" -PassThru
Start-Sleep -Seconds 5
# окно ищем только у процесса Chrome с нашим временным профилем — чужие окна не трогаем
$edgeWin = [IntPtr]::Zero
for ($i = 0; $i -lt 30 -and $edgeWin -eq [IntPtr]::Zero; $i++) {
    $own = Get-CimInstance Win32_Process -Filter "Name='chrome.exe'" |
        Where-Object { $_.CommandLine -like "*obj\chrome-profile*" -and $_.CommandLine -notlike "*--type=*" }
    foreach ($o in $own) {
        $gp = Get-Process -Id $o.ProcessId -ErrorAction SilentlyContinue
        if ($gp -and $gp.MainWindowHandle -ne [IntPtr]::Zero) { $edgeWin = $gp.MainWindowHandle }
    }
    if ($edgeWin -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 300 }
}
if ($edgeWin -eq [IntPtr]::Zero) { throw "Chrome window (temp profile) not found" }
[Win]::ShowWindow($edgeWin, 9) | Out-Null
[Win]::MoveWindow($edgeWin, 60, 30, $Width, $Height, $true) | Out-Null
Start-Sleep -Seconds 1

try {
    Save-Window $edgeWin $Out
    if ($ConsoleOut) { Save-Window $conWin $ConsoleOut }
}
finally {
    # закрываем только запущенный здесь Chrome (по временному профилю), сервер и его консоль
    Get-CimInstance Win32_Process -Filter "Name='chrome.exe'" | Where-Object { $_.CommandLine -like "*obj\chrome-profile*" } |
        ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    Get-CimInstance Win32_Process -Filter "ParentProcessId=$($con.Id)" | ForEach-Object {
        Get-CimInstance Win32_Process -Filter "ParentProcessId=$($_.ProcessId)" | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
        Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue
    }
    Stop-Process $con -Force -ErrorAction SilentlyContinue
}
