#include "ecgwidget.h"
#include "common.h"

#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QGuiApplication>
#include <QScreen>

EcgWidget::EcgWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setStyleSheet("background-color:rgba(255,255,255,1);");

    m_sampleRate = 1000;
    m_fGain = 10.0;
    m_iSpeed = 50;
    m_bAntialiasing = false;
    m_bOptimize = false;
    m_bDrawGrid = true;
    m_bDrawPoint = true;

    m_pMenu = new QMenu();
    m_pMenu->setStyleSheet("QMenu{\
                           background:rgba(255,255,255,1);\
                           border:none;\
                         }\
                         QMenu::item{\
                           padding:4px 16px;\
                           color:rgba(51,51,51,1);\
                           font-size:12px;\
                         }\
                         QMenu::item:hover{\
                           background-color:#409CE1;\
                         }\
                         QMenu::item:selected{\
                           background-color:#409CE1;\
                         }");
    QMenu *gain = m_pMenu->addMenu("Gain");
    QAction *gain5 = gain->addAction("5");
    QAction *gain10 = gain->addAction("10");
    QAction *gain20 = gain->addAction("20");
    QActionGroup *gainGroup = new QActionGroup(this);
    gainGroup->addAction(gain5);
    gainGroup->addAction(gain10);
    gainGroup->addAction(gain20);
    gainGroup->setExclusive(true);
    gain5->setCheckable(true);
    gain5->setChecked(false);
    gain10->setCheckable(true);
    gain10->setChecked(true);
    gain20->setCheckable(true);
    gain20->setChecked(false);
    connect(gain5, SIGNAL(triggered()), this, SLOT(onGain5Triggered()));
    connect(gain10, SIGNAL(triggered()), this, SLOT(onGain10Triggered()));
    connect(gain20, SIGNAL(triggered()), this, SLOT(onGain20Triggered()));

    m_pMenu->addSeparator();

    QMenu *speed = m_pMenu->addMenu("Speed");
    QAction *speed25 = speed->addAction("25");
    QAction *speed50 = speed->addAction("50");
    QAction *speed100 = speed->addAction("100");
    QActionGroup *speedGroup = new QActionGroup(this);
    speedGroup->addAction(speed25);
    speedGroup->addAction(speed50);
    speedGroup->addAction(speed100);
    speedGroup->setExclusive(true);
    speed25->setCheckable(true);
    speed25->setChecked(false);
    speed50->setCheckable(true);
    speed50->setChecked(true);
    speed100->setCheckable(true);
    speed100->setChecked(false);
    connect(speed25, SIGNAL(triggered()), this, SLOT(onSpeed25Triggered()));
    connect(speed50, SIGNAL(triggered()), this, SLOT(onSpeed50Triggered()));
    connect(speed100, SIGNAL(triggered()), this, SLOT(onSpeed100Triggered()));

    m_pMenu->addSeparator();

    QAction *gridAction = m_pMenu->addAction("DrawGrid");
    gridAction->setCheckable(true);
    gridAction->setChecked(true);
    connect(gridAction, SIGNAL(triggered()), this, SLOT(onDrawGridTriggered()));

    m_pMenu->addSeparator();

    QAction *pointAction = m_pMenu->addAction("DrawPoint");
    pointAction->setCheckable(true);
    pointAction->setChecked(true);
    connect(pointAction, SIGNAL(triggered()), this, SLOT(onDrawPointTriggered()));

    m_pMenu->addSeparator();

    QAction *smoothAct = m_pMenu->addAction("Smooth");
    smoothAct->setCheckable(true);
    smoothAct->setChecked(false);
    connect(smoothAct, SIGNAL(triggered()), this, SLOT(onAntialiasingTriggered()));

    m_pMenu->addSeparator();

    QAction *promptEcg = m_pMenu->addAction("Optimize");
    promptEcg->setCheckable(true);
    promptEcg->setChecked(false);
    connect(promptEcg, SIGNAL(triggered()), this, SLOT(onOptimizeTriggered()));

    m_pMenu->addSeparator();

    QAction *printAction = m_pMenu->addAction("Print Preview");
    connect(printAction, SIGNAL(triggered()), this, SLOT(onPrintPreviewTriggered()));
}

EcgWidget::~EcgWidget() {
    delete m_pMenu;
    m_pMenu = nullptr;
}

void EcgWidget::mousePressEvent(QMouseEvent *event) {
    event->accept();
}

void EcgWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        m_pMenu->exec(event->globalPos());
    }
    event->accept();
}

void EcgWidget::ReadEcgFile(QString &file) {
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qDebug() << "open file failed.";
        return;
    }
    QByteArray content = f.readAll();

    //format content to QJsonObject
    QJsonDocument doc = QJsonDocument::fromJson(content);
    int leadNum = 0;
    if (doc.isArray()) {
        QJsonArray waveArr = doc.array();
        for (int i = 0; i < waveArr.size(); ++i) {
            QJsonValue val = waveArr.at(i);
            QJsonObject obj = val.toObject();
            QString code = obj["code"].toString();
            //QJsonArray waveArr = obj["value"].toArray();
            //qDebug() << "code:" << code;
            leadNum++;
        }
    } else {
        QMessageBox::warning(this, "Warn", "May not be ecg file.");
    }

    //qDebug() << "leadNum:" << leadNum;
    m_leadNums = leadNum;
    if (m_leadNums > 0) {
        m_waveDoc = doc;
        update();
    }
}

void EcgWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int w = this->width(), h = this->height();
    drawGrid(painter, w, h, DOTS_PER_MM);
    drawWave(painter, w, h, DOTS_PER_MM);

    painter.save();
    QPen pen(Qt::blue);
    QFont f("Consolas");
    f.setBold(true);
    painter.setPen(pen);
    painter.setFont(f);
    QString text = QString("Lead:%1 Gain:%2 Speed:%3").arg(m_leadNums).arg(m_fGain).arg(m_iSpeed);
    painter.drawText(10, 20, text);
    painter.restore();
    event->accept();
}

void EcgWidget::drawGrid(QPainter &painter, int w, int h, double dDotsPerMM) {
    painter.save();
    painter.setPen(QColor(253, 180, 218));
    int bigGrid = (5.0 * dDotsPerMM);
    if (m_bDrawGrid) {
        for (int i = 0; i <= w / bigGrid; ++i) {
            painter.drawLine(0, i * bigGrid, w, i * bigGrid);
            painter.drawLine(i * bigGrid, 0, i * bigGrid, h);
        }
    }
    if (m_bDrawPoint) {
        for (int x = 0; x <= w / bigGrid; ++x) {
            for (int y = 0; y <= h / bigGrid; ++y) {
                for (int i = 1; i < 5; ++i) {
                    for (int j = 1; j < 5; ++j) {
                        QPointF p((qreal)(1.0 * x * bigGrid + 1.0 * i * dDotsPerMM), (qreal)(1.0 * y * bigGrid + 1.0 * j * dDotsPerMM));
                        painter.drawPoint(p);
                    }
                }
            }
        }
    }
    painter.restore();
}

void EcgWidget::drawWave(QPainter &painter, int width, int height, double dotsPerMM, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, m_bAntialiasing);
    float fNumPerPixel = m_sampleRate * 1.0 / (m_iSpeed * dotsPerMM);
    float fPixelPerMv = m_fGain * dotsPerMM;
    float fPixelPerUv = 0.001 * fPixelPerMv;
    int rectWidth = width / 2, rowHeight = height / 6, middleHeight = rowHeight / 2;
    int cols = 0, rows = 0;
    if (m_leadNums == 12) {
        cols = 2;
        rows = 6;
        //split the screen to two parts; each part split to six rows
        rectWidth = width / cols;
        rowHeight = height / rows;
        middleHeight = rowHeight / 2;
    } else if (15 == m_leadNums || 18 == m_leadNums) {
        //split the screen to three parts
        cols = 3;
        rows = (15 == m_leadNums ? 5 : 6);
        //split the screen to two parts; each part split to six rows
        rectWidth = width / cols;
        rowHeight = height / rows;
        middleHeight = rowHeight / 2;
    }

    //draw waves here
    QJsonArray waveArrs = m_waveDoc.array();
    for (int col = 0; col < cols; ++col) {
        for (int row = 0; row < rows; ++row) {
            QJsonObject singleWaveItem = waveArrs.at(col * rows + row).toObject();
            QString leadName = singleWaveItem["code"].toString();
            //qDebug() << "leadNum:" << leadName;
            QJsonArray waveArr = singleWaveItem["value"].toArray();
            int waveLen = waveArr.size();
            QVector<QPointF> vecPoints;
            if (!m_bOptimize) {
                for (int i = 0; i < waveLen; ++i) {
                    if (i / fNumPerPixel > rectWidth - DOTS_PER_MM * 2) {
                        break;
                    }
                    QPointF p((qreal)(col * rectWidth + i / fNumPerPixel), (qreal)(row * rowHeight + middleHeight - (waveArr[i].toInt() * fPixelPerUv)));
                    vecPoints.append(p);
                }
            } else {
                int numPerPixel = fNumPerPixel + 0.5;
                for (int i = 0; i < waveLen; i+=numPerPixel) {
                    if (i / fNumPerPixel > rectWidth - DOTS_PER_MM * 2) {
                        break;
                    }
                    int minIndex = i;
                    int maxIndex = i;
                    int minValue = waveArr[i].toInt();
                    int maxValue = waveArr[i].toInt();
                    for (int j = 0; j < numPerPixel; ++j) {
                        if (i + j >= waveLen) {
                            break;
                        }
                        if (minValue > waveArr[i + j].toInt()) {
                            minValue = waveArr[i + j].toInt();
                            minIndex = i + j;
                        }
                        if (maxValue < waveArr[i + j].toInt()) {
                            maxValue = waveArr[i + j].toInt();
                            maxIndex = i + j;
                        }
                    }
                    QPointF minPoint((qreal)(col * rectWidth + minIndex / fNumPerPixel), (qreal)(row * rowHeight + middleHeight - (minValue * fPixelPerUv)));
                    QPointF maxPoint((qreal)(col * rectWidth + maxIndex / fNumPerPixel), (qreal)(row * rowHeight + middleHeight - (maxValue * fPixelPerUv)));
                    minIndex >= maxIndex ? vecPoints.append(maxPoint) : vecPoints.append(minPoint);
                }
            }

            painter.save();
            painter.setPen(QPen(Qt::red));
            painter.setFont(QFont("Helvetica [Cronyx]", 12, 30));
            painter.drawText(col * rectWidth, row * rowHeight + middleHeight, leadName);
            painter.restore();
            for (int i = 0; i < vecPoints.size() - 1; ++i) {
                painter.drawLine(vecPoints[i], vecPoints[i + 1]);
            }
        }
    }
    painter.restore();
}

void EcgWidget::onGain5Triggered() {
    m_fGain = 5.0;
    update();
}

void EcgWidget::onGain10Triggered() {
    m_fGain = 10.0;
    update();
}

void EcgWidget::onGain20Triggered() {
    m_fGain = 20.0;
    update();
}

void EcgWidget::onSpeed25Triggered() {
    m_iSpeed = 25;
    update();
}

void EcgWidget::onSpeed50Triggered() {
    m_iSpeed = 50;
    update();
}

void EcgWidget::onSpeed100Triggered() {
    m_iSpeed = 100;
    update();
}

void EcgWidget::onAntialiasingTriggered() {
    m_bAntialiasing = !m_bAntialiasing;
    update();
}

void EcgWidget::onOptimizeTriggered() {
    m_bOptimize = !m_bOptimize;
    update();
}

void EcgWidget::onDrawGridTriggered() {
    m_bDrawGrid = !m_bDrawGrid;
    update();
}

void EcgWidget::onDrawPointTriggered() {
    m_bDrawPoint = !m_bDrawPoint;
    update();
}

void EcgWidget::onPrintPreviewTriggered() {
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    printer.setPageSize(QPrinter::Custom);
    printer.setOrientation(QPrinter::Landscape);
    printer.setPaperSize(QPagedPaintDevice::A4);
    QPrintPreviewDialog preview(&printer);
    preview.setWindowFlags(preview.windowFlags() & Qt::WindowCloseButtonHint);
    preview.setWindowTitle("Print Preview");
    QScreen *screen = QGuiApplication::screens()[0];
    int availableWidth = screen->availableGeometry().width();
    int availableHeight = screen->availableGeometry().height();
    preview.setMinimumSize(availableWidth, availableHeight);
    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printPreviewSlot(QPrinter *)));
    preview.exec();
}

void EcgWidget::printPreviewSlot(QPrinter *printer) {
    double dDotsPerMMForPrinter = printer->pageRect().width() / 297.0;  //A4纸的尺寸是297mm*210mm 如果是横着打印 则297为宽度
    double dScale = dDotsPerMMForPrinter / DOTS_PER_MM; //打印机的窗口和桌面窗口的尺寸相比 放大倍数 用于调整文字
    int w = printer->pageRect().width() - 10 * dDotsPerMMForPrinter,    //边框有1cm左右 减去1cm的像素宽度防止绘制时显示不完整
                h = printer->pageRect().height();

    QPainter painter;
    painter.begin(printer);

    drawGrid(painter, w, h, dDotsPerMMForPrinter);
    drawWave(painter, w, h, dDotsPerMMForPrinter, dScale);
    painter.save();
    QPen pen(Qt::blue);
    QFont f("Consolas");
    f.setBold(true);
    painter.setPen(pen);
    painter.setFont(f);
    QString text = QString("Lead:%1 Gain:%2 Speed:%3").arg(m_leadNums).arg(m_fGain).arg(m_iSpeed);
    painter.drawText(10 * dScale, 20 * dScale, text);
    painter.restore();
    painter.end();
}
