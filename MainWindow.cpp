#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Worker.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QLibrary>
#include <QStandardItemModel>
#include <QProgressDialog>
#include <QThread>

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

    // 清空日志 & 重置进度条
    ui->textLog->clear();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(selectedFiles.size());

    QThread *thread = new QThread(this);
    Worker *worker = new Worker(selectedFiles, actualOutputDir, ui->comboFormat->currentText());

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::start);
    connect(worker, &Worker::log, this, [=](const QString &msg) {
        ui->textLog->append(msg);
    });
    connect(worker, &Worker::progress, this, [=](int current, int total) {
        ui->progressBar->setValue(current);
    });
    connect(worker, &Worker::done, this, [=]() {
        ui->textLog->append("✅ 所有文件处理完成！");
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
    });

    thread->start();
}

bool MainWindow::downloadFfmpegToLib() {
    return false;
}
