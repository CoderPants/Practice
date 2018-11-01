#include "shareddata.h"
#include <QThread>

SharedData::SharedData(QObject *parent) : QObject(parent)
{

}

int SharedData::length() const
{
    return m_length;
}

QVector<complex> SharedData::getQueueElem()
{
    qDebug() << "Queue size IN GETTER" << sampleQueue.size() << " Max size " << m_length;
    return sampleQueue.dequeue();
}

bool SharedData::isEmpty()
{
    if(sampleQueue.isEmpty())
    {
        qDebug() << "QUEUE IS EMPTY!!!!";
    }
    return sampleQueue.isEmpty();
}

void SharedData::lock()
{
    mutex.lock();
}

void SharedData::unlock()
{
    mutex.unlock();
}

bool SharedData::tryToLock()
{
    return mutex.try_lock();
}

void SharedData::setLength(int length)
{
    if (m_length == length)
        return;

    m_length = length;
    emit lengthChanged(m_length);
}

void SharedData::setQueueElem(QVector<complex> *byteVector)
{
    mutex.lock();

    qDebug() << "Thread in Shared: " << QThread::currentThread();
    sampleQueue.enqueue(*byteVector);
    qDebug() << "Queue size in Setter" << sampleQueue.size() << " Max size " << m_length;
    mutex.unlock();
    //Don't know another option
    //That's why, emit()
    emit(queueIsReady());

}

int SharedData::getQueueSize()
{
    return sampleQueue.size();
}

