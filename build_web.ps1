# ===============================
# Web Build Script for GeomDash
# ===============================

# 1. Change execution policy for current session
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force

# 2. Activate EMSDK environment
& "C:\Users\midhu\dev\emsdk\emsdk_env.ps1"

# 3. Navigate to project folder
Set-Location "E:\Codaing\GeomDash"

# 4. Compile C++ to WebAssembly
em++ main.cpp -o webBuild/index.js -Os -s USE_GLFW=3 -s ASYNCIFY `
    --preload-file assets `
    --preload-file level.txt `
    -s MODULARIZE=1 `
    -s EXPORT_ES6 `
    -s ALLOW_MEMORY_GROWTH=1 `
    -I "C:\Users\midhu\dev\raylib\src" `
    -L "C:\Users\midhu\dev\raylib\build_web\raylib" `
    -lraylib

Write-Host "`n✅ Build complete! Files in: E:\Codaing\GeomDash\webBuild" -ForegroundColor Green
