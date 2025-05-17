#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QLibrary>
#include <QStandardItemModel>
#include <QProgressDialog>

typedef void* (__cdecl *CreateCryptFunc)(const char*);
typedef int   (__cdecl *DumpFunc)(void*, const char*);
typedef void  (__cdecl *FixFunc)(void*);
typedef void  (__cdecl *DestroyFunc)(void*);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->comboFormat->addItems({"保持原始格式", "转为 MP3", "转为 FLAC"});
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->comboFormat->model());
    if (model && model->item(2)) {
        model->item(2)->setToolTip("MP3 是有损格式，转为 FLAC 并不会提升音质。");
    }
    ui->textLog->setReadOnly(true);
    ui->listFiles->setSelectionMode(QAbstractItemView::NoSelection);
    this->resize(800, 600);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_btnSelectFile_clicked() {
    selectedFiles = QFileDialog::getOpenFileNames(this, "选择 .ncm 文件", "", "NCM Files (*.ncm)");
    ui->listFiles->clear();

    if (selectedFiles.isEmpty()) {
        ui->labelFile->setText("未选择文件");
    } else {
        ui->labelFile->setText(QString("已选择 %1 个 文件").arg(selectedFiles.size()));
    }

    for (const auto &f : selectedFiles) {
        ui->listFiles->addItem(f);
    }
}

void MainWindow::on_btnSelectOutput_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "选择输出目录");
    if (!dir.isEmpty()) {
        outputDir = dir;
        ui->labelOutput->setText(dir);
    }
}

void MainWindow::on_btnStart_clicked() {
    if (selectedFiles.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择文件");
        return;
    }
    QString actualOutputDir = outputDir;
    if (actualOutputDir.isEmpty()) {
        actualOutputDir = QFileInfo(selectedFiles.first()).absolutePath();
        ui->labelOutput->setText("默认路径: " + actualOutputDir);
    }
    for (const QString &filePath : selectedFiles) {
        QString targetOutputDir = outputDir.isEmpty() ?
            QFileInfo(filePath).absolutePath() : outputDir;

        ui->textLog->append("🔄 处理：" + filePath);
        if (!callLibNcmdump(filePath, targetOutputDir)) {
            ui->textLog->append("❌ 解密失败: " + filePath);
            continue;
        }

        QFileInfo finfo(filePath);
        QString baseName = targetOutputDir + "/" + finfo.completeBaseName();
        QString rawFile;
        for (const auto &ext : {".flac", ".mp3", ".wav"}) {
            if (QFile::exists(baseName + ext)) {
                rawFile = baseName + ext;
                break;
            }
        }

        if (rawFile.isEmpty()) {
            ui->textLog->append("⚠️ 找不到输出音频文件");
            continue;
        }

        QString format = ui->comboFormat->currentText();
        if (format == "保持原始格式") {
            ui->textLog->append("✅ 解密完成: " + rawFile);
        } else {
            QString targetFormat = (format.contains("MP3")) ? "mp3" : "flac";
            if (convertWithFfmpeg(rawFile, targetFormat)) {
                ui->textLog->append("🎵 转码完成: " + baseName + "." + targetFormat);
                QFile::remove(rawFile);
            } else {
                ui->textLog->append("⚠️ 转码失败");
            }
        }
    }
}

bool MainWindow::callLibNcmdump(const QString &filePath, const QString &outputDir) {
    QString dllPath = QCoreApplication::applicationDirPath() + "/lib/libncmdump.dll";
    QLibrary lib(dllPath);
    if (!lib.load()) {
        ui->textLog->append("❌ 加载 DLL 失败：" + dllPath);
        return false;
    }

    auto create = (CreateCryptFunc)lib.resolve("CreateNeteaseCrypt");
    auto dump = (DumpFunc)lib.resolve("Dump");
    auto fix = (FixFunc)lib.resolve("FixMetadata");
    auto destroy = (DestroyFunc)lib.resolve("DestroyNeteaseCrypt");

    if (!create || !dump || !fix || !destroy) {
        ui->textLog->append("❌ 解析 DLL 接口失败");
        return false;
    }

    void* obj = create(filePath.toUtf8().data());
    int result = dump(obj, outputDir.toUtf8().data());
    fix(obj);
    destroy(obj);

    return result == 0;
}

bool MainWindow::convertWithFfmpeg(const QString &inputPath, const QString &targetFormat) {
    static bool ffmpegAvailableChecked = false;
    static QString ffmpegPath;
    static bool ffmpegAvailable = false;

    if (!ffmpegAvailableChecked) {
        ffmpegPath = QCoreApplication::applicationDirPath() + "/lib/ffmpeg.exe";

        if (!QFile::exists(ffmpegPath)) {
            QProcess check;
            check.start("ffmpeg", {"-version"});
            check.waitForStarted(1000);
            check.waitForFinished(3000);
            QString out = check.readAllStandardOutput() + check.readAllStandardError();

            if (check.exitStatus() == QProcess::NormalExit &&
                check.exitCode() == 0 &&
                out.contains("ffmpeg version")) {
                ffmpegPath = "ffmpeg";
                ffmpegAvailable = true;
            } else {
                int ret = QMessageBox::question(this, "转码需要 ffmpeg",
                    "找不到 ffmpeg，是否现在下载？");

                if (ret == QMessageBox::Yes) {
                    if (downloadFfmpegToLib()) {
                        ffmpegPath = QCoreApplication::applicationDirPath() + "/lib/ffmpeg.exe";
                        ffmpegAvailable = true;
                    } else {
                        QMessageBox::warning(this, "失败", "下载失败，将跳过所有转码。");
                        ffmpegAvailable = false;
                    }
                } else {
                    ffmpegAvailable = false;
                }
            }
        } else {
            ffmpegAvailable = true;
        }

        ffmpegAvailableChecked = true;
    }

    if (!ffmpegAvailable)
        return false;
    
    // 转码逻辑不变
    QString output = inputPath;
    output.chop(QFileInfo(inputPath).suffix().length());
    output += targetFormat;

    QProcess process;
    QStringList args = {"-y", "-i", inputPath, "-codec:a",
                        (targetFormat == "mp3" ? "libmp3lame" : "flac"),
                        output};
    process.start(ffmpegPath, args);
    bool finished = process.waitForFinished();

    if (!finished || !QFile::exists(output)) {
        QString err = process.readAllStandardError();
        qDebug() << "ffmpeg failed:" << err;
        return false;
    }

    return true;
}


bool MainWindow::downloadFfmpegToLib() {
    return false;
}
