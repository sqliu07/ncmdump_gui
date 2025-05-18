@echo off
setlocal

:: 配置路径
set QT_BIN=C:\Qt\6.9.0\msvc2022_64\bin
set UPX_EXE=D:\software\upx-5.0.1-win64\upx.exe
set NSIS_EXE=C:\Program Files (x86)\NSIS\makensis.exe
set EXE_NAME=ncmtool.exe

:: 清理旧安装目录
echo [INFO] Cleaning previous installer directory...
rmdir /s /q installer
mkdir installer

:: 复制可执行文件
echo [INFO] Copying %EXE_NAME%...
copy build\Release\%EXE_NAME% installer\

:: 运行 windeployqt
echo [INFO] Deploying Qt runtime...
"%QT_BIN%\windeployqt.exe" ^
  --no-translations --no-quick --no-opengl-sw --no-compiler-runtime ^
  --release --dir installer installer\%EXE_NAME%

:: 拷贝 lib 目录（含 ffmpeg.exe 和 dll）
xcopy /s /y lib installer\lib

:: 压缩 exe 和 dll（可选）
echo [INFO] Compressing with UPX...
pushd installer
for %%f in (*.exe *.dll) do (
    "%UPX_EXE%" --best "%%f"
)

pushd lib
for %%f in (*.exe *.dll) do (
    "%UPX_EXE%" --best "%%f"
)
popd
popd

echo [INFO] Removing unused Qt modules...
del installer\Qt6Network.dll >nul 2>&1

:: 编译安装包
echo [INFO] Building NSIS installer...
"%NSIS_EXE%" ./ncmtool_install/ncmtool_installer.nsi

echo [DONE] Installer is ready: ncmtool_setup.exe
endlocal
pause
