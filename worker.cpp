#include "worker.h"
#include <QDebug>

#include <unistd.h>

#define sampleBlock 16384

Worker::Worker(QObject *parent, QString str) : QObject(parent), m_filePath(str)
{
    audioFile.setFileName(m_filePath);
    audioFile.open(QIODevice::ReadOnly);

    stream.setDevice(&audioFile);
    stream.setVersion(QDataStream::Qt_5_11);
    stream.setByteOrder(QDataStream::BigEndian);
}

Worker::~Worker()
{
    stream.~QDataStream();
    audioFile.close();
}

bool Worker::running() const
{
    return m_running;
}

QString Worker::filePath() const
{
    return m_filePath;
}

void Worker::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    emit runningChanged(m_running);
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
        if(m_running)
       {
            if(i % sampleBlock == 0 && i != 0)
            {
                qDebug() << "i in worker" << i;
                qDebug() << m_running;
                emit(sendSamples(&sampleQueue));
                sleep(1);
            }

            if(stream.atEnd() || i == sampleBlock*2)
            {
                break;
            }

            stream >> byte;
            sampleQueue.enqueue(static_cast<qint8>(byte-127));

            i++;
        }
        emit(finished());
        //break;
    }
}
