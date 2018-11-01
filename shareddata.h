#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include <QQueue>
#include <QObject>
#include <QDebug>
#include <QVector>
#include <QMutex>

#define sampleBlock 8192

typedef struct complex
{
    qint8 real;
    qint8 im;
} complex;


class SharedData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int length READ length WRITE setLength NOTIFY lengthChanged)
    int m_length;

public:
    explicit SharedData(QObject *parent = nullptr);

    int length() const;

    bool isEmpty();
    int getQueueSize();

    void lock();
    void unlock();
    bool tryToLock();

    QVector <complex> getQueueElem();
    void setQueueElem(QVector<complex>*);

signals:
    void lengthChanged(int length);

    void queueIsReady();

public slots:
    void setLength(int length);

private:
    QQueue <QVector<complex>> sampleQueue;
    QMutex mutex;
};

#endif // SHAREDDATA_H
