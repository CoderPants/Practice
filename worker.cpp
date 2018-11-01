#include "worker.h"
#include <QDebug>
#include <QThread>

#include <unistd.h>


Worker::Worker(QObject *parent) : QObject(parent)
{
    byteVector.resize(sampleBlock);
    i = 0;
}

Worker::~Worker()
{
    stream.~QDataStream();
    audioFile.close();
}

QString Worker::filePath() const
{
    return m_filePath;
}

void Worker::openFile()
{
    audioFile.setFileName(m_filePath);
    audioFile.open(QIODevice::ReadOnly);

    stream.setDevice(&audioFile);
    stream.setVersion(QDataStream::Qt_5_11);
    stream.setByteOrder(QDataStream::BigEndian);
}

void Worker::setQueue(SharedData *queue)
{
    this->queue = queue;
}

void Worker::setFilePath(QString filePath)
{
    if (m_filePath == filePath)
        return;

    m_filePath = filePath;
    emit filePathChanged(m_filePath);
}

void Worker::readingSamples()
{
    forever
    {
        if(queue->getQueueSize() < queue->length())
        {
            queue->lock();
            if(stream.atEnd())
            {
                queue->unlock();
                break;
            }

            if(i == sampleBlock)
            {
                qDebug() << "Thread in worker: " << QThread::currentThread() << "i"<<i;
                queue->unlock();
                queue->setQueueElem(&byteVector);

                sleep(1);
                i = 0;
            }

            stream >> byte;
            number.real = static_cast<qint8>(byte-127);

            stream >> byte;
            number.im = static_cast<qint8>(byte-127);

            byteVector[i] = number;

            i++;
            queue->unlock();
        }
    }
    qDebug()<<"";
    qDebug()<<"";
    qDebug() << "Out of forever";
    //To main
    emit(finished());
}
