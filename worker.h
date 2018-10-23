#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QQueue>
#include <QFile>
#include <QDataStream>

class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)

    bool m_running;

public:
    explicit Worker(QObject *parent = nullptr, QString str = "");

    ~Worker();

    bool running() const;

    QString filePath() const;

signals:
    void sendSamples(QQueue <qint8> *sampleQueue);

    void finished();

    void runningChanged(bool running);

    void filePathChanged(QString filePath);

public slots:
    void readingSamples();

    void setRunning(bool running);

    void setFilePath(QString filePath);

private:
    QFile audioFile;

    QString m_filePath;

    QQueue <qint8> sampleQueue;

    quint8 byte;

    QDataStream stream;

    int i = 0;
};

#endif // WORKER_H
