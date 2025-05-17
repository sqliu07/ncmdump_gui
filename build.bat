@echo off
setlocal

set BUILD_DIR=build
set QT_PATH=C:\Qt\6.5.3\msvc2019_64
set CMAKE_PREFIX=%QT_PATH%\lib\cmake
set TARGET=ncmtool

echo [1] Cleaning previous build directory...
rd /s /q %BUILD_DIR%

echo [2] Creating build directory...
mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo [3] Running CMake configuration...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX%"

echo [4] Building the project...
cmake --build . --config Release

echo [5] Copying lib/ directory to Release folder...
xcopy /E /Y ..\lib Release\lib\

echo [6] Deploying Qt runtime dependencies...
    "C:/Qt/6.9.0/msvc2022_64/bin/windeployqt.exe" Release\%TARGET%.exe

echo [7] Build complete. Opening output directory...
start Release

endlocal
