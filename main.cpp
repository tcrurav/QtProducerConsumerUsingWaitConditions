// MODIFIED FROM THIS EXAMPLE: https://doc.qt.io/qt-6/qtcore-threads-waitconditions-example.html

#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QRandomGenerator>
#include <QThread>
#include <QWaitCondition>
#include <iostream>

constexpr int DataSize = 10000;
constexpr int BufferSize = 50;

QMutex mutex; // protects the buffer and the counter
char buffer[BufferSize];
int numUsedBytes;

QMutex writtingMutex; // protects the console so that only one can write at the same time in the console

QWaitCondition bufferNotEmpty;
QWaitCondition bufferNotFull;

class Producer : public QThread
{
public:
    Producer() {}

private:
    void run() override
    {
        for (int i = 0; i < DataSize; ++i) {
            {
                const QMutexLocker locker(&mutex);
                while (numUsedBytes == BufferSize)
                    bufferNotFull.wait(&mutex);
            }

            buffer[i % BufferSize] = "ACGT"[QRandomGenerator::global()->bounded(4)];

            {
                const QMutexLocker locker(&writtingMutex);
                std::cout << "INSERTING..." << buffer[i % BufferSize] << '\t' << i % BufferSize << '\t' << buffer << "\n";
            }

            {
                const QMutexLocker locker(&mutex);
                ++numUsedBytes;
                bufferNotEmpty.wakeAll();
            }
        }
    }
};

class Consumer : public QThread
{
public:
    Consumer(){}

private:
    void run() override
    {
        for (int i = 0; i < DataSize; ++i) {
            {
                const QMutexLocker locker(&mutex);
                while (numUsedBytes == 0)
                    bufferNotEmpty.wait(&mutex);
            }

            char auxCharacter = buffer[i % BufferSize];
            buffer[i % BufferSize] = '_';

            {
                const QMutexLocker locker(&writtingMutex);
                std::cout << "EXTRACTING..." << auxCharacter << '\t' << i % BufferSize << '\t' << buffer << "\n";
            }

            {
                const QMutexLocker locker(&mutex);
                --numUsedBytes;
                bufferNotFull.wakeAll();
            }
        }
        // fprintf(stderr, "\n");
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Producer producer;
    Consumer consumer;
    producer.start();
    consumer.start();
    producer.wait();
    consumer.wait();
    return 0;
}


