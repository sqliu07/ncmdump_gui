#include "Worker.h"
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QProcess>
#include <QLibrary>

typedef void* (__cdecl *CreateCryptFunc)(const char*);
typedef int   (__cdecl *DumpFunc)(void*, const char*);
typedef void  (__cdecl *FixFunc)(void*);
typedef void  (__cdecl *DestroyFunc)(void*);

Worker::Worker(const QStringList &files, const QString &outputDir, const QString &format, bool deleteOriginal)
    : files(files), outputDir(outputDir), format(format), deleteOriginal(deleteOriginal) {}

void Worker::start() {
    int total = files.size();

    for (int i = 0; i < total; ++i) {
        if (cancelRequested.load()) {
            emit log("🛑 用户取消任务");
            emit cancelled();
            return;
        }

        const QString &filePath = files[i];
        emit log(QString("🔄 处理：%1").arg(filePath));

        if (!callLibNcmdump(filePath, outputDir)) {
            emit log("❌ 解密失败: " + filePath);
            emit progress(i + 1, total);
            continue;
        }

        QFileInfo finfo(filePath);
        QString baseName = outputDir + "/" + finfo.completeBaseName();
        QString rawFile;

        for (const auto &ext : {".flac", ".mp3", ".wav"}) {
            if (QFile::exists(baseName + ext)) {
                rawFile = baseName + ext;
                break;
            }
        }

        if (rawFile.isEmpty()) {
            emit log("⚠️ 找不到输出音频文件");
            emit progress(i + 1, total);
            continue;
        }

        if (format == "保持原始格式") {
            emit log("✅ 解密完成: " + rawFile);
        } else {
            QString targetFormat = format.contains("MP3") ? "mp3" : "flac";
            if (convertWithFfmpeg(rawFile, targetFormat)) {
                emit log("🎵 转码完成: " + baseName + "." + targetFormat);
                QFile::remove(rawFile);
            } else {
                emit log("⚠️ 转码失败");
            }
        }

        if (deleteOriginal) {
            if (QFile::remove(filePath)) {
                emit log("🧹 已删除原始文件: " + filePath);
            } else {
                emit log("⚠️ 无法删除原始文件: " + filePath);
            }
        }

        emit progress(i + 1, total);
    }

    emit done();
}
void Worker::cancel() {
    cancelRequested.store(true);
}
bool Worker::callLibNcmdump(const QString &filePath, const QString &outputDir) {
    QString dllPath = QCoreApplication::applicationDirPath() + "/lib/libncmdump.dll";
    QLibrary lib(dllPath);
    if (!lib.load()) {
        emit log("❌ 加载 DLL 失败：" + dllPath);
        return false;
    }

    auto create = (CreateCryptFunc)lib.resolve("CreateNeteaseCrypt");
    auto dump = (DumpFunc)lib.resolve("Dump");
    auto fix = (FixFunc)lib.resolve("FixMetadata");
    auto destroy = (DestroyFunc)lib.resolve("DestroyNeteaseCrypt");

    if (!create || !dump || !fix || !destroy) {
        emit log("❌ 解析 DLL 接口失败");
        return false;
    }

    void* obj = create(filePath.toUtf8().data());
    int result = dump(obj, outputDir.toUtf8().data());
    fix(obj);
    destroy(obj);

    return result == 0;
}

bool Worker::convertWithFfmpeg(const QString &inputPath, const QString &targetFormat) {
    if (cancelRequested.load()) return false;  // ✅ 一开始就检查

    QString ffmpegPath = QCoreApplication::applicationDirPath() + "/lib/ffmpeg.exe";
    if (!QFile::exists(ffmpegPath)) {
        emit log("⚠️ ffmpeg.exe 未找到，跳过转码");
        return false;
    }

    QString output = inputPath;
    output.chop(QFileInfo(inputPath).suffix().length());
    output += targetFormat;

    QProcess process;
    QStringList args = {"-y", "-i", inputPath, "-codec:a",
                        (targetFormat == "mp3" ? "libmp3lame" : "flac"),
                        output};

    process.start(ffmpegPath, args);

    // ⏳ 等待期间允许用户中断
    while (!process.waitForFinished(200)) {
        if (cancelRequested.load()) {
            emit log("🛑 取消中：中止 ffmpeg 转码");
            process.kill();
            return false;
        }
    }

    if (!QFile::exists(output)) {
        emit log("⚠️ 转码失败，输出文件未生成");
        return false;
    }

    return true;
}

