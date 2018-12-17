#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "shareddata.h"
#include "worker.h"

#include <QtCharts/QLineSeries>
#include <QtCore/QLine>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtCore/QtMath>
#include <QtCharts/QChartView>
#include <QHBoxLayout>

#include <QMainWindow>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QtCore/QTimer>
#include <QtCore/QElapsedTimer>


namespace Ui {
class MainWindow;
}

QT_CHARTS_USE_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void fullDots();

    void drawFilters();

    void drawWaterfall();

    void drawGraphs();


private slots:
    void on_loadFileBtn_clicked();

    void on_startCountingBtn_clicked();

    void getSamples();

    void on_recurseReadingFlag_stateChanged(int checkFlag);

    void on_topSpinBox_valueChanged(double topLine);

    void on_bottomSpinBox_valueChanged(double bottomLine);

private:
    Ui::MainWindow *ui;
    Worker *worker;
    QThread *thread;
    SharedData *queue;

    //For graphics
    QChart* reChart;
    QChartView* reChartView;
    QLineSeries* reSeries;
    QValueAxis* reXAxis;
    QValueAxis* reYAxis;

    QChart* imChart;
    QChartView* imChartView;
    QLineSeries* imSeries;
    QValueAxis* imXAxis;
    QValueAxis* imYAxis;

    QChart* specChart;
    QChartView* specChartView;
    QLineSeries* specSeries;
    QValueAxis* specXAxis;
    QValueAxis* specYAxis;

    //Filters for spectrum
    QLineSeries *topLine, *bottomLine;
    QPointF *startT, *endT;
    QPointF *startB, *endB;

    double topFilter;
    double bottomFilter;

    //Warerfall graph
    QImage *waterFall;
    QColor *averageColor;
    QColor *topColor;
    QColor *bottomColor;

    int pixelLine;
    int widthWf;
    int heightWf;

    //Step with which we reduce number of pixels, drawn on the image
    const int REDUCTION = 7;

    //Step for waterfall lines 8 pixels
    const int pixelStep = 8;

    QVBoxLayout *areaForWidget;

    //Working with dots
    qint8 maxImSample;
    qint8 minImSample;

    qint8 maxReSample;
    qint8 minReSample;

    double maxSpecSample;
    double minSpecSample;

    //Working with queue elem
    QVector <Complex> byteVector;
    const int SAMPLE_BLOCK = 8192;

    //For thread freezing
    QTimer timer;

    //Const for Timer
    const int WAIT_TIME = 40;

    //Const for wait()
    const quint32 WORKING_TIME = 5000;

    //Const for graphs
    const int INDENT = 2;

    //Dots vectors
    QVector <QPointF> imVector, reVector, specVector;

    //Fps count
    QElapsedTimer m_fpsTimer;
    qint64 elapsed;
    int fpsCount;
    qreal fps;

};

#endif // MAINWINDOW_H
