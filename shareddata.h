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
} Complex;


class SharedData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int length READ length WRITE setLength NOTIFY lengthChanged)
    int m_length;

public:
    explicit SharedData(QObject *parent = nullptr);

    //Length of queue
    int length() const;
    bool isEmpty();
    bool notFull();
    int getQueueSize();

    //Mutex
    void lock();
    void unlock();
    bool tryToLock();

    //Elements
    QVector <Complex> getQueueElem();
    void setQueueElem(QVector<Complex>*);

    //Flag for reading
    bool getRunning() const;
    void setRunning(bool value);

signals:
    void lengthChanged(int length);

    void queueIsReady();

public slots:
    void setLength(int length);

private:
    QQueue <QVector<Complex>> sampleQueue;
    QMutex mutex;
    bool running;
};

//Realisation of inline
//Mutex
inline void SharedData::lock()
{
    mutex.lock();
}

inline void SharedData::unlock()
{
    mutex.unlock();
}

inline bool SharedData::tryToLock()
{
    return mutex.try_lock();
}


//Length of queue
inline int SharedData::getQueueSize()
{
    return sampleQueue.size();
}

inline int SharedData::length() const
{
    return m_length;
}

inline bool SharedData::isEmpty()
{
    return sampleQueue.isEmpty();
}

inline bool SharedData::notFull()
{
    return sampleQueue.size() < m_length ? true : false;
}


//Flag
inline void SharedData::setRunning(bool value)
{
    running = value;
}

inline bool SharedData::getRunning() const
{
    return running;
}


#endif // SHAREDDATA_H
