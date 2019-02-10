#include "cacheupdatethread.h"

void CacheUpdateThread::run()
{
    QPixmap pres = presLabel->getPixmap(presPage);
    QPixmap notes = noteLabel->getPixmap(notePage);
    QPixmap small = smallLabel->getPixmap(presPage);
    emit resultsReady(pres, notes, small, presPage->index());
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
