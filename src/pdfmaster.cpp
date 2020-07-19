#include "src/pdfmaster.h"
#ifdef INCLUDE_POPPLER
#include "src/rendering/popplerdocument.h"
#endif
#ifdef INCLUDE_MUPDF
#include "src/rendering/mupdfdocument.h"
#endif

PdfMaster::PdfMaster(const QString &filename)
{
    // Load the document
#ifdef INCLUDE_POPPLER
#ifdef INCLUDE_MUPDF
    switch (preferences().pdf_backend)
    {
    case PdfDocument::PopplerBackend:
        document = new PopplerDocument(filename);
        break;
    case PdfDocument::MuPdfBackend:
        document = new MuPdfDocument(filename);
        break;
    }
#else
    document = new PopplerDocument(filename);
#endif
#else
    document = new MuPdfDocument(filename);
#endif
    if (document == nullptr || !document->isValid())
        qFatal("Loading document failed");
}

PdfMaster::~PdfMaster()
{
    qDeleteAll(paths);
    paths.clear();
    delete document;
}

const QSizeF PdfMaster::getPageSize(const int page_number) const
{
    return document->pageSize(page_number);
}

bool PdfMaster::loadDocument()
{
    return document->loadDocument();
}

void PdfMaster::updatePaths(const SlideView* sender)
{
    // TODO!
}

void PdfMaster::receiveAction(const Action action)
{
    switch (action)
    {
    case ReloadFiles:
        if (loadDocument())
            // TODO: implement update
            emit update();
        break;
    default:
        break;
    }
}

void PdfMaster::resolveLink(const int page, const QPointF &position) const
{
    const PdfLink link = document->linkAt(page, position);
    if (link.type >= 0)
        emit nagivationSignal(link.type);
}
