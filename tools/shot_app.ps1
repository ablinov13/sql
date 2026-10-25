# Снимок окна обычной программы (например, DB Browser for SQLite) для отчётов.
# Пример: powershell -File tools\shot_app.ps1 -Exe "C:\Program Files\DB Browser for SQLite\DB Browser for SQLite.exe" -ArgLine "lab1.db -s q1.sql" -Out screenshots\lab1_q1.png
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [string]$ArgLine = "",
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$WaitSeconds = 5,
    [int]$Width = 1100,
    [int]$Height = 760,
    [switch]$KeepOpen
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
$proc = Start-Process $Exe -ArgumentList $ArgLine -WorkingDirectory $root -PassThru
Start-Sleep -Seconds $WaitSeconds
$proc.Refresh()
$h = $proc.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { Stop-Process $proc -Force; throw "window not found" }

[Win]::ShowWindow($h, 9) | Out-Null
[Win]::MoveWindow($h, 40, 40, $Width, $Height, $true) | Out-Null
[Win]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 1500

$r = New-Object Win+RECT
[Win]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $hgt = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[Win]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()

$crop = 7
$rect = New-Object System.Drawing.Rectangle $crop, 0, ($w - 2 * $crop), ($hgt - $crop)
$cut = $bmp.Clone($rect, $bmp.PixelFormat)
New-Item -ItemType Directory -Force (Split-Path (Join-Path $root $Out)) | Out-Null
$cut.Save((Join-Path $root $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$cut.Dispose(); $bmp.Dispose()

if (-not $KeepOpen) { Stop-Process $proc -Force -ErrorAction SilentlyContinue }
"saved $Out (${w}x${hgt})"
