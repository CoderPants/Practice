#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "shareddata.h"
#include "worker.h"

#include <fftw3.h>

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

    void drawOscillogram(QVector <Complex> *byteVector);

    void calcFFTW(QVector <Complex> *byteVector);

    void drawSpectrum();

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

    QChart* imChart;
    QChartView* imChartView;
    QLineSeries* imSeries;

    QChart* specChart;
    QChartView* specChartView;
    QLineSeries* specSeries;

    QVBoxLayout *areaForWidget;

    //Working with dots
    qint8 maxSample;
    qint8 minSample;
    double maxSpecSample;
    double minSpecSample;

    double *samples;
    fftw_plan plan_forward;
    fftw_complex *complexSpec;

    //Working with queue elem
    QVector <Complex> byteVector;
    const int SAMPLE_BLOCK = 8192;

    //For thread freezing
    QTimer timer;

    //Const for Timer
    const int WAIT_TIME = 2000;
    //Const for wait()
    const quint32 WORKING_TIME = 5000;
};

#endif // MAINWINDOW_H
