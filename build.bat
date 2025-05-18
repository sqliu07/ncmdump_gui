@echo off
setlocal enabledelayedexpansion

:: ===== Configuration =====
:: ===== Modify these variables based on your environment. =====
set TARGET=ncmtool
set QT_PATH=C:\Qt\6.9.0\msvc2022_64
set CMAKE_PREFIX=%QT_PATH%\lib\cmake

set BUILD_DIR=out\build-release
set OUTPUT_DIR=%BUILD_DIR%\Release

:: ===== Step 1: Clean previous build =====
call :log_info "Removing previous build directory..."
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

:: ===== Step 2: Run CMake configuration =====
call :log_info "Configuring CMake..."
cmake -S . -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX%"
if errorlevel 1 (
    call :log_error "CMake configuration failed."
    exit /b 1
)

:: ===== Step 3: Build the project =====
call :log_info "Building project with %NUMBER_OF_PROCESSORS% threads..."
cmake --build "%BUILD_DIR%" --config Release -- /m:%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    call :log_error "Build failed."
    exit /b 1
)

:: ===== Step 4: Copy runtime lib directory =====
call :log_info "Copying lib directory..."
xcopy /E /Y "lib" "%OUTPUT_DIR%\lib\" >nul

:: ===== Step 5: Deploy Qt dependencies =====
call :log_info "Deploying Qt runtime files..."
"%QT_PATH%\bin\windeployqt.exe" --no-network --no-translations --no-opengl-sw --release "%OUTPUT_DIR%\%TARGET%.exe"
if errorlevel 1 (
    call :log_error "windeployqt failed."
    exit /b 1
)

:: ===== Step 6: Done =====
call :log_info "Build completed successfully."

endlocal
exit /b

:: ===== Logging Functions =====
:log_info
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host '%~1'"
goto :eof

:log_error
powershell -Command "Write-Host '[ERROR] ' -ForegroundColor Red -NoNewline; Write-Host '%~1'"
goto :eof