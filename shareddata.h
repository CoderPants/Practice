#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include <QQueue>
#include <QObject>
#include <QDebug>
#include <QVector>
#include <QMutex>

typedef struct Complex
{
    qint8 real;
    qint8 im;
    double fftw;
} Complex;


class SharedData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int length READ length WRITE setLength NOTIFY lengthChanged)
    int m_length;

public:
    explicit SharedData(QObject *parent = nullptr);
    ~SharedData();

    //Length of queue
    inline int length() const { return m_length; }

    inline bool isEmpty() { return sampleQueue.isEmpty(); }

    inline int getQueueSize() { return sampleQueue.size(); }

    inline bool notFull() { return sampleQueue.size() < m_length ? true : false; }

    //Mutex
    inline void lock() { mutex.lock(); }

    inline void unlock() { mutex.unlock(); }

    inline bool tryToLock() { return mutex.try_lock(); }

    //Elements
    QVector <Complex> getQueueElem();

    void setQueueElem(const QVector<Complex>*);

signals:
    void lengthChanged(int length);

    void queueIsReady();

public slots:
    void setLength(int length);

private:
    QQueue <QVector<Complex>> sampleQueue;

    QMutex mutex;
};

#endif // SHAREDDATA_H
