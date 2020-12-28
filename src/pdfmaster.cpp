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
    {
        // TODO: this takes the wrong page part if page aspect is below threshold.
        PathContainer* const path = paths.value(preferences().page | preferences().page_part);
        if (path)
        {
            qDebug() << "undo:" << path;
            const int page = preferences().page | preferences().page_part;
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
    {
        PathContainer* const path = paths.value(preferences().page | preferences().page_part);
        if (path)
        {
            qDebug() << "redo:" << path;
            const int page = preferences().page | preferences().page_part;
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
    {
        PathContainer* const path = paths.value(preferences().page | preferences().page_part);
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
    {
        writable_preferences().page = link.type;
        emit nagivationSignal(link.type);
    }
    // Next try to handle multimedia annotation.
    const VideoAnnotation annotation = document->annotationAt(page, position);
    if (annotation.mode != VideoAnnotation::Invalid)
        qDebug() << annotation.file << annotation.mode << annotation.rect;
}

void PdfMaster::receiveNewPath(const int page, QGraphicsItem *item)
{
    if (!paths.contains(page))
        paths[page] = new PathContainer(this);
    paths[page]->append(item);
}

void PdfMaster::distributeNavigationEvents(const int page) const
{
    // Map (shifted) page numbers with page parts to slide scenes.
    // Like this it can be detected if multiple scenes want to show the same
    // page. In this case the SlideViews showing the same page will all be
    // connected to the same scene.
    QMap<int, SlideScene*> scenemap;
    for (const auto scene : scenes)
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

void PdfMaster::limitHistoryInvisible(const int page) const
{
    // A nagivation event moves preferences().page away from the given page.
    // Tell paths[page] that it's history should now be limited by
    // preferences().history_length_hidden_slides.
    PathContainer *const container = paths.value(page);
    if (container)
        container->clearHistory(preferences().history_length_hidden_slides);
}
