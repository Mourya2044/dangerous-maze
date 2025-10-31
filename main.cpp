#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load the pixel font
    int fontId = QFontDatabase::addApplicationFont(":/fonts/pixel-font.ttf");
    if (fontId != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        a.setFont(QFont(fontFamily, 12)); // Set as default
    } else {
        qWarning() << "Could not load pixel font!";
    }

    MainWindow w;
    w.show();
    return a.exec();
}