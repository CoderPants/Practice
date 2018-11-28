#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "shareddata.h"
#include "worker.h"

#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QtCore/QtMath>
#include <QtCharts/QChartView>
#include <QHBoxLayout>

#include <QMainWindow>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QTimer>
#include <QTime>



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

    void drawGraphs();


private slots:
    void on_loadFileBtn_clicked();

    void on_startCountingBtn_clicked();

    void getSamples();

    void on_recurseReadingFlag_stateChanged(int checkFlag);

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
    const int WAIT_TIME = 2000;

    //Const for wait()
    const quint32 WORKING_TIME = 5000;

    //Const for graphs
    const int INDENT = 2;

    //Dots vectors
    QVector <QPointF> imVector, reVector, specVector;

};

#endif // MAINWINDOW_H
