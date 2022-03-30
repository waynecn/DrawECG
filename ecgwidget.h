#ifndef ECGWIDGET_H
#define ECGWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QJsonDocument>
#include <QPainter>
#include <QMenu>

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
    void drawWave(QPainter &painter, int width, int height);

private:
    int m_leadNums; //lead num count: 12 15 18
    int m_sampleRate;   //sample rate: 1000
    QJsonDocument m_waveDoc;
    QMenu *m_pMenu;
    float m_fGain;
    int m_iSpeed;
    bool m_bAntialiasing;

signals:

public slots:
    void onGain5Triggered();
    void onGain10Triggered();
    void onGain20Triggered();
    void onSpeed25Triggered();
    void onSpeed50Triggered();
    void onSpeed100Triggered();
    void onAntialiasingTriggered();
};

#endif // ECGWIDGET_H
