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
#include <QMimeData>
#include <QUrl>
#include <QDragEnterEvent>
#include <QDropEvent>

typedef void* (__cdecl *CreateCryptFunc)(const char*);
typedef int   (__cdecl *DumpFunc)(void*, const char*);
typedef void  (__cdecl *FixFunc)(void*);
typedef void  (__cdecl *DestroyFunc)(void*);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
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
    selectedFiles = QFileDialog::getOpenFileNames(this, "选择 .ncm 文件(支持拖拽)", "", "NCM Files (*.ncm)");
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

    ui->textLog->clear();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(selectedFiles.size());
    ui->btnCancel->setVisible(true);
    ui->btnCancel->setEnabled(true);

    worker = new Worker(selectedFiles, actualOutputDir, ui->comboFormat->currentText());
    thread = new QThread(this);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::start);
    connect(worker, &Worker::log, this, [=](const QString &msg) {
        ui->textLog->append(msg);
    });
    connect(worker, &Worker::progress, this, [=](int current) {
        ui->progressBar->setValue(current);
    });

    connect(worker, &Worker::done, this, [=]() {
        ui->textLog->append("✅ 所有文件处理完成！");
        ui->btnCancel->setVisible(false);
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        worker = nullptr;
        thread = nullptr;
    });

    connect(worker, &Worker::cancelled, this, [=]() {
        ui->textLog->append("🚫 任务已取消");
        ui->btnCancel->setVisible(false);
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        worker = nullptr;
        thread = nullptr;
    });

    connect(ui->btnCancel, &QPushButton::clicked, this, [=]() {
        if (worker) {
            ui->btnCancel->setEnabled(false);
            worker->cancel();
            ui->textLog->append("⏹ 正在尝试取消...");
        }
    });

    thread->start();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
        QString filePath = url.toLocalFile();
        if (filePath.endsWith(".ncm", Qt::CaseInsensitive)) {
            if (!selectedFiles.contains(filePath)) {
                selectedFiles.append(filePath);
                ui->listFiles->addItem(filePath);
            }
        }
    }

    if (selectedFiles.isEmpty()) {
        ui->labelFile->setText("未选择文件");
    } else {
        ui->labelFile->setText(QString("已选择 %1 个 文件").arg(selectedFiles.size()));
    }
}
bool MainWindow::downloadFfmpegToLib() {
    return false;
}
