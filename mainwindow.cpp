#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "chrono"
#include <limits>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

using namespace std;
using namespace chrono;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(1665, 1000);

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

    //Fiter for spectrum
    topLine = new QLineSeries();
    bottomLine = new QLineSeries();

    startTopLine = new QPointF();
    endTopLine = new QPointF();

    startBottomLine = new QPointF();
    endBottomLine = new QPointF();

    startTopLine->setX(0);
    endTopLine->setX(SAMPLE_BLOCK);

    startBottomLine->setX(0);
    endBottomLine->setX(SAMPLE_BLOCK);

    specChart->addSeries(specSeries);
    specChart->addSeries(topLine);
    specChart->addSeries(bottomLine);

    specSeries->setUseOpenGL(true);
    topLine->setUseOpenGL(true);
    bottomLine->setUseOpenGL(true);

    specChart->legend()->hide();

    specChart->setAxisX(specXAxis, specSeries);
    specChart->setAxisY(specYAxis, specSeries);

    specChart->setAxisX(specXAxis, topLine);
    specChart->setAxisY(specYAxis, topLine);

    specChart->setAxisX(specXAxis, bottomLine);
    specChart->setAxisY(specYAxis, bottomLine);

    specChart->setTheme(QChart::ChartThemeDark) ;

    specChartView = new QChartView(specChart);
    specChartView->setRenderHint(QPainter::Antialiasing);
    specChartView->setRubberBand(QChartView::HorizontalRubberBand);

    specSeries->setColor(QColor(0, 0, 255, 150));
    topLine->setColor(QColor(255, 255, 240, 150));
    bottomLine->setColor(QColor(255, 255, 240, 150));

    //Adding to the vertical box layout
    areaForOscillogram = new QVBoxLayout();
    areaForOscillogram->setSpacing(0);
    areaForOscillogram->setMargin(0);
    areaForOscillogram->addWidget(reChartView);
    areaForOscillogram->addWidget(imChartView);
    ui->magnitudeGraph->setLayout(areaForOscillogram);

    areaForSpectrum = new QVBoxLayout();
    areaForSpectrum->setSpacing(0);
    areaForSpectrum->setMargin(0);
    areaForSpectrum->addWidget(specChartView);
    ui->spectrumWidget->setLayout(areaForSpectrum);

    //WaterFall graph
    widthWf = ui->waterFallLabel->width();
    heightWf = ui->waterFallLabel->height();
    qDebug() << widthWf << heightWf;

    waterFall = new QImage(widthWf, heightWf, QImage::Format_RGB32);
    ui->waterFallLabel->setPixmap(QPixmap::fromImage(*waterFall));

    //Because, sometimes top and bottom color can be (0, 0, 0)
    averageColor = new QColor(1, 1, 1);
    topColor = new QColor(255, 0, 0);
    bottomColor = new QColor(0, 0, 255);

    pixelLine = 0;

    //Filters for the waterfall
    topFilter = 0.0;
    bottomFilter = 0.0;

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

    //Fps
    fpsCount = 0;
    fps = 0;

    //Status bar
    QFont serifFont("Times", 15, QFont::Bold);
    ui->statusBar->setFont(serifFont);
    ui->statusBar->showMessage("FPS  0");

    //Spinbox
    ui->topSpinBox->setMaximum(10000);
    ui->topSpinBox->setMinimum(0);

    ui->bottomSpinBox->setMaximum(0);
    ui->bottomSpinBox->setMinimum(-10000);

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
    delete reXAxis;
    delete reYAxis;
    delete reSeries;
    delete reChart;
    delete reChartView;

    imChart->removeSeries(imSeries);
    delete imXAxis;
    delete imYAxis;
    delete imSeries;
    delete imChart;
    delete imChartView;

    specChart->removeSeries(specSeries);
    specChart->removeSeries(topLine);
    specChart->removeSeries(bottomLine);
    delete specXAxis;
    delete specYAxis;
    delete startTopLine;
    delete endTopLine;
    delete startBottomLine;
    delete endBottomLine;
    delete specSeries;
    delete topLine;
    delete bottomLine;
    delete specChart;
    delete specChartView;

    delete areaForOscillogram;

    delete waterFall;

    delete ui;
}

void MainWindow::fillDots()
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

/*Drawing 2 lines for filters
*Top and bottom one
*If it changed, redrew it
*Don't know, how to use qline, that's why - series
*/
void MainWindow::drawFilters()
{
    if(fabs(startTopLine->y() - topFilter) > EPSILON)
    {
        topLine->clear();
        startTopLine->setY(topFilter);
        endTopLine->setY(topFilter);
        *topLine << *startTopLine << *endTopLine;
    }

    if(fabs(startBottomLine->y() - bottomFilter) > EPSILON)
    {
        bottomLine->clear();
        startBottomLine->setY(bottomFilter);
        endBottomLine->setY(bottomFilter);
        *bottomLine << *startBottomLine << *endBottomLine;
    }

}

void MainWindow::setPixelLine(const int x, const int y, QColor *color, const double fftw, const int position)
{
    switch(position)
    {
    //if it's above topFilter
    case(1):
        if(fftw - topFilter > 255)
        {
            color->setRgb(255, 0, 0);
        }
        else
        {
            color->setRgb(static_cast<int>(fftw - topFilter), 0, 0);
        }
        break;

    //if it's below bottomFilter
    case(2):
        if(fabs(fftw - bottomFilter) > 255)
        {
            color->setRgb(0, 0, 255);
        }
        else
        {
            color->setRgb(0, 0, static_cast<int>(fabs(fftw - bottomFilter)));
        }
        break;

    default:
        break;

    }
    for(int i = 0; i < PIXEL_LINE_WIDTH; ++i)
    {
        waterFall->setPixel(x, y+i, color->rgb());
    }
}


/*Drawing waterfall
*Using lines of 5 pixels
*Checking for the same color, and for the unchanged filters
*If in the middle of the filters -> black color
*If above topFilter -> red color
*If below bottomFilter -> blue color
*Drawing till the end of the label
*then, zero iterator
*/

void MainWindow::drawWaterfall()
{
    for(int i = 0; i < SAMPLE_BLOCK; i++)
    {
        //In the middle of the filters -> black color
        if( byteVector[i].fftw < topFilter &&
                byteVector[i].fftw > bottomFilter &&
                checkColorPixel(getX(i), pixelLine, averageColor->rgb()) )
        {
              setPixelLine(getX(i), pixelLine, averageColor, byteVector[i].fftw, 0);
        }

        //Above top filter -> red color
        if( fabs(topFilter) >= EPSILON &&
                byteVector[i].fftw >= topFilter &&
                checkColorPixel(getX(i), pixelLine, topColor->rgb()) )
        {
            setPixelLine(getX(i), pixelLine, topColor, byteVector[i].fftw, 1);
        }

        //Below bottom filter -> blue color
        if( fabs(bottomFilter) >= EPSILON &&
                byteVector[i].fftw <= bottomFilter &&
                checkColorPixel(getX(i), pixelLine, bottomColor->rgb()) )
        {
            setPixelLine(getX(i), pixelLine, bottomColor, byteVector[i].fftw, 2);
        }
    }

    ui->waterFallLabel->setPixmap(QPixmap::fromImage(*waterFall));

    pixelLine += PIXEL_STEP;

    if(pixelLine == heightWf)
    {
        pixelLine = 0;
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

    //Filter lines
    drawFilters();

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

    //Update faster, then repaint
    reChartView->update();

    if(imYAxis->min() > minImSample)
    {
        imYAxis->setMin(minImSample - INDENT);
    }
    if(imYAxis->max() < maxImSample)
    {
        imYAxis->setMax(maxImSample + INDENT);
    }

    imChartView->update();

    if(specYAxis->min() > minSpecSample)
    {
        specYAxis->setMin(minSpecSample - INDENT);
    }
    if(specYAxis->max() < maxSpecSample)
    {
        specYAxis->setMax(maxSpecSample + INDENT);
    }

    specChartView->update();

    //Fps counting
    fpsCount++;

    //Fps printing
    elapsed = m_fpsTimer.elapsed();
    if(elapsed >= 1000)
    {
        elapsed = m_fpsTimer.restart();
        fps = qreal(0.1 * int(10000.0 * (qreal(fpsCount) / qreal(elapsed))));
        ui->statusBar->showMessage("FPS " + QString::number(fps, 'f', 1));
        fpsCount = 0;
    }
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

    m_fpsTimer.start();
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
            fillDots();
            drawGraphs();

            //If filters != 0
            if(fabs(topFilter - bottomFilter) >= EPSILON)
            {
                drawWaterfall();
            }

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
    switch(checkFlag)
    {
    //If checkFlag = false
    case(0):
        worker->setRecurse(false);
        break;

    //If checkFlag = true
    case(2):
        worker->setRecurse(true);
        break;

    default:
        break;
    }
}

void MainWindow::on_topSpinBox_valueChanged(double topDot)
{
    topFilter = topDot;
}

void MainWindow::on_bottomSpinBox_valueChanged(double bottomDot)
{
    bottomFilter = bottomDot;
}

