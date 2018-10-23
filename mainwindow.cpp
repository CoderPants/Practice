#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(500,300);
}

MainWindow::~MainWindow()
{
    qDebug() << "In destructor";
    thread->quit();
    thread->wait(5000);
    qDebug() << "Thread terminated";

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

    thread = new QThread;
    worker = new Worker(nullptr, filePath);
    worker->setRunning(true);

    worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), worker, SLOT(readingSamples()));
    connect(worker, SIGNAL(sendSamples(QQueue <qint8>*)),
            this, SLOT(getSamples(QQueue <qint8>*)));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));

    thread->start();

}

void MainWindow::getSamples(QQueue<qint8> *sampleQueue)
{
    worker->setRunning(false);

    qDebug() << "In function";
    qint8 maxSample = 0;
    qint8 minSample = 0;
    qint8 sample = 0;

    qDebug() << "Length of the queue" << sampleQueue->length();

    while(!sampleQueue->isEmpty())
    {
        sample = sampleQueue->dequeue();
        maxSample = qMax(maxSample, sample);
        minSample = qMin(minSample, sample);
    }

    QString value;
    value.setNum(maxSample);
    ui->maxValueLabel->setText(value);

    value.setNum(minSample);
    ui->minValueLabel->setText(value);

    qDebug() << maxSample;
    qDebug() << minSample;

    worker->setRunning(true);
    //connect(this, SIGNAL(startReading()), worker, SLOT(readingSamples()));
    //emit(startReading());
    qDebug() <<  worker->running();
}
