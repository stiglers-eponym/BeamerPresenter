// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <map>
#include <algorithm>
#include <zlib.h>
#include <QFileInfo>
#include <QPainter>
#include <QRegularExpression>
#include <QBuffer>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileDialog>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStyleOptionGraphicsItem>

#include "src/config.h"
#include "src/log.h"
#include "src/pdfmaster.h"
#include "src/preferences.h"
#include "src/slidescene.h"
#include "src/drawing/pathcontainer.h"
#include "src/rendering/pdfdocument.h"
#include "src/rendering/abstractrenderer.h"
#ifdef USE_QTPDF
#include "src/rendering/qtdocument.h"
#endif
#ifdef USE_POPPLER
#include "src/rendering/popplerdocument.h"
#endif
#ifdef USE_MUPDF
#include "src/rendering/mupdfdocument.h"
#endif

void PdfMaster::initialize(const QString &filename)
{
    QMimeDatabase db;
    const QMimeType type = db.mimeTypeForFile(filename, QMimeDatabase::MatchContent);
    if (type.name() == "application/pdf")
        loadDocument(filename);
    else if (type.name() == "application/xml" || type.name() == "application/gzip")
        loadXopp(filename);
    else
    {
        qCritical() << tr("File type of document not understood! Trying to load it anyway.");
        loadDocument(filename);
        if (!document)
            loadXopp(filename);
    }
}

PdfMaster::~PdfMaster()
{
    qDeleteAll(paths);
    paths.clear();
    delete document;
}

void PdfMaster::loadDocument(const QString &filename)
{
    if (document)
    {
        if (filename != document->getPath())
            preferences()->showErrorMessage(
                        tr("Error while loading file"),
                        tr("Tried to load a pdf file, but a different file is already loaded!"));
        else if (document->loadDocument())
            document->loadLabels();
        return;
    }

    // Load the document
    switch (preferences()->pdf_engine)
    {
#ifdef USE_POPPLER
    case PopplerEngine:
        document = new PopplerDocument(filename);
        break;
#endif
#ifdef USE_MUPDF
    case MuPdfEngine:
        document = new MuPdfDocument(filename);
        break;
#endif
#ifdef USE_QTPDF
    case QtPDFEngine:
        document = new QtDocument(filename);
        break;
#endif
    }

    if (document == NULL || !document->isValid())
        preferences()->showErrorMessage(
                    tr("Error while loading file"),
                    tr("Loading PDF document failed!"));
    else
        document->loadLabels();
}

bool PdfMaster::loadDocument()
{
    if (document && document->loadDocument())
    {
        document->loadLabels();
        return true;
    }
    return false;
}

void PdfMaster::receiveAction(const Action action)
{
    switch (action)
    {
    case UndoDrawing:
    case UndoDrawingLeft:
    case UndoDrawingRight:
    {
        const int page = preferences()->page | (action ^ UndoDrawing);
        PathContainer* const path = paths.value(page, NULL);
        if (path)
        {
            debug_msg(DebugDrawing, "undo:" << path);
            auto scene_it = scenes.cbegin();
            while ( scene_it != scenes.cend() && ( (*scene_it)->getPage() | (*scene_it)->pagePart() ) != page)
                ++scene_it;
            SlideScene * const scene = scene_it == scenes.cend() ? NULL : *scene_it;
            if (path->undo(scene))
            {
                _flags |= UnsavedDrawings;
                if (scene)
                    scene->updateSelectionRect();
            }
        }
        break;
    }
    case RedoDrawing:
    case RedoDrawingLeft:
    case RedoDrawingRight:
    {
        const int page = preferences()->page | (action ^ RedoDrawing);
        PathContainer* const path = paths.value(page, NULL);
        if (path)
        {
            debug_msg(DebugDrawing, "redo:" << path);
            auto scene_it = scenes.cbegin();
            while ( scene_it != scenes.cend() && ( (*scene_it)->getPage() | (*scene_it)->pagePart() ) != page)
                ++scene_it;
            SlideScene * const scene = scene_it == scenes.cend() ? NULL : *scene_it;
            if (path->redo(scene))
            {
                _flags |= UnsavedDrawings;
                if (scene)
                    scene->updateSelectionRect();
            }
        }
        break;
    }
    case ClearDrawing:
    case ClearDrawingLeft:
    case ClearDrawingRight:
    {
        PathContainer* const path = paths.value(preferences()->page | (action ^ ClearDrawing), NULL);
        if (path)
        {
            debug_msg(DebugDrawing, "clear:" << path);
            if (path->clearPaths())
                _flags |= UnsavedDrawings;
        }
        break;
    }
    default:
        break;
    }
}

void PdfMaster::replacePath(int page, QGraphicsItem *olditem, QGraphicsItem *newitem)
{
    if (!olditem && !newitem)
        return;
    if (preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
    assertPageExists(page);
    paths[page]->replaceItem(olditem, newitem);
    _flags |= UnsavedDrawings;
}

void PdfMaster::addItemsForeground(int page, const QList<QGraphicsItem*> &items)
{
    if (items.empty())
        return;
    if (preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
    assertPageExists(page);
    paths[page]->addItemsForeground(items);
    _flags |= UnsavedDrawings;
}

void PdfMaster::removeItems(int page, const QList<QGraphicsItem*> &items)
{
    if (items.empty())
        return;
    if (preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
    assertPageExists(page);
    paths[page]->removeItems(items);
    _flags |= UnsavedDrawings;
}

void PdfMaster::addHistoryStep(int page,
        std::map<QGraphicsItem*, QTransform> *transforms,
        std::map<QGraphicsItem*, drawHistory::DrawToolDifference> *tools,
        std::map<QGraphicsItem*, drawHistory::TextPropertiesDifference> *texts)
{
    if (preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
    assertPageExists(page);
    if (paths[page]->addChanges(transforms, tools, texts))
        _flags |= UnsavedDrawings;
}

void PdfMaster::bringToForeground(int page, const QList<QGraphicsItem*> &to_foreground)
{
    if (to_foreground.empty())
        return;
    if (preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
    assertPageExists(page);
    if (paths[page]->bringToForeground(to_foreground))
        _flags |= UnsavedDrawings;
}

void PdfMaster::bringToBackground(int page, const QList<QGraphicsItem*> &to_background)
{
    if (to_background.empty())
        return;
    if (preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
    assertPageExists(page);
    if (paths[page]->bringToBackground(to_background))
        _flags |= UnsavedDrawings;
}

void PdfMaster::distributeNavigationEvents(const int page) const
{
    // Map (shifted) page numbers with page parts to slide scenes.
    // Like this it can be detected if multiple scenes want to show the same
    // page. In this case the SlideViews showing the same page will all be
    // connected to the same scene.
    QMap<int, SlideScene*> scenemap;
    if (preferences()->overlay_mode == PerLabel)
    {
        for (const auto scene : qAsConst(scenes))
        {
            int shift = scene->getShift();
            const int scenepage = overlaysShifted(page, shift) | scene->pagePart();
            const int indexpage = overlaysShifted(page, (shift & ~AnyOverlay) | FirstOverlay) | scene->pagePart();
            if (scenemap.contains(indexpage))
                scene->navigationEvent(scenepage & ~NotFullPage, scenemap[indexpage]);
            else
            {
                scenemap[indexpage] = scene;
                scene->navigationEvent(scenepage & ~NotFullPage);
            }
        }
    }
    else
    {
        // Redistribute views to scenes.
        for (const auto scene : qAsConst(scenes))
        {
            const int scenepage = overlaysShifted(page, scene->getShift()) | scene->pagePart();
            if (scenemap.contains(scenepage))
                scene->navigationEvent(scenepage & ~NotFullPage, scenemap[scenepage]);
            else
                scenemap[scenepage] = scene;
        }
        // Navigation events to active scenes.
        for (auto it = scenemap.cbegin(); it != scenemap.cend(); ++it)
            (*it)->navigationEvent(it.key() & ~NotFullPage);
    }
    for (const auto scene : qAsConst(scenes))
        scene->createSliders();
}

void PdfMaster::saveXopp(const QString &filename)
{
    drawings_path = filename;
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    QXmlStreamWriter writer(&buffer);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(0);
    writer.writeStartDocument();

    writer.writeStartElement("xournal");
    writer.writeAttribute("creator", "beamerpresenter " APP_VERSION);
    writer.writeTextElement("title", "BeamerPresenter document, compatible with Xournal++ - see https://github.com/stiglers-eponym/BeamerPresenter");

    {
        // Write preview picture.
        writer.writeStartElement("preview");
        const QSizeF &pageSize = getPageSize(0);
        const qreal resolution = 128 / std::max(pageSize.width(), pageSize.height());
        const QPixmap pixmap = exportImage(0, resolution);
        QByteArray data;
        QBuffer buffer(&data);
        pixmap.save(&buffer, "PNG");
        buffer.close();
        writer.writeCharacters(data.toBase64());
        writer.writeEndElement();
    }

    // Some attributes specific for beamerpresenter (Xournal++ will ignore that)
    writer.writeStartElement("beamerpresenter");
    if (preferences()->msecs_total)
        writer.writeAttribute("duration", QTime::fromMSecsSinceStartOfDay(preferences()->msecs_total).toString("h:mm:ss"));
    emit writeNotes(writer);
    writer.writeEndElement(); // "beamerpresenter" element

    const PdfDocument *doc = preferences()->document;
    PathContainer *container;
    for (int i=0; i < preferences()->number_of_pages; i++)
    {
        QSizeF size = doc->pageSize(i);
        QRectF drawing_rect;
        for (const auto page_part : {FullPage, LeftHalf, RightHalf})
        {
            if (preferences()->overlay_mode == PerLabel)
                container = paths.value(doc->overlaysShifted(i | page_part, FirstOverlay), NULL);
            else
                container = paths.value(i | page_part, NULL);
            if (container)
                drawing_rect = drawing_rect.united(container->boundingBox());
        }
        size.setWidth(std::max(size.width(), drawing_rect.right()));
        size.setHeight(std::max(size.height(), drawing_rect.bottom()));
        writer.writeStartElement("page");
        writer.writeAttribute("width", QString::number(size.width()));
        writer.writeAttribute("height", QString::number(size.height()));
        writer.writeEmptyElement("background");
        writer.writeAttribute("type", "pdf");
        writer.writeAttribute("pageno", QString::number(i+1));
        if (i == 0)
        {
            writer.writeAttribute("domain", "absolute");
            writer.writeAttribute("filename", doc->getPath());
        }
        if (target_times.contains(i))
            writer.writeAttribute("endtime", QTime::fromMSecsSinceStartOfDay(target_times[i]).toString("h:mm:ss"));

        writer.writeStartElement("layer");
        for (const auto page_part : {FullPage, LeftHalf, RightHalf})
        {
            if (preferences()->overlay_mode == PerLabel)
                container = paths.value(doc->overlaysShifted(i | page_part, FirstOverlay), NULL);
            else
                container = paths.value(i | page_part, NULL);
            if (container)
                container->writeXml(writer);
        }
        writer.writeEndElement(); // "layer" element
        writer.writeEndElement(); // "page" element
    }

    writer.writeEndElement(); // "xournal" element
    writer.writeEndDocument();
    bool saving_failed = writer.hasError();
    if (saving_failed)
        preferences()->showErrorMessage(
                    tr("Error while saving file"),
                    tr("Writing document resulted in error! Resulting document is probably corrupt."));

    gzFile file = gzopen(filename.toUtf8(), "wb");
    if (file)
    {
        gzwrite(file, buffer.data().data(), buffer.data().length());
        gzclose_w(file);
    }
    else
    {
        QFile file(filename);
        if (file.open(QFile::WriteOnly))
        {
            qWarning() << "Compressing document failed. Saving without compression.";
            file.write(buffer.data());
            file.close();
        }
        else
        {
            saving_failed = true;
            preferences()->showErrorMessage(
                        tr("Error while saving file"),
                        tr("Saving document failed for file path: ") + filename);
        }
    }
    if (!saving_failed)
        _flags &= ~(UnsavedDrawings|UnsavedTimes|UnsavedNotes);
}

void PdfMaster::loadXopp(const QString &filename)
{
    QBuffer *buffer = loadZipToBuffer(filename);
    QXmlStreamReader reader(buffer);
    while (!reader.atEnd() && (reader.readNext() != QXmlStreamReader::StartElement || reader.name().toUtf8() != "xournal")) {}

    if (!reader.atEnd())
    {
        drawings_path = filename;
        bool nontrivial_page_part = false;
        for (const auto scene : qAsConst(scenes))
        {
            if (scene->pagePart() != FullPage)
            {
                nontrivial_page_part = true;
                break;
            }
        }
        while (reader.readNextStartElement())
        {
            debug_msg(DebugDrawing, "Reading element" << reader.name());
            if (reader.name().toUtf8() == "page")
                readPageFromStream(reader, nontrivial_page_part);
            else if (reader.name().toUtf8() == "beamerpresenter")
                readPropertiesFromStream(reader);
            else if (!reader.isEndElement())
                reader.skipCurrentElement();
        }
        if (reader.hasError())
            preferences()->showErrorMessage(
                        tr("Error while loading file"),
                        tr("Failed to read xopp document: ") + reader.errorString());
        else if (!document)
        {
            qWarning() << "Failed to determine PDF document from xournal file." << filename;
            const QString filename = QFileDialog::getOpenFileName(
                                     NULL,
                                     tr("PDF file could not be opened, select the correct PDF file."),
                                     "",
                                     tr("Documents (*.pdf);;All files (*)")
                                 );
            if (filename.isNull())
                preferences()->showErrorMessage(
                        tr("Error while loading file"),
                        tr("No PDF file found"));
            else
            {
                const QFileInfo fileinfo(filename);
                if (fileinfo.exists())
                    loadDocument(fileinfo.absoluteFilePath());
                else
                    preferences()->showErrorMessage(
                            tr("Error while loading file"),
                            tr("File does not exist: ") + fileinfo.absoluteFilePath());
            }
        }
    }
    else
        preferences()->showErrorMessage(
                    tr("Error while loading file"),
                    tr("Failed to read xopp document: ") + reader.errorString());
    reader.clear();
    buffer->close();
    delete buffer;
}

void PdfMaster::reloadXoppProperties()
{
    debug_msg(DebugWidgets, "reloading Xopp properties" << drawings_path);
    if (drawings_path.isEmpty())
        return;
    QBuffer *buffer = loadZipToBuffer(drawings_path);
    if (!buffer)
        return;
    QXmlStreamReader reader(buffer);
    while (!reader.atEnd() && (reader.readNext() != QXmlStreamReader::StartElement || reader.name().toUtf8() != "xournal")) {}

    if (!reader.atEnd())
    {
        while (reader.readNextStartElement())
        {
            if (reader.name().toUtf8() == "beamerpresenter")
                readPropertiesFromStream(reader);
            else if (!reader.isEndElement())
                reader.skipCurrentElement();
        }
        if (reader.hasError())
            preferences()->showErrorMessage(
                        tr("Error while loading file"),
                        tr("Failed to read xopp document: ") + reader.errorString());
    }
    else
        preferences()->showErrorMessage(
                    tr("Error while loading file"),
                    tr("Failed to read xopp document: ") + reader.errorString());
    reader.clear();
    buffer->close();
    delete buffer;
}

QBuffer *PdfMaster::loadZipToBuffer(const QString &filename)
{
    // This is probably not how it should be done.
    QBuffer *buffer(new QBuffer());
    buffer->open(QBuffer::ReadWrite);
    gzFile file = gzopen(filename.toUtf8(), "rb");
    if (!file)
    {
        qWarning() << "Loading drawings failed: file" << filename << "could not be opened";
        preferences()->showErrorMessage(
                    tr("Error while loading file"),
                    tr("Loading drawings failed: file ") + filename + tr(" could not be opened"));
        return NULL;
    }
    gzbuffer(file, 32768);
    char chunk[512];
    int status = 0;
    do {
        status = gzread(file, chunk, 512);
        buffer->write(chunk, status);
    } while (status == 512);
    gzclose_r(file);
    buffer->seek(0);
    return buffer;
}

void PdfMaster::readPageFromStream(QXmlStreamReader &reader, bool &nontrivial_page_part)
{
    debug_msg(DebugDrawing, "read page from stream" << reader.name());
    int page = -1;
    static const QRegularExpression regexpr_2nondigits("[^0-9]{2,2}$");
    while (reader.readNextStartElement())
    {
        debug_msg(DebugDrawing, "Searching background" << reader.name());
        if (reader.name().toUtf8() == "background")
        {
            QString string = reader.attributes().value("pageno").toString();
            // For some reason Xournal++ adds "ll" as a sufix to the page number.
            if (string.contains(regexpr_2nondigits))
                string.chop(2);
            bool ok;
            page = string.toInt(&ok) - 1;
            if (!ok)
                return;
            string = reader.attributes().value("endtime").toString();
            if (!string.isEmpty())
            {
                const QTime time = QTime::fromString(string, "h:mm:ss");
                if (time.isValid())
                    target_times[page] = time.msecsSinceStartOfDay();
            }
#if (QT_VERSION_MAJOR >= 6)
            const QStringView filename = reader.attributes().value("filename");
#else
            const QStringRef filename = reader.attributes().value("filename");
#endif
            if (!filename.isEmpty())
            {
                if (!document)
                {
                    const QFileInfo fileinfo(filename.toString());
                    if (fileinfo.exists())
                    {
                        loadDocument(fileinfo.absoluteFilePath());
                        if (document && preferences()->page_part_threshold > 0.01)
                        {
                            const QSizeF size = document->pageSize(0);
                            if (size.width() > preferences()->page_part_threshold*size.height())
                                nontrivial_page_part = true;
                        }
                    }
                    else
                        qWarning() << "Document does not exist:" << filename;
                }
                else if (filename != document->getPath())
                    qWarning() << "reading document for possibly wrong PDF file:" << filename << document->getPath();
            }
            if (!reader.isEndElement())
                reader.skipCurrentElement();
            break;
        }
        if (!reader.isEndElement())
            reader.skipCurrentElement();
    }
    if (page < 0)
        return;
    while (reader.readNextStartElement())
    {
        debug_msg(DebugDrawing, "Searching layer" << reader.name());
        if (reader.name().toUtf8() == "layer")
        {
            if (nontrivial_page_part)
            {
                const qreal page_half = document ? document->pageSize(page).width()/2 : 226.772;
                PathContainer *left = paths.value(page | LeftHalf, NULL),
                            *right = paths.value(page | RightHalf, NULL);
                if (!left)
                {
                    left = new PathContainer(this);
                    paths[page | LeftHalf] = left;
                }
                if (!right)
                {
                    right = new PathContainer(this);
                    paths[page | RightHalf] = right;
                }
                PathContainer::loadDrawings(reader, left, right, page_half);
            }
            else
            {
                PathContainer *container = paths.value(page);
                if (!container)
                {
                    container = new PathContainer(this);
                    paths[page] = container;
                }
                container->loadDrawings(reader);
            }
            if (!reader.isEndElement())
                reader.skipCurrentElement();
            break;
        }
        if (!reader.isEndElement())
            reader.skipCurrentElement();
    }
    reader.skipCurrentElement();
}

void PdfMaster::readPropertiesFromStream(QXmlStreamReader &reader)
{
    const QTime time = QTime::fromString(reader.attributes().value("duration").toString(), "h:mm:ss");
    if (time.isValid())
    {
        emit setTotalTime(time);
        // If may happen that this is called before a timer widget is created.
        // Then setTotalTime(time) will do nothing and preferences()->msecs_total
        // must be set directly.
        writable_preferences()->msecs_total = time.msecsSinceStartOfDay();
    }
    while (reader.readNextStartElement())
    {
        if (reader.name().toUtf8() == "speakernotes")
            emit readNotes(reader);
        if (!reader.isEndElement())
            reader.skipCurrentElement();
    }
}

PathContainer *PdfMaster::pathContainerCreate(int page)
{
    switch (preferences()->overlay_mode)
    {
    case PerPage:
        return paths.value(page, NULL);
    case PerLabel:
        page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) | (page & NotFullPage);
        return paths.value(page, NULL);
    case Cumulative:
        PathContainer *container = paths.value(page);
        if (container && !container->empty() && !container->isPlainCopy())
            return container;
        const int page_part = page & NotFullPage;
        int basic_page = page ^ page_part;
        const int start_overlay = document->overlaysShifted(basic_page, FirstOverlay);
        PathContainer *copy_container;
        while (basic_page-- > start_overlay)
        {
            copy_container = paths.value(basic_page | page_part, NULL);
            if (copy_container)
            {
                delete container;
                container = copy_container->copy();
                paths[page] = container;
                return container;
            }
        }
    }
    return nullptr;
}

void PdfMaster::clearAllDrawings()
{
    for (const auto container : qAsConst(paths))
    {
        if (container)
        {
            container->clearPaths();
            container->clearHistory();
        }
    }
}

void PdfMaster::getTimeForPage(const int page, quint32 &time) const noexcept
{
    if (target_times.empty() || page > target_times.lastKey())
        time = UINT32_MAX;
    else
        time = *target_times.lowerBound(page);
}

bool PdfMaster::hasDrawings() const noexcept
{
    for (auto path : qAsConst(paths))
        if (!path->isCleared())
            return true;
    return false;
}

void PdfMaster::search(const QString &text, const int &page, const bool forward)
{
    if (!document || page < 0)
        return;
    if (text == "")
    {
        search_results.second.clear();
        emit updateSearch();
        return;
    }
    search_results = document->searchAll(text, page, forward);
    if (search_results.first == preferences()->page)
        emit updateSearch();
    else if (search_results.first >= 0)
        emit navigationSignal(search_results.first);
}

QPixmap PdfMaster::exportImage(const int page, const qreal resolution) const noexcept
{
    if (!document || resolution <= 0 || page < 0 || page >= document->numberOfPages())
        return QPixmap();
    debug_msg(DebugDrawing, "Export image" << page << resolution);
    const auto *renderer = document->createRenderer(static_cast<PagePart>(page & PagePart::NotFullPage));
    if (!renderer || !renderer->isValid())
        return QPixmap();
    QPixmap pixmap = renderer->renderPixmap(page & ~PagePart::NotFullPage, resolution);
    const auto *container = paths.value(page, nullptr);
    if (container)
    {
        debug_msg(DebugDrawing, "Exporting items");
        QStyleOptionGraphicsItem style;
        QPainter painter;
        painter.begin(&pixmap);
        for (auto item : *container)
        {
            painter.resetTransform();
            painter.scale(resolution, resolution);
            painter.setTransform(item->sceneTransform(), true);
            item->paint(&painter, &style);
        }
        painter.end();
    }
    return pixmap;
}
