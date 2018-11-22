#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setMinimumSize(1280, 520);

    //App color
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, QColor(169, 169, 169, 150));
    this->setPalette(Pal);

    this->setWindowTitle("Audio file analysis");

    //Real part graph creation
    reSeries = new QLineSeries();

    reChart = new QtCharts::QChart();
    reChart->addSeries(reSeries);
    //reChart->setTitle("Oscillogram real");
    reChart->setAnimationOptions(QChart::SeriesAnimations);
    reChart->legend()->hide();
    reChart->createDefaultAxes();

    reChart->axisX(reSeries)->setMin(0);
    reChart->axisX(reSeries)->setMax(SAMPLE_BLOCK);

    reChart->setTheme(QChart::ChartThemeDark) ;

    reChartView = new QChartView(reChart);
    reChartView->setRenderHint(QPainter::HighQualityAntialiasing);
    reChartView->setRubberBand(QChartView::HorizontalRubberBand);

    //Imaginary part graph creation
    imSeries = new QLineSeries();

    imChart = new QtCharts::QChart();
    imChart->addSeries(imSeries);
    //imChart->setTitle("Oscillogram imaginary");
    imChart->setAnimationOptions(QChart::SeriesAnimations);
    imChart->legend()->hide();
    imChart->createDefaultAxes();

    imChart->axisX(imSeries)->setMin(0);
    imChart->axisX(imSeries)->setMax(SAMPLE_BLOCK);

    imChart->setTheme(QChart::ChartThemeDark) ;

    imChartView = new QChartView(imChart);
    imChartView->setRenderHint(QPainter::Antialiasing);
    imChartView->setRubberBand(QChartView::HorizontalRubberBand);

    imSeries->setColor(QColor(255, 0, 0, 150));

    //Spectrum graph creation
    specSeries = new QLineSeries();

    specChart = new QtCharts::QChart();
    specChart->addSeries(specSeries);
    //specChart->setTitle("Spectrum");
    specChart->setAnimationOptions(QChart::SeriesAnimations);
    specChart->legend()->hide();
    specChart->createDefaultAxes();

    specChart->axisX(specSeries)->setMin(0);
    specChart->axisX(specSeries)->setMax(SAMPLE_BLOCK);

    specChart->setTheme(QChart::ChartThemeDark) ;

    specChartView = new QChartView(specChart);
    specChartView->setRenderHint(QPainter::Antialiasing);
    specChartView->setRubberBand(QChartView::HorizontalRubberBand);

    specSeries->setColor(QColor(0, 0, 255, 150));

    //Adding to the vertical box layout
    areaForWidget = new QVBoxLayout();
    areaForWidget->setSpacing(0);
    areaForWidget->setMargin(0);
    areaForWidget->addWidget(specChartView);
    areaForWidget->addWidget(reChartView);
    areaForWidget->addWidget(imChartView);
    ui->magnitudeGraph->setLayout(areaForWidget);

    //For data in thread
    thread = new QThread;
    worker = new Worker(nullptr, SAMPLE_BLOCK);
    queue = new SharedData;

    byteVector.resize(SAMPLE_BLOCK);

    //For graph
    samples = new double[SAMPLE_BLOCK];
    complexSpec =
            (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (SAMPLE_BLOCK*2));

    //Threads
    connect(thread, SIGNAL(started()), worker, SLOT(readingSamples()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    qDebug() << "In destructor";
    worker->setStopThread(true);
    thread->quit();
    thread->wait(WORKING_TIME);
    qDebug() << "Thread terminated";

    delete thread;
    delete queue;
    delete worker;

    reChart->removeSeries(reSeries);
    delete  reSeries;
    delete reChart;
    delete  reChartView;

    imChart->removeSeries(imSeries);
    delete  imSeries;
    delete imChart;
    delete  imChartView;

    specChart->removeSeries(specSeries);
    delete  specSeries;
    delete specChart;
    delete  specChartView;

    delete [] samples;
    fftw_free(complexSpec);

    delete ui;
}

void MainWindow::drawOscillogram(QVector <Complex> *byteVector)
{
    reSeries->clear();

    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        QPointF dot(i, (*byteVector)[i].real);
        *reSeries << dot;

        if(i == 0)
        {
            maxSample = (*byteVector)[i].real;
            minSample = (*byteVector)[i].real;
        }
        else
        {
            maxSample = qMax(maxSample, (*byteVector)[i].real);
            minSample = qMin(minSample, (*byteVector)[i].real);
        }
    }

    reChart->axisY(reSeries)->setMin(minSample - 2);
    reChart->axisY(reSeries)->setMax(maxSample + 2);

    reChartView->update();

    imSeries->clear();

    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        QPointF dot(i, (*byteVector)[i].im);
        *imSeries << dot;

        if(i == 0)
        {
            maxSample = (*byteVector)[i].im;
            minSample = (*byteVector)[i].im;
        }
        else
        {
            maxSample = qMax(maxSample, (*byteVector)[i].im);
            minSample = qMin(minSample, (*byteVector)[i].im);
        }
    }

    imChart->axisY(imSeries)->setMin(minSample - 2);
    imChart->axisY(imSeries)->setMax(maxSample + 2);

    imChartView->update();
}

void MainWindow::calcFFTW(QVector<Complex> *byteVector)
{
    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        complexSpec[i][0] = (*byteVector)[i].real;
        complexSpec[i][1] = (*byteVector)[i].im;
    }

    plan_forward = fftw_plan_dft_c2r_1d (SAMPLE_BLOCK, complexSpec,
                                          samples, FFTW_ESTIMATE);

    fftw_execute (plan_forward);
    fftw_destroy_plan(plan_forward);
}


void MainWindow::drawSpectrum()
{
    specSeries->clear();

    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        QPointF dot(i, samples[i]);
        *specSeries << dot;

        if(i == 0)
        {
            maxSpecSample = samples[i];
            minSpecSample = samples[i];
        }
        else
        {
            maxSpecSample = qMax(maxSpecSample, samples[i]);
            minSpecSample = qMin(minSpecSample, samples[i]);
        }
    }

    specChart->axisY(specSeries)->setMax(maxSpecSample);
    specChart->axisY(specSeries)->setMin(minSpecSample);

    specChart->update();

}



void MainWindow::on_loadFileBtn_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose the file",
                                                    "/home", "*.wav *.bin");
    ui->filePathLine->setText(filePath);
}

void MainWindow::on_startCountingBtn_clicked()
{
    QString filePath = ui->filePathLine->text();

    if(filePath == "")
    {
        QMessageBox::critical(this, "Error", "File path is missing!");
        return;
    }

    worker->setFilePath(filePath);
    worker->openFile();
    worker->setQueue(queue);
    worker->moveToThread(thread);

    queue->setLength(10);

    thread->start();

    timer.setInterval(WAIT_TIME);
    connect (&timer, SIGNAL(timeout()), this, SLOT(getSamples()));
    timer.start();
}

void MainWindow::getSamples()
{
    qDebug() << "\nIN GET SAMPLES";
    if(thread->isFinished())
    {
        thread->quit();
        thread->wait(WORKING_TIME);
    }

    if(queue->tryToLock())
    {
        if (!queue->isEmpty())
        {
            byteVector = queue->getQueueElem();
            queue->unlock();

            drawOscillogram(&byteVector);
            calcFFTW(&byteVector);
            drawSpectrum();
        }
        else
        {
            queue->unlock();
            timer.stop();
            QMessageBox::information(this, "Info", "End of the file!");
        }
    }
    else
    {
        qDebug() << "Didn't lock!";
    }
}

void MainWindow::on_recurseReadingFlag_stateChanged(int checkFlag)
{
    //If checkFlag = true
    if(checkFlag == 2)
    {
        worker->setRecurse(true);
    }

    //If checkFlag = false
    if(checkFlag == 0)
    {
        worker->setRecurse(false);
    }
}
