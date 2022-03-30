#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ecgwidget.h"

#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openEcgFilePushButton_clicked()
{
    QUrl url = QFileDialog::getOpenFileUrl(this);
    QString path = url.path();
    qDebug() << "path:" << path;
    if (!path.isEmpty()) {
        path = path.mid(1);
        EcgWidget *widget = new EcgWidget();
        widget->ReadEcgFile(path);
        widget->showMaximized();
    }
}
