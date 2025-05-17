# ncmtool

`ncmtool` 是一个基于 [ncmdump](https://github.com/taurusxin/ncmdump) 的Windows版图形界面解密工具，支持将网易云 `.ncm` 文件解密为常见音频格式（如 FLAC、MP3），并可选执行转码。

> 本项目仅为原始命令行工具的图形封装，核心解密逻辑来自原作者项目：[taurusxin/ncmdump](https://github.com/taurusxin/ncmdump)。

---

## ✨ 功能特性

- 📂 支持批量选择 `.ncm` 文件
- 📁 自定义输出目录（或默认与输入文件相同）
- 🔐 解密为 FLAC/MP3/WAV
- 🎵 可选转码为 MP3 或 FLAC（内置 `ffmpeg.exe`，无需联网下载）
- 🪄 图形界面基于 Qt 6 构建
- 📦 支持一键打包为 Windows 安装包（含图标、UPX 压缩）

---

## 🛠 构建说明

项目使用 C++17 + Qt 6 + CMake 构建，并集成一键打包流程。

### 构建依赖

- Qt 6.x（推荐 Qt 6.5 或以上）
- CMake（建议 3.16+）
- Ninja 或 MSVC 编译器
- [UPX](https://upx.github.io/)（可选，用于压缩可执行文件和 DLL）
- [NSIS](https://nsis.sourceforge.io/)（用于生成安装包）

### 脚本说明

项目提供以下两个脚本，位于项目根目录：

####  `build.bat`
1. 生成ncmtool.exe可执行文件，路径：`build/Release/ncmtool.exe `如果你不需要安装，执行它即可

####  `build_installer.bat`

一键执行以下步骤：

1. 调用 `windeployqt` 收集 Qt 运行库到 `installer/` 目录
2. 拷贝 `lib/` 中的依赖（如 `ffmpeg.exe`、`libncmdump.dll`）
3. 使用 `UPX` 压缩所有可执行文件和 DLL（可选）
4. 调用 `makensis` 执行打包脚本，生成安装包 `ncmtool_setup.exe`

####  `ncmtool_installer.nsi`

NSIS 安装脚本，定义了以下内容：

- 安装目录（默认 `C:\Program Files\ncmtool`）
- 创建桌面快捷方式与开始菜单项
- 卸载支持
- 图标资源（可选配置）

---

## 💡 致谢

- 解密核心逻辑：[@taurusxin](https://github.com/taurusxin) 的 [ncmdump](https://github.com/taurusxin/ncmdump)
- 图标素材：自制 / 免费资源（可替换）

---

## 📄 License

本 GUI 项目使用 MIT 协议开源，核心库 `ncmdump` 遵循其原始 GPL-3.0 协议。请自行确认依赖项的许可条款。
