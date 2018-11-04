#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <unistd.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QtMath>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(500,300);

    thread = new QThread;
    worker = new Worker;
    queue = new SharedData;

    byteVector.resize(worker->SAMPLE_BLOCK);

    connect(thread, SIGNAL(started()), worker, SLOT(readingSamples()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    qDebug() << "In destructor";
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
    worker->setQueue(queue);
    worker->moveToThread(thread);

    queue->setRunning(true);
    queue->setLength(10);

    thread->start();

    //Replacement of signals/slots
    while(queue->getRunning())
    {
        getSamples();
    }
}

void MainWindow::getSamples()
{
    qDebug() << "\nIN GET SAMPLES";
    sleep(2);

    if(queue->tryToLock())
    {
        if (!queue->isEmpty())
        {
            byteVector = queue->getQueueElem();

            //That's why we're going from the first index
            number = qSqrt(byteVector[0].real * byteVector[0].real +
                           byteVector[0].im * byteVector[0].im);

            maxSample = number;
            minSample = number;

            for(int i = 1; i < worker->SAMPLE_BLOCK; i++)
            {
                number = qSqrt(byteVector[i].real * byteVector[i].real +
                               byteVector[i].im * byteVector[i].im);
                maxSample = qMax(number, maxSample);
                minSample = qMin(number, minSample);
            }

            valueStr.setNum(maxSample);
            ui->maxValueLabel->setText(valueStr);

            valueStr.setNum(minSample);
            ui->minValueLabel->setText(valueStr);
        }
        else
        {
            qDebug() << "QUEUE IS EMPTY!";
            queue->setRunning(false);
        }

        queue->unlock();
    }
    else
    {
        qDebug() << "Didn't lock!";
    }
}
