#ifndef WORKER_H
#define WORKER_H

#include <QDataStream>
#include <QObject>
#include <QQueue>
#include <QVector>
#include <QFile>

#include "shareddata.h"

class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)

public:
    explicit Worker(QObject *parent = nullptr);
    ~Worker();

    //Working with file
    QString filePath() const;
    void openFile();

    //Samples
    const int SAMPLE_BLOCK = 8192;

    void setQueue(SharedData *queue);

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

    //Queue
    quint8 byte;
    Complex number;
    QVector <Complex> byteVector;
    SharedData *queue;

    //Counter
    int i;
};

//Realisation of inline
inline void Worker::setQueue(SharedData *queue)
{
    this->queue = queue;
}

inline QString Worker::filePath() const
{
    return m_filePath;
}

#endif // WORKER_H
