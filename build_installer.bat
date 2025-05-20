@echo off
setlocal enabledelayedexpansion

:: ===== Configuration =====
set "TARGET=ncmtool"

:: Qt path: CI or fallback
if defined QTDIR (
    set "QT_BIN=%QTDIR%\bin"
    powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Using Qt from QTDIR: %QT_BIN%'"
) else (
    set "QT_BIN=C:\Qt\6.9.0\msvc2022_64\bin"
    powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Using default Qt path: %QT_BIN%'"
)

:: Try detect UPX
where upx >nul 2>nul
if %errorlevel%==0 (
    set "UPX_EXE=upx"
) else (
    set "UPX_EXE=D:\software\upx-5.0.1-win64\upx.exe"
)

:: Check if UPX exists
if exist "%UPX_EXE%" (
    set "USE_UPX=1"
    powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Using UPX: %UPX_EXE%'"
) else (
    set "USE_UPX=0"
    powershell -Command "Write-Host '[WARN]  ' -ForegroundColor Yellow -NoNewline; Write-Host 'UPX not found, skipping compression.'"
)

:: NSIS
set "NSIS_EXE=C:\Program Files (x86)\NSIS\makensis.exe"
if not exist "%NSIS_EXE%" (
    powershell -Command "Write-Host '[ERROR] ' -ForegroundColor Red -NoNewline; Write-Host 'NSIS not found: %NSIS_EXE%'"
    exit /b 1
)

:: Paths
set "BUILD_DIR=out\build-release"
set "OUTPUT_DIR=%BUILD_DIR%\Release"
set "INSTALLER_DIR=%BUILD_DIR%\installer"

:: ===== Step 1: Clean installer output =====
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Cleaning installer directory...'"
if exist "%INSTALLER_DIR%" rd /s /q "%INSTALLER_DIR%"
mkdir "%INSTALLER_DIR%"

:: ===== Step 2: Copy executable =====
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Copying %TARGET%.exe...'"
copy "%OUTPUT_DIR%\%TARGET%.exe" "%INSTALLER_DIR%\" >nul

:: ===== Step 3: Deploy Qt runtime =====
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Running windeployqt...'"
"%QT_BIN%\windeployqt.exe" ^
  --no-translations --no-quick --no-opengl-sw --no-network --no-compiler-runtime ^
  --release --dir "%INSTALLER_DIR%" "%INSTALLER_DIR%\%TARGET%.exe"
if errorlevel 1 (
    powershell -Command "Write-Host '[ERROR] ' -ForegroundColor Red -NoNewline; Write-Host 'windeployqt failed.'"
    exit /b 1
)

:: ===== Step 4: Copy lib folder =====
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Copying lib/ folder...'"
xcopy /s /y "lib" "%INSTALLER_DIR%\lib\" >nul

:: ===== Step 5: UPX compression (optional) =====
if "%USE_UPX%"=="1" (
    powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Compressing with UPX...'"
    pushd "%INSTALLER_DIR%"
    for %%f in (*.exe) do (
        "%UPX_EXE%" --best "%%f" >nul 2>&1
    )
    if exist lib (
        pushd lib
        for %%f in (*.exe *.dll) do (
            "%UPX_EXE%" --best "%%f" >nul 2>&1
        )
        popd
    )
    popd
)

:: ===== Step 6: Remove unused Qt DLLs =====
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Cleaning unnecessary Qt DLLs...'"
del "%INSTALLER_DIR%\Qt6Network.dll" >nul 2>&1

:: ===== Step 7: Build NSIS installer =====
powershell -Command "Write-Host '[INFO]  ' -ForegroundColor Cyan -NoNewline; Write-Host 'Running NSIS installer builder...'"
"%NSIS_EXE%" ncmtool_install\ncmtool_installer.nsi
if errorlevel 1 (
    powershell -Command "Write-Host '[ERROR] ' -ForegroundColor Red -NoNewline; Write-Host 'NSIS build failed.'"
    exit /b 1
)

:: ===== Done =====
powershell -Command "Write-Host '[OK]    ' -ForegroundColor Green -NoNewline; Write-Host 'Installer complete: ncmtool_setup.exe'"
endlocal
pause
exit /b
