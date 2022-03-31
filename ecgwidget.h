#ifndef ECGWIDGET_H
#define ECGWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QJsonDocument>
#include <QPainter>
#include <QMenu>
#include <QtPrintSupport/QPrinter>

class EcgWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EcgWidget(QWidget *parent = nullptr);
    ~EcgWidget();

    void ReadEcgFile(QString &file);

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void drawGrid(QPainter &painter, int w, int h, double dDotsPerMM);
    void drawWave(QPainter &painter, int width, int height, double dotsPerMM, double scale = 1.0);

private:
    int m_leadNums; //lead num count: 12 15 18
    int m_sampleRate;   //sample rate: 1000
    QJsonDocument m_waveDoc;
    QMenu *m_pMenu;
    float m_fGain;
    int m_iSpeed;
    bool m_bAntialiasing;
    bool m_bOptimize;//draw ecg graphic more perfect
    bool m_bDrawGrid;
    bool m_bDrawPoint;

signals:

public slots:
    void onGain5Triggered();
    void onGain10Triggered();
    void onGain20Triggered();
    void onSpeed25Triggered();
    void onSpeed50Triggered();
    void onSpeed100Triggered();
    void onAntialiasingTriggered();
    void onOptimizeTriggered();
    void onDrawGridTriggered();
    void onDrawPointTriggered();
    void onPrintPreviewTriggered();
    void printPreviewSlot(QPrinter *printer);
};

#endif // ECGWIDGET_H
