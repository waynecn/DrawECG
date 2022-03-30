#include "ecgwidget.h"
#include "common.h"

#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

EcgWidget::EcgWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_sampleRate = 1000;
    m_fGain = 10.0;
    m_iSpeed = 50;
    m_bAntialiasing = false;

    m_pMenu = new QMenu();
    QMenu *gain = m_pMenu->addMenu("Gain");
    QAction *gain5 = gain->addAction("5");
    QAction *gain10 = gain->addAction("10");
    QAction *gain20 = gain->addAction("20");
    connect(gain5, SIGNAL(triggered()), this, SLOT(onGain5Triggered()));
    connect(gain10, SIGNAL(triggered()), this, SLOT(onGain10Triggered()));
    connect(gain20, SIGNAL(triggered()), this, SLOT(onGain20Triggered()));

    m_pMenu->addSeparator();

    QMenu *speed = m_pMenu->addMenu("Speed");
    QAction *speed25 = speed->addAction("25");
    QAction *speed50 = speed->addAction("50");
    QAction *speed100 = speed->addAction("100");
    connect(speed25, SIGNAL(triggered()), this, SLOT(onSpeed25Triggered()));
    connect(speed50, SIGNAL(triggered()), this, SLOT(onSpeed50Triggered()));
    connect(speed100, SIGNAL(triggered()), this, SLOT(onSpeed100Triggered()));

    m_pMenu->addSeparator();

    QAction *smoothAct = m_pMenu->addAction("Smooth");
    smoothAct->setCheckable(true);
    smoothAct->setChecked(false);
    connect(smoothAct, SIGNAL(triggered()), this, SLOT(onAntialiasingTriggered()));
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
    drawWave(painter, w, h);
    event->accept();
}

void EcgWidget::drawWave(QPainter &painter, int width, int height) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, m_bAntialiasing);
    float fNumPerPixel = m_sampleRate * 1.0 / (m_iSpeed * DOTS_PER_MM);
    float fPixelPerMv = m_fGain * DOTS_PER_MM;
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
            for (int i = 0; i < waveLen; ++i) {
                if (i / fNumPerPixel > rectWidth) {
                    break;
                }
                QPointF p((qreal)(col * rectWidth + i / fNumPerPixel), (qreal)(row * rowHeight + middleHeight - (waveArr[i].toInt() * fPixelPerUv)));
                vecPoints.append(p);
            }

            painter.save();
            painter.setPen(QPen(Qt::red));
            painter.setFont(QFont("Helvetica [Cronyx]", 12, 30));
            painter.drawText(vecPoints[0], leadName);
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
