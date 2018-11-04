#include "worker.h"
#include <QDebug>
#include <QThread>

#include <unistd.h>

Worker::Worker(QObject *parent, const int amount) : QObject(parent), SAMPLE_BLOCK(amount)
{
    byteVector.resize(SAMPLE_BLOCK);
    i = 0;
}

Worker::~Worker()
{
    stream.~QDataStream();
    audioFile.close();
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
        if(queue->notFull())
        {
            if(stream.atEnd())
            {
                qDebug() << "\n\nEND OF THE FILE!\n\n";
                emit(finished());

                //Not sure at all
                qDebug() << audioFile.pos();
                audioFile.seek(0);
                qDebug() << audioFile.pos();
            }

            if(i == SAMPLE_BLOCK)
            {
                queue->lock();
                queue->setQueueElem(&byteVector);
                queue->unlock();

                sleep(1);
                i = 0;
            }

            stream >> byte;
            number.real = static_cast<qint8>(byte-127);

            stream >> byte;
            number.im = static_cast<qint8>(byte-127);

            byteVector[i] = number;

            i++;
        }
    }
}
