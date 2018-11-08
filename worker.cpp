#include "worker.h"
#include <QDebug>
#include <QThread>

#include <unistd.h>

Worker::Worker(QObject *parent, const int amount) : QObject(parent), sampleBlock(amount)
{
    byteVector.resize(sampleBlock);
    isRecurse = false;
    i = 0;
}

Worker::~Worker()
{
    stream.~QDataStream();
    audioFile.close();
}

void Worker::openFile()
{
    //If file new
    if(audioFile.fileName() != m_filePath)
    {
        audioFile.close();
        audioFile.setFileName(m_filePath);
        audioFile.open(QFile::ReadOnly);

        stream.setDevice(&audioFile);
        stream.setVersion(QDataStream::Qt_5_11);
        stream.setByteOrder(QDataStream::BigEndian);
    }
    else
    {
        audioFile.seek(0);
    }
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
            //Collecting sample block
            for( i = 0; i < sampleBlock && !stream.atEnd(); i++)
            {
                stream >> byte;
                number.real = static_cast<qint8>(byte-127);

                stream >> byte;
                number.im = static_cast<qint8>(byte-127);

                byteVector[i] = number;
            }

            queue->lock();
            queue->setQueueElem(&byteVector);
            queue->unlock();

            if(stream.atEnd())
            {
                if(isRecurse)
                {
                    audioFile.seek(0);
                }
                else
                {
                    qDebug() << "\n\nEND OF THE FILE!\n\n";
                    emit(finished());
                    break;
                }
            }

            sleep(1);

        }
    }
}
