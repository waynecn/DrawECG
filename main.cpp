#include "mainwindow.h"
#include "common.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("wayne");
    QCoreApplication::setOrganizationDomain("wayne.com");
    QCoreApplication::setApplicationName("DrawECG");

    QScreen *screen = QGuiApplication::screens()[0];      //
    qreal dotsPerInchX = screen->physicalDotsPerInchX();
    qreal dotsPerInchY = screen->physicalDotsPerInchY();
    qreal dotsPerInch = qMin(dotsPerInchX, dotsPerInchY);
    qDebug() << "dpi:" << dotsPerInch << " dpiX:" << dotsPerInchX << " dpiY:" << dotsPerInchY;
    DOTS_PER_MM = dotsPerInch / 25.39999918;
    qDebug() << "DOTS_PER_MM:" << DOTS_PER_MM << " dpi:" << dotsPerInch;

    MainWindow w;
    w.show();

    return a.exec();
}
