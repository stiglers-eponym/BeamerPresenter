#ifndef PDFMASTER_H
#define PDFMASTER_H

#include <QFileInfo>
#include <QInputDialog>
#include <zlib.h>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMimeDatabase>
#include "src/slidescene.h"
#include "src/slideview.h"
#include "src/preferences.h"
#include "src/drawing/pathcontainer.h"
#include "src/rendering/pixcache.h"
#include "src/rendering/pdfdocument.h"

/// Full document including PDF and paths / annotations added by user.
/// This should also manage drawings and multimedia content of the PDF.
class PdfMaster : public QObject
{
    Q_OBJECT

private:
    /// Poppler document representing the PDF
    PdfDocument *document {NULL};

    /// Graphics scenes of this application. For each combination of PDF file
    /// and page shift one scene is created.
    /// Master scene is the first scene in the list.
    QList<SlideScene*> scenes;

    /// Path to file in which drawings are saved.
    QString drawings_path;

    /// Map page (part) numbers to containers of paths.
    /// page (part) numbers are given as (page | page_part)
    /// Paths can be drawn per slide label by creating references to the main
    /// path list from other slide numbers.
    QMap<int, PathContainer*> paths;

    // TODO: multimedia, slide transitions
    QMap<int, QList<MediaAnnotation>> mediaAnnotations;


public:
    /// Create a new PdfMaster from a given file name.
    explicit PdfMaster(const QString &filename);

    /// Destructor. Deletes paths and document.
    ~PdfMaster();

    /// Load PDF file.
    void loadDocument(const QString &filename);

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument();

    /// Get path to PDF file.
    const QString &getFilename() const
    {return document->getPath();}

    /// Get the list of SlideScenes connected to this PDF.
    QList<SlideScene*> &getScenes()
    {return scenes;}

    /// Get size of page in points (floating point precision).
    const QSizeF getPageSize(const int page_number) const;

    /// Get PdfDocument.
    PdfDocument *getDocument() const
    {return document;}

    /// Return true if document contains pages of different size.
    bool flexiblePageSizes() const noexcept
    {return document->flexiblePageSizes();}

    /// This function should be restructured!
    void resolveLink(const int page, const QPointF &position, const QPointF &startpos = {}) const;

    /// Clear history of given page.
    void clearHistory(const int page, const int remaining_entries) const;

    /// Slide transition when reaching the given page number.
    const SlideTransition transition(const int page) const
    {return document->transition(page);}

    /// Number of pages in the document.
    int numberOfPages() const
    {return document->numberOfPages();}

    /// Get page number of start shifted by shift_overlay.
    /// Here in shift_overlay the bits of ShiftOverlay::FirstOverlay and
    /// ShiftOverlay::LastOverlay control the interpretation of the shift.
    /// Shifting with overlays means that every page with a different page
    /// label starts a new "real" side.
    int overlaysShifted(const int start, const int shift_overlay) const
    {return document->overlaysShifted(start, shift_overlay);}

    /// Save drawings to gzip-compressed xml file.
    /// This does not check whether filename is valid and accessible!
    void saveXopp(const QString &filename);

    /// Load drawings from gzip-compressed xml file.
    /// This does not check whether filename is valid and accessible!
    /// If no document is loaded, this will call loadDocument(path)
    /// with the pdf file path from the xopp file.
    void loadXopp(const QString &filename);

    /// Get path container at given page. If overlay_mode==Cumulative, this may
    /// create and return a copy of a previous path container.
    PathContainer *pathContainer(int page);

    /// Get file path at which drawings are saved.
    const QString &drawingsPath() const noexcept
    {return drawings_path;}

    /// Clear all drawings including history.
    void clearAllDrawings();

public slots:
    /// Handle the given action.
    void receiveAction(const Action action);

    /// Add a new path (or QGraphicsItem) to paths[page].
    /// Page (part) number is given as (page | page_part).
    void receiveNewPath(int page, QGraphicsItem *item);

    /// Send navigation events to all SlideScenes reading from this document.
    /// This is done centrally via PdfMaster because it may be necessary
    /// to reconnect SlideViews and SlideScenes if multiple scenes would
    /// show the same page.
    void distributeNavigationEvents(const int page) const;

    /// Get container of paths on given page.
    /// page (part) number is given as (page | page_part).
    void requestPathContainer(PathContainer **container, int page);

signals:
    /// Notify all associated SlidesScenes that paths have changed.
    void pathsUpdated() const;
    /// Send a navigation signal (to master).
    void navigationSignal(const int page) const;
    /// Notify that views need to be updated.
    void update() const;
};

#endif // PDFMASTER_H
