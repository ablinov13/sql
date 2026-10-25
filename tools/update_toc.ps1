# Обновляет оглавление в отчётах через Word и (по желанию) сохраняет PDF для проверки.
# Запуск: powershell -File tools\update_toc.ps1 [-PdfDir obj\pdf] [-Only Лаба1]
param([string]$PdfDir = "", [string]$Only = "")
$root = Resolve-Path "$PSScriptRoot\.."
$word = New-Object -ComObject Word.Application
$word.Visible = $true   # в скрытом режиме Word иногда зависает
$word.DisplayAlerts = 0
try {
    Get-ChildItem (Join-Path $root "reports") -Filter "*.docx" |
        Where-Object { $_.Name -notlike "~*" -and (-not $Only -or $_.BaseName -eq $Only) } | ForEach-Object {
        # работаем с копией в obj\word: так Word не путается с файлом, который открыт или «помечен» после сбоя
        $work = Join-Path $root "obj\word"
        New-Item -ItemType Directory -Force $work | Out-Null
        $copy = Join-Path $work $_.Name
        Copy-Item $_.FullName $copy -Force
        $doc = $word.Documents.Open($copy)
        foreach ($toc in $doc.TablesOfContents) { $toc.Update() }
        $doc.Save()
        if ($PdfDir) {
            $dir = Join-Path $root $PdfDir
            New-Item -ItemType Directory -Force $dir | Out-Null
            $doc.SaveAs2([ref](Join-Path $dir ($_.BaseName + ".pdf")), [ref]17)  # 17 = PDF
        }
        $doc.Close([ref]0)
        Copy-Item $copy $_.FullName -Force
        "updated $($_.Name)"
    }
}
finally {
    $word.Quit()
    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($word) | Out-Null
}
