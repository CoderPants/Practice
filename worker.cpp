#include "worker.h"
#include <QDebug>
#include <QThread>

#include <unistd.h>


Worker::Worker(QObject *parent) : QObject(parent)
{
    locker = new SharedData;
    byteVector.resize(sampleBlock);
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
        locker->lock();
        if(stream.atEnd())
        {
            locker->unlock();
            break;
        }

        if(i == sampleBlock)
        {
            qDebug() << "Thread in worker: " << QThread::currentThread();
            locker->unlock();
            emit(sendQueueElem(&byteVector));

            //Synchronization of thread
            //Witout it, will be collapse of threads
            sleep(1);
            i = 0;
        }

        stream >> byte;
        number.real = static_cast<qint8>(byte-127);

        stream >> byte;
        number.im = static_cast<qint8>(byte-127);

        byteVector[i] = number;

        i++;
        locker->unlock();
    }
    //To shareddata
    emit(fileEnd());
    //To main
    emit(finished());
}
