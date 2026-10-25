# Снимок настоящего окна консоли с выводом команды (для отчётов).
# Пример: powershell -File tools\shot.ps1 -Command "bin\Debug\lab2.exe" -Out screenshots\lab2.png -Lines 40
param(
    [Parameter(Mandatory = $true)][string]$Command,
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$Cols = 100,
    [int]$Lines = 40,
    [int]$WaitSeconds = 3,
    [string]$Title = ""
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
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  public struct RECT { public int Left, Top, Right, Bottom; }
}
"@
[Win]::SetProcessDPIAware() | Out-Null

$root = Resolve-Path "$PSScriptRoot\.."
# заголовок окна: как у VS при Ctrl+F5 — полный путь к программе (по нему же ищем окно)
$title = if ($Title) { $Title } else { Join-Path $root ($Command -split " ")[0] }
$inner = "chcp 65001 >nul & mode con: cols=$Cols lines=$Lines & title $title & cd /d `"$root`" & cls & $Command"
# conhost.exe — классическое окно консоли, принадлежит только нам (терминал пользователя не трогаем)
$proc = Start-Process conhost.exe -ArgumentList "cmd.exe /k $inner" -PassThru
Start-Sleep -Seconds $WaitSeconds

$h = [IntPtr]::Zero
for ($i = 0; $i -lt 20 -and $h -eq [IntPtr]::Zero; $i++) {
    # окно консоли числится за процессом cmd, поэтому ищем его по уникальному заголовку
    $owner = Get-Process | Where-Object { $_.MainWindowTitle -like "$title*" } | Select-Object -First 1
    if ($owner) { $h = $owner.MainWindowHandle }
    if ($h -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 300 }
}
if ($h -eq [IntPtr]::Zero) { Stop-Process $proc -Force; throw "Окно консоли не найдено" }

[Win]::ShowWindow($h, 9) | Out-Null       # SW_RESTORE
[Win]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 700

$r = New-Object Win+RECT
[Win]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $hgt = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[Win]::PrintWindow($h, $hdc, 2) | Out-Null   # PW_RENDERFULLCONTENT: снимок даже если окно перекрыто
$g.ReleaseHdc($hdc); $g.Dispose()

# Обрезаем невидимую рамку окна Windows 11 (тень по краям)
$crop = 7
$rect = New-Object System.Drawing.Rectangle $crop, 0, ($w - 2 * $crop), ($hgt - $crop)
$cut = $bmp.Clone($rect, $bmp.PixelFormat)
New-Item -ItemType Directory -Force (Split-Path $Out) | Out-Null
$cut.Save((Join-Path $root $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$cut.Dispose(); $bmp.Dispose()

# закрываем окно: процесс conhost и запущенные в нём программы
Get-CimInstance Win32_Process -Filter "ParentProcessId=$($proc.Id)" | ForEach-Object {
    Get-CimInstance Win32_Process -Filter "ParentProcessId=$($_.ProcessId)" | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue
}
Stop-Process $proc -Force -ErrorAction SilentlyContinue
"saved $Out (${w}x${hgt})"
