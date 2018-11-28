#ifndef WORKER_H
#define WORKER_H

#include <fftw3.h>

#include <QDataStream>
#include <QObject>
#include <QVector>
#include <QQueue>
#include <QFile>

#include "shareddata.h"

class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)

public:
    explicit Worker(QObject *parent = nullptr, const int amount = 0);
    ~Worker();

    //Working with file
    inline QString filePath() const { return m_filePath; }

    inline void startOfFile() { audioFile.seek(0); }

    void openFile();

    //Queue elem
    inline void setQueue(SharedData *queue) { this->queue = queue; }

    //Flag
    inline void setRecurse(bool flag) { isRecurse = flag; }

    inline bool getRecurse() { return isRecurse; }

    inline void setStopThread(bool flag) { stopThread = flag; }

    //FFTW
    void calcFFTW();


signals:
    void finished();

    void filePathChanged(QString filePath);

public slots:
    void readingSamples();

    void setFilePath(QString filePath);

private:
    //File
    QFile audioFile;
    QString m_filePath;
    QDataStream stream;

    //FFTW
    double *specSamples;
    fftw_plan plan_forward;
    fftw_complex *complexSpec;

    //Queue
    quint8 byte;
    Complex number;
    QVector <Complex> byteVector;
    SharedData *queue;

    //Samples
    int sampleBlock;
    int i;

    //Flag
    bool isRecurse;
    bool stopThread;

    //Const for sleep()
    const quint32 SLEEP_TIME = 20;

};

#endif // WORKER_H
