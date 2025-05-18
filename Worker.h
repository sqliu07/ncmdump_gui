#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QStringList>

class Worker : public QObject {
    Q_OBJECT
public:
    Worker(const QStringList &files, const QString &outputDir, const QString &format, bool deleteOriginal);
    ~Worker() = default;

signals:
    void log(const QString &msg);
    void progress(int current, int total);
    void done();
    void cancelled();

public slots:
    void start();
    void cancel();

private:
    QStringList files;
    QString outputDir;
    QString format;
    std::atomic<bool> cancelRequested = false;
    bool deleteOriginal = false;

    bool callLibNcmdump(const QString &filePath, const QString &outputDir);
    bool convertWithFfmpeg(const QString &inputPath, const QString &targetFormat);
};

#endif // WORKER_H
