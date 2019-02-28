#ifndef CACHEUPDATETHREAD_H
#define CACHEUPDATETHREAD_H

#include <QThread>
#include "pagelabel.h"
#include "externalrenderer.h"

enum Renderer {
    poppler = 0,
    custom = 1,
};

class CacheUpdateThread : public QThread
{
    Q_OBJECT

private:
    Renderer mode = Renderer::poppler;
    QString renderCommand = "";
    QString presFileName = "";
    QString noteFileName = "";
    QStringList renderArguments = {};
    PageLabel const* presLabel;
    PageLabel const* noteLabel;
    PageLabel const* smallLabel;
    Poppler::Page const* presPage;
    Poppler::Page const* notePage;

public:
    CacheUpdateThread(QObject* parent=nullptr) : QThread(parent) {}
    void setCustomRenderer(QString const& renderCommand, QString const& presFileName, QString const& noteFileName, QStringList const& renderArguments, Renderer renderer = Renderer::custom);
    void setLabels(PageLabel const* pres, PageLabel const* note, PageLabel const* small);
    void setPages(Poppler::Page const* pres, Poppler::Page const* note);
    void setRenderer(Renderer renderer) {mode=renderer;}
    bool hasRenderCommand() {return !renderCommand.isEmpty();}
    void run() override;

signals:
    void resultsReady(QByteArray const* pres, QByteArray const* note, QByteArray const* small, int const index);
};

#endif // CACHEUPDATETHREAD_H
