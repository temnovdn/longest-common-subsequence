#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow w;
    w.show();  // Show the GUI window

    return app.exec();  // Start Qt event loop
}