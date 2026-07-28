# Copy-Changelog.ps1
$repoPath = "C:\Users\Administrator\Documents\GitHub\Knot"

Set-Location -LiteralPath $repoPath

try {
    $tag = git describe --tags --abbrev=0 2>$null
    if (-not $tag) {
        throw "No tags found. Please run git fetch --tags first."
    }

    $log = git log "$tag..HEAD" --pretty=format:"- %s" --no-merges

    if ([string]::IsNullOrWhiteSpace($log)) {
        Write-Host "[!] No new commits since $tag" -ForegroundColor Yellow
    } else {
        # 使用 clip.exe 替代 Set-Clipboard，避免剪贴板 API 失败
        $log | clip.exe

        $count = ($log -split "`n").Count
        Write-Host "[OK] Copied $count records to clipboard!" -ForegroundColor Green
        Write-Host ""
        Write-Host $log
    }
} catch {
    Write-Host "[ERROR] $_" -ForegroundColor Red
}

Write-Host ""
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
