#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            QString path = QString::fromLocal8Bit(argv[i]);
            if (path.endsWith(".ncm", Qt::CaseInsensitive)) {
                w.addExternalFile(path);
            }
        }
    }

    w.show();
    return a.exec();
}
