#include "pixcachethread.h"

PixCacheThread::PixCacheThread(QObject *parent) : QThread(parent)
{
}

void PixCacheThread::run()
{
    // TODO: render
    emit sendData(data);
}
