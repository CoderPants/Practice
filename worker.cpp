#include "worker.h"
#include <QDebug>
#include <QThread>

#include <unistd.h>

Worker::Worker(QObject *parent, const int amount) : QObject(parent), sampleBlock(amount)
{
    byteVector.resize(sampleBlock);
    isRecurse = false;
    stopThread = false;
    i = 0;

    //FFTW
    specSamples = new double[sampleBlock];
    complexSpec =
            (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (sampleBlock*2));
}

Worker::~Worker()
{
    delete [] specSamples;
    fftw_free(complexSpec);

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

void Worker::calcFFTW()
{
    for(int i = 0; i < sampleBlock; i++)
    {
        complexSpec[i][0] = byteVector[i].real;
        complexSpec[i][1] = byteVector[i].im;
    }

    plan_forward = fftw_plan_dft_c2r_1d (sampleBlock, complexSpec,
                                          specSamples, FFTW_ESTIMATE);

    fftw_execute (plan_forward);
    fftw_destroy_plan(plan_forward);

    for(int i = 0; i < sampleBlock; i++)
    {
        byteVector[i].fftw = specSamples[i];
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
        if(stopThread)
        {
            break;
        }

        if(queue->notFull())
        {
            //Collecting sample block of real and imaginary
            for( i = 0; i < sampleBlock && !stream.atEnd(); i++)
            {
                stream >> byte;
                number.real = static_cast<qint8>(byte-127);

                stream >> byte;
                number.im = static_cast<qint8>(byte-127);

                byteVector[i] = number;
            }

            //Collecting fftw samples
            calcFFTW();

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

            sleep(SLEEP_TIME);

        }
    }
}
