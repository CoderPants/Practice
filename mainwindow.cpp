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
    this->setFixedSize(500,300);

    thread = new QThread;
    worker = new Worker(nullptr, SAMPLE_BLOCK);
    queue = new SharedData;

    byteVector.resize(SAMPLE_BLOCK);

    connect(thread, SIGNAL(started()), worker, SLOT(readingSamples()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    qDebug() << "In destructor";
    worker->setStopThread(true);
    thread->quit();
    thread->wait(5000);
    qDebug() << "Thread terminated";

    delete thread;
    delete queue;
    delete worker;
    delete ui;
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


    timer.setInterval(2000);
    connect (&timer, SIGNAL(timeout()), this, SLOT(getSamples()));
    timer.start();
}

void MainWindow::getSamples()
{
    qDebug() << "\nIN GET SAMPLES";
    if(thread->isFinished())
    {
        thread->quit();
        thread->wait(5000);
    }

    if(queue->tryToLock())
    {
        if (!queue->isEmpty())
        {
            byteVector = queue->getQueueElem();

            queue->unlock();

            for(int i = 0; i < SAMPLE_BLOCK; i++)
            {
                number = qSqrt(byteVector[i].real * byteVector[i].real +
                               byteVector[i].im * byteVector[i].im) ;

                if( i == 0)
                {
                    maxSample = number;
                    minSample = number;
                }
                else
                {
                    maxSample = qMax(number, maxSample);
                    minSample = qMin(number, minSample);
                }
            }

            valueStr.setNum(maxSample);
            ui->maxValueLabel->setText(valueStr);

            valueStr.setNum(minSample);
            ui->minValueLabel->setText(valueStr);                    
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
