#include "cacheupdatethread.h"

void CacheUpdateThread::run()
{
    QPixmap presimg = presLabel->getPixmap(presPage);
    if (isInterruptionRequested())
        return;
    QPixmap noteimg = noteLabel->getPixmap(notePage);
    if (isInterruptionRequested())
        return;
    QPixmap smallimg = smallLabel->getPixmap(presPage);
    if (isInterruptionRequested())
        return;
    QByteArray* pres = new QByteArray();
    {
        QBuffer buffer(pres);
        buffer.open(QIODevice::WriteOnly);
        presimg.save(&buffer, "PNG");
    }
    QByteArray* note = new QByteArray();
    {
        QBuffer buffer(note);
        buffer.open(QIODevice::WriteOnly);
        noteimg.save(&buffer, "PNG");
    }
    QByteArray* small = new QByteArray();
    {
        QBuffer buffer(small);
        buffer.open(QIODevice::WriteOnly);
        smallimg.save(&buffer, "PNG");
    }
    if (isInterruptionRequested()) {
        delete pres;
        delete note;
        delete small;
        return;
    }
    emit resultsReady(pres, note, small, presPage->index());
}

void CacheUpdateThread::setLabels(const PageLabel *pres, const PageLabel *note, const PageLabel *small)
{
    presLabel = pres;
    noteLabel = note;
    smallLabel = small;
}

void CacheUpdateThread::setPages(const Poppler::Page *pres, const Poppler::Page *note)
{
    presPage = pres;
    notePage = note;
}
