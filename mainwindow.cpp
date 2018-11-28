#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "chrono"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

using namespace std::chrono;


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

    reXAxis = new QValueAxis();
    reYAxis = new QValueAxis();

    reChart->addSeries(reSeries);
    reSeries->setUseOpenGL(true);
    reChart->legend()->hide();
    reChart->setAxisX(reXAxis, reSeries);
    reChart->setAxisY(reYAxis, reSeries);

    reChart->setTheme(QChart::ChartThemeDark) ;

    reChartView = new QChartView(reChart);
    reChartView->setRenderHint(QPainter::HighQualityAntialiasing);
    reChartView->setRubberBand(QChartView::HorizontalRubberBand);

    //Imaginary part graph creation
    imSeries = new QLineSeries();
    imChart = new QtCharts::QChart();

    imXAxis = new QValueAxis();
    imYAxis = new QValueAxis();

    imChart->addSeries(imSeries);
    imSeries->setUseOpenGL(true);
    imChart->legend()->hide();
    imChart->setAxisX(imXAxis, imSeries);
    imChart->setAxisY(imYAxis, imSeries);

    imChart->setTheme(QChart::ChartThemeDark) ;

    imChartView = new QChartView(imChart);
    imChartView->setRenderHint(QPainter::Antialiasing);
    imChartView->setRubberBand(QChartView::HorizontalRubberBand);

    imSeries->setColor(QColor(255, 0, 0, 150));

    //Spectrum graph creation
    specSeries = new QLineSeries();
    specChart = new QtCharts::QChart();

    specXAxis = new QValueAxis();
    specYAxis = new QValueAxis();

    specChart->addSeries(specSeries);
    specSeries->setUseOpenGL(true);
    specChart->legend()->hide();
    specChart->setAxisX(specXAxis, specSeries);
    specChart->setAxisY(specYAxis, specSeries);

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

    //Vector of samples
    byteVector.resize(SAMPLE_BLOCK);

    //Vectors of dots
    imVector.resize(SAMPLE_BLOCK);
    reVector.resize(SAMPLE_BLOCK);
    specVector.resize(SAMPLE_BLOCK);

    //Max and min for xAxis
    reXAxis->setRange(0, SAMPLE_BLOCK);
    imXAxis->setRange(0, SAMPLE_BLOCK);
    specXAxis->setRange(0, SAMPLE_BLOCK);

    //Max and min for yAxis
    maxReSample = -127;
    minReSample = 127;
    reYAxis->setRange(minReSample, maxReSample);

    maxImSample = -127;
    minImSample = 127;
    imYAxis->setRange(minImSample, maxImSample);

    maxSpecSample = -99999;
    minSpecSample = 99999;
    specYAxis->setRange(minSpecSample, maxSpecSample);

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

    delete ui;
}

void MainWindow::fullDots()
{
    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        //real
        reVector[i].setX(i); reVector[i].setY(byteVector[i].real);
        //imaginary
        imVector[i].setX(i); imVector[i].setY(byteVector[i].im);
        //spectrum
        specVector[i].setX(i); specVector[i].setY(byteVector[i].fftw);
    }
}

void MainWindow::drawGraphs()
{
    reSeries->clear();
    imSeries->clear();
    specSeries->clear();

    reSeries->replace(reVector);
    imSeries->replace(imVector);
    specSeries->replace(specVector);

    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        //real
        maxReSample = qMax(maxReSample, byteVector[i].real);
        minReSample = qMin(minReSample, byteVector[i].real);
        //imaginary
        maxImSample = qMax(maxImSample, byteVector[i].im);
        minImSample = qMin(minImSample, byteVector[i].im);
        //spectrum
        maxSpecSample = qMax(maxSpecSample, byteVector[i].fftw);
        minSpecSample = qMin(minSpecSample, byteVector[i].fftw);
    }

    //Checking if max/min was changed
    if(reYAxis->min() > minReSample)
    {
        reYAxis->setMin(minReSample - INDENT);
    }
    if(reYAxis->max() < maxReSample)
    {
        reYAxis->setMax(maxReSample + INDENT);
    }

    reChartView->repaint();

    if(imYAxis->min() > minImSample)
    {
        imYAxis->setMin(minImSample - INDENT);
    }
    if(imYAxis->max() < maxImSample)
    {
        imYAxis->setMax(maxImSample + INDENT);
    }

    imChartView->repaint();

    if(specYAxis->min() > minSpecSample)
    {
        specYAxis->setMin(minSpecSample - INDENT);
    }
    if(specYAxis->max() < maxSpecSample)
    {
        specYAxis->setMax(maxSpecSample + INDENT);
    }

    specChartView->repaint();
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

            auto start = steady_clock::now();
            fullDots();
            drawGraphs();
            auto end = steady_clock::now();
            qDebug() << duration_cast<milliseconds>(end - start).count();
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
