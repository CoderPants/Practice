#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <unistd.h>

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
    byteVector.resize(sampleBlock);

    thread = new QThread;
    worker = new Worker;
    queue = new SharedData;

    connect(thread, SIGNAL(started()), worker, SLOT(readingSamples()));
    connect(worker, SIGNAL(sendQueueElem(QVector <complex>*)),
            queue, SLOT(setQueueElem(QVector<complex>*)));
    connect(worker, SIGNAL(fileEnd()), queue, SLOT(end()));
    connect(queue, SIGNAL(queueIsReady()), this, SLOT(getSamples()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    qDebug() << "In destructor";
    //Trying to therminate thread
    thread->quit();
    thread->wait(5000);
    qDebug() << "Thread terminated";

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
    worker->moveToThread(thread);
    queue->setLength(10);
    thread->start();
}

void MainWindow::getSamples()
{
    if(queue->tryToLock())
    {
        qDebug() << "Main Thread: " << QThread::currentThread();
        while (!queue->isEmpty())
        {
            byteVector = queue->getQueueElem();

            number = qSqrt(byteVector[0].real * byteVector[0].real +
                           byteVector[0].im * byteVector[0].im);

            maxSample = number;
            minSample = number;

            for(int i = 1; i < sampleBlock; i++)
            {
                number = qSqrt(byteVector[i].real * byteVector[i].real +
                               byteVector[i].im * byteVector[i].im);
                maxSample = qMax(number, maxSample);
                minSample = qMin(number, minSample);
            }

        }

        valueStr.setNum(maxSample);
        ui->maxValueLabel->setText(valueStr);

        valueStr.setNum(minSample);
        ui->minValueLabel->setText(valueStr);

        queue->unlock();
    }
    else
    {
        qDebug() << "Didn't lock!";
    }
}
