#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "Worker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addExternalFile(const QString &path);

private slots:
    void on_btnSelectFile_clicked();
    void on_btnSelectFolder_clicked();
    void on_btnSelectOutput_clicked();
    void on_btnStart_clicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::MainWindow *ui;
    QStringList selectedFiles;
    QString outputDir;
    Worker* worker = nullptr;
    QThread* thread = nullptr;

    bool callLibNcmdump(const QString &filePath, const QString &outputDir);
    bool convertWithFfmpeg(const QString &inputPath, const QString &targetFormat);
    bool downloadFfmpegToLib();

};

#endif // MAINWINDOW_H
