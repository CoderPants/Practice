#ifndef WORKER_H
#define WORKER_H

#include <QDataStream>
#include <QObject>
#include <QQueue>
#include <QVector>
#include <QFile>

#include "shareddata.h"

#define sampleBlock 8192

class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)

public:
    explicit Worker(QObject *parent = nullptr);

    ~Worker();

    QString filePath() const;

    void openFile();

    void setQueue(SharedData *queue);

    void checkRunning();

signals:
    void finished();

    void filePathChanged(QString filePath);

public slots:
    void readingSamples();

    void setFilePath(QString filePath);

private:
    SharedData *queue;

    QFile audioFile;

    QString m_filePath;

    quint8 byte;

    complex number;

    QVector <complex> byteVector;

    QDataStream stream;

    int i;
};

#endif // WORKER_H
