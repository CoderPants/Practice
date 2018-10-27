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
    QMutexLocker locker(&mutex);
    qDebug() << "Thread in Shared: " << QThread::currentThread();
    sampleQueue.enqueue(*byteVector);

    if(m_length == sampleQueue.size())
    {
        mutex.unlock();
        emit(queueIsReady());
    }
}

void SharedData::end()
{
    emit(queueIsReady());
}
