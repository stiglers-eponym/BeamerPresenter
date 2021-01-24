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
    switch (preferences().pdf_engine)
    {
    case PdfDocument::PopplerEngine:
        document = new PopplerDocument(filename);
        break;
    case PdfDocument::MuPdfEngine:
        document = new MuPdfDocument(filename);
        break;
    }
#else
    document = new PopplerDocument(filename);
#endif
#else
    document = new MuPdfDocument(filename);
#endif
    if (document == NULL || !document->isValid())
        qFatal("Loading document failed");

    // This is mainly for testing:
    document->loadOutline();
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

void PdfMaster::receiveAction(const Action action)
{
    switch (action)
    {
    case ReloadFiles:
        if (loadDocument())
            // TODO: implement update
            emit update();
        break;
    case UndoDrawing:
    case UndoDrawing | PagePart::LeftHalf:
    case UndoDrawing | PagePart::RightHalf:
    {
        // TODO: enable undo on different page parts
        const int page = preferences().page | (action ^ UndoDrawing);
        PathContainer* const path = paths.value(page);
        if (path)
        {
            qDebug() << "undo:" << path;
            auto scene_it = scenes.cbegin();
            while ( scene_it != scenes.cend() && ( (*scene_it)->getPage() | (*scene_it)->pagePart() ) != page)
                ++scene_it;
            if (scene_it == scenes.cend())
                path->undo();
            else
                path->undo(*scene_it);
        }
        break;
    }
    case RedoDrawing:
    case RedoDrawing | PagePart::LeftHalf:
    case RedoDrawing | PagePart::RightHalf:
    {
        const int page = preferences().page | (action ^ RedoDrawing);
        PathContainer* const path = paths.value(page);
        if (path)
        {
            qDebug() << "redo:" << path;
            auto scene_it = scenes.cbegin();
            while ( scene_it != scenes.cend() && ( (*scene_it)->getPage() | (*scene_it)->pagePart() ) != page)
                ++scene_it;
            if (scene_it == scenes.cend())
                path->redo();
            else
                path->redo(*scene_it);
        }
        break;
    }
    case ClearDrawing:
    case ClearDrawing | PagePart::LeftHalf:
    case ClearDrawing | PagePart::RightHalf:
    {
        PathContainer* const path = paths.value(preferences().page | (action ^ ClearDrawing));
        qDebug() << "clear:" << path;
        if (path)
            path->clearPaths();
        break;
    }
    default:
        break;
    }
}

void PdfMaster::resolveLink(const int page, const QPointF &position) const
{
    // First try to resolve navigation link.
    const PdfLink link = document->linkAt(page, position);
    if (link.type >= 0 && link.type < document->numberOfPages())
        emit navigationSignal(link.type);
    // Next try to handle multimedia annotation.
    const VideoAnnotation annotation = document->annotationAt(page, position);
    if (annotation.mode != VideoAnnotation::Invalid)
        qDebug() << annotation.file << annotation.mode << annotation.rect;
}

void PdfMaster::receiveNewPath(const int page, QGraphicsItem *item)
{
    if (!paths.contains(page))
        paths[page] = new PathContainer();
    paths[page]->append(item);
}

void PdfMaster::distributeNavigationEvents(const int page) const
{
    // Map (shifted) page numbers with page parts to slide scenes.
    // Like this it can be detected if multiple scenes want to show the same
    // page. In this case the SlideViews showing the same page will all be
    // connected to the same scene.
    QMap<int, SlideScene*> scenemap;
    for (const auto scene : qAsConst(scenes))
    {
        const int scenepage = overlaysShifted(page, scene->getShift()) | scene->pagePart();
        if (scenemap.contains(scenepage))
            scene->navigationEvent(scenepage & ~NotFullPage, scenemap[scenepage]);
        else
        {
            scenemap[scenepage] = scene;
            scene->navigationEvent(scenepage & ~NotFullPage);
        }
    }
}
