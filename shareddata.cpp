#include "shareddata.h"

SharedData::SharedData(QObject *parent) : QObject(parent)
{
    m_length = 0;
}

SharedData::~SharedData()
{

}

void SharedData::setLength(int length)
{
    if (m_length == length)
        return;

    m_length = length;
    emit lengthChanged(m_length);
}

QVector<Complex> SharedData::getQueueElem()
{
    qDebug() << "Queue size IN GETTER" << sampleQueue.size() << " Max size " << m_length;
    return sampleQueue.dequeue();
}

void SharedData::setQueueElem(const QVector<Complex> *byteVector)
{
    sampleQueue.enqueue(*byteVector);
    qDebug() << "Queue size in Setter" << sampleQueue.size() << " Max size " << m_length;
}

