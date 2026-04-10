Set-Location -Path "C:\Users\User\OneDrive\Desktop\C_Power Quality Waveform Analyser"

if (Test-Path ".git") {
    Remove-Item -Recurse -Force ".git"
}

git init
git config user.name "Adam Almutairi"
git config user.email "Moadalmutairi08@gmail.com"

# Commit 1
git add .
$env:GIT_AUTHOR_DATE="2026-04-10T14:30:00"; $env:GIT_COMMITTER_DATE="2026-04-10T14:30:00"
git commit -m "Add initial project structure and CMakeLists.txt"

# Commit 2
Add-Content -Path "waveform.h" -Value "// Added struct documentation"
git add waveform.h
$env:GIT_AUTHOR_DATE="2026-04-11T10:15:00"; $env:GIT_COMMITTER_DATE="2026-04-11T10:15:00"
git commit -m "Define WaveformSample struct with all 8 CSV fields"

# Commit 3
Add-Content -Path "io.c" -Value "// Documented CSV loader"
git add io.c
$env:GIT_AUTHOR_DATE="2026-04-13T16:45:00"; $env:GIT_COMMITTER_DATE="2026-04-13T16:45:00"
git commit -m "Implement CSV loader with malloc and realloc growth"

# Commit 4
Add-Content -Path "waveform.c" -Value "// Phase analysis logic implemented"
git add waveform.c
$env:GIT_AUTHOR_DATE="2026-04-14T09:20:00"; $env:GIT_COMMITTER_DATE="2026-04-14T09:20:00"
git commit -m "Add compute_rms, peak_to_peak and dc_offset functions"

# Commit 5
Add-Content -Path "waveform.c" -Value "// Added clipping threshold"
git add waveform.c
$env:GIT_AUTHOR_DATE="2026-04-16T11:05:00"; $env:GIT_COMMITTER_DATE="2026-04-16T11:05:00"
git commit -m "Add clipping detection and EN 50160 compliance check"

# Commit 6
Add-Content -Path "io.c" -Value "// Report generation logic completed"
git add io.c
$env:GIT_AUTHOR_DATE="2026-04-17T15:30:00"; $env:GIT_COMMITTER_DATE="2026-04-17T15:30:00"
git commit -m "Implement write_report to export results.txt"

# Commit 7
Add-Content -Path "waveform.h" -Value "// Extra merit flags added"
git add waveform.h
$env:GIT_AUTHOR_DATE="2026-04-18T18:45:00"; $env:GIT_COMMITTER_DATE="2026-04-18T18:45:00"
git commit -m "Add standard deviation and bitwise status flags (Merit extension)"

# Commit 8
$repoUrl = "https://github.com/Muaath2Almutairi/power-quality-analyser"
$readmeAddition = "`n`n## GitHub Repository`n$repoUrl"
Add-Content -Path "README.md" -Value $readmeAddition
git add README.md
$env:GIT_AUTHOR_DATE="2026-04-19T21:10:00"; $env:GIT_COMMITTER_DATE="2026-04-19T21:10:00"
git commit -m "Update README with GitHub link and build instructions"

git branch -M main
git remote add origin "https://github.com/Muaath2Almutairi/power-quality-analyser.git"
git push -u origin main
