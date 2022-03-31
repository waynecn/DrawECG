#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ecgwidget.h"

#include <QFileDialog>
#include <QDebug>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setStyleSheet("background-color:rgba(255,255,255,1);");
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openEcgFilePushButton_clicked()
{
    QSettings settings;
    QVariant val = settings.value("OpenFileDir");
    QString dir;
    if (val.isValid()) {
        dir = val.toString();
    } else {
        dir = "C:/";
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("SelectEcgFile"), dir);
    qDebug() << "fileName:" << fileName;
    if (!fileName.isEmpty()) {
        EcgWidget *widget = new EcgWidget();
        widget->ReadEcgFile(fileName);
        widget->showMaximized();
    }
    QFileInfo f(fileName);
    qDebug() << "dir:" << f.dir().path();
    settings.setValue("OpenFileDir", f.dir().path());
}
