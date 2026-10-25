# Скриншот DB Browser for SQLite с выполненным запросом (для отчёта по лабе 1).
# Открывает БД, вкладку «Execute SQL», вставляет текст запроса, нажимает F5 и снимает окно.
# Пример: powershell -File tools\shot_db4s.ps1 -DbFile lab1.db -SqlFile obj\lab1\q1.sql -Out screenshots\lab1_q1.png
param(
    [Parameter(Mandatory = $true)][string]$DbFile,
    [Parameter(Mandatory = $true)][string]$SqlFile,
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$Width = 1300,
    [int]$Height = 820,
    [int]$Lift = 0
)
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing, System.Windows.Forms, UIAutomationClient, UIAutomationTypes
Add-Type @"
using System; using System.Runtime.InteropServices;
public class Win {
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h, int x, int y, int w, int hh, bool repaint);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint x, uint y, uint d, IntPtr e);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint f, IntPtr e);
  public struct RECT { public int Left, Top, Right, Bottom; }
}
"@
[Win]::SetProcessDPIAware() | Out-Null

# нажатие клавиш по виртуальным кодам (не зависит от раскладки, в отличие от SendKeys)
function Keys([byte[]]$vks) {
    foreach ($vk in $vks) { [Win]::keybd_event($vk, 0, 0, [IntPtr]::Zero) }
    [array]::Reverse($vks)
    foreach ($vk in $vks) { [Win]::keybd_event($vk, 0, 2, [IntPtr]::Zero) }
    Start-Sleep -Milliseconds 300
}

function Click([int]$x, [int]$y) {
    [Win]::SetCursorPos($x, $y) | Out-Null
    Start-Sleep -Milliseconds 150
    [Win]::mouse_event(0x2, 0, 0, 0, [IntPtr]::Zero)
    [Win]::mouse_event(0x4, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 400
}

$root = Resolve-Path "$PSScriptRoot\.."
$exe = "C:\Program Files\DB Browser for SQLite\DB Browser for SQLite.exe"
$proc = Start-Process $exe -ArgumentList "`"$DbFile`"" -WorkingDirectory $root -PassThru
Start-Sleep -Seconds 5
$proc.Refresh()
$h = $proc.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { Stop-Process $proc -Force; throw "window not found" }
[Win]::ShowWindow($h, 9) | Out-Null
[Win]::MoveWindow($h, 40, 40, $Width, $Height, $true) | Out-Null
[Win]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 800

# вкладка Execute SQL
$ui = [System.Windows.Automation.AutomationElement]::FromHandle($h)
$cond = New-Object System.Windows.Automation.AndCondition(
    (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::TabItem)),
    (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::NameProperty, "Execute SQL")))
$tab = $ui.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $cond)
if (-not $tab) { Stop-Process $proc -Force; throw "Execute SQL tab not found" }
$r = $tab.Current.BoundingRectangle
Click ([int]($r.X + $r.Width / 2)) ([int]($r.Y + $r.Height / 2))

# щелчок в редактор SQL (под вкладкой), вставка текста запроса через буфер обмена, F5
Click ([int]($r.X + $r.Width / 2)) ([int]($r.Bottom + 120))
$saved = $null
try { $saved = [System.Windows.Forms.Clipboard]::GetText() } catch {}
[System.Windows.Forms.Clipboard]::SetText((Get-Content (Join-Path $root $SqlFile) -Raw -Encoding UTF8).Trim())
Keys @(0x11, 0x41)   # Ctrl+A
Keys @(0x11, 0x56)   # Ctrl+V
Keys @(0x74)         # F5 — выполнить
Start-Sleep -Seconds 2

# опускаем разделитель между таблицей результатов и журналом выполнения, чтобы влезли все строки
$tbl = $ui.FindFirst([System.Windows.Automation.TreeScope]::Descendants,
    (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::Table)))
if ($tbl) {
    $t = $tbl.Current.BoundingRectangle
    $sx = [int]($t.X + $t.Width / 2); $sy = [int]($t.Y + $t.Height + 3)
    [Win]::SetCursorPos($sx, $sy) | Out-Null; Start-Sleep -Milliseconds 200
    [Win]::mouse_event(0x2, 0, 0, 0, [IntPtr]::Zero)
    for ($y = $sy; $y -le $sy + $Lift; $y += 10) { [Win]::SetCursorPos($sx, $y) | Out-Null; Start-Sleep -Milliseconds 15 }
    [Win]::mouse_event(0x4, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 600
}
if ($saved) { [System.Windows.Forms.Clipboard]::SetText($saved) } else { [System.Windows.Forms.Clipboard]::Clear() }

# снимок окна
$wr = New-Object Win+RECT
[Win]::GetWindowRect($h, [ref]$wr) | Out-Null
$w = $wr.Right - $wr.Left; $hgt = $wr.Bottom - $wr.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[Win]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()
$crop = 7
$cut = $bmp.Clone((New-Object System.Drawing.Rectangle $crop, 0, ($w - 2 * $crop), ($hgt - $crop)), $bmp.PixelFormat)
New-Item -ItemType Directory -Force (Split-Path (Join-Path $root $Out)) | Out-Null
$cut.Save((Join-Path $root $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$cut.Dispose(); $bmp.Dispose()

Stop-Process $proc -Force -ErrorAction SilentlyContinue
"saved $Out (${w}x${hgt})"
