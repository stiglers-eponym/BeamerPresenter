#ifndef CACHEUPDATETHREAD_H
#define CACHEUPDATETHREAD_H

#include <QThread>
#include "pagelabel.h"

class CacheUpdateThread : public QThread
{
    Q_OBJECT
public:
    CacheUpdateThread(QObject* parent=nullptr) : QThread(parent) {}
    void setLabels(PageLabel const* pres, PageLabel const* note, PageLabel const* small);
    void setPages(Poppler::Page const* pres, Poppler::Page const* note);
    void run() override;
private:
    PageLabel const* presLabel;
    PageLabel const* noteLabel;
    PageLabel const* smallLabel;
    Poppler::Page const* presPage;
    Poppler::Page const* notePage;
signals:
    void resultsReady(QByteArray const* pres, QByteArray const* note, QByteArray const* small, int const index);
};

#endif // CACHEUPDATETHREAD_H
