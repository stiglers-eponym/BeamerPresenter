#ifndef PDFMASTER_H
#define PDFMASTER_H

#include <QFileInfo>
#include <QInputDialog>
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
    PdfDocument *document {nullptr};

    /// Graphics scenes of this application. For each combination of PDF file
    /// and page shift one scene is created.
    /// Master scene is the first scene in the list.
    QList<SlideScene*> scenes;

    /// Map page numbers to containers of paths.
    /// Paths can be drawn per slide label by creating references to the main
    /// path list from other slide numbers.
    QMap<int, PathContainer*> paths;

    // TODO: multimedia, slide transitions


public:
    /// Create a new PdfMaster from a given file name.
    explicit PdfMaster(const QString &filename);
    ~PdfMaster();

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument();

    /// Get path to PDF file.
    const QString &getFilename() const
    {return document->getPath();}

    QList<SlideScene*> &getScenes() {return scenes;}

    /// Get size of page in points (floating point precision).
    const QSizeF getPageSize(const int page_number) const;

    const PdfDocument * getDocument() const
    {return document;}

    PathContainer *pathContainer(const int page) const {return paths.value(page, nullptr);}

    void resolveLink(const int page, const QPointF& position) const;

    const SlideTransition transition(const int page) const
    {return document->transition(page);}

    int numberOfPages() const
    {return document->numberOfPages();}

    /// Get page number of start shifted by shift_overlay.
    /// Here in shift_overlay the bits of ShiftOverlay::FirstOverlay and
    /// ShiftOverlay::LastOverlay control the interpretation of the shift.
    /// Shifting with overlays means that every page with a different page
    /// label starts a new "real" side.
    int overlaysShifted(const int start, const int shift_overlay) const
    {return document->overlaysShifted(start, shift_overlay);}

public slots:
    /// Handle the given action.
    void receiveAction(const Action action);
    /// Add a new path (or QGraphicsItem) to paths[page].
    void receiveNewPath(const int page, QGraphicsItem *item);

    /// Send navigation events to all SlideScenes reading from this document.
    /// This is done centrally via PdfMaster because it may be necessary
    /// to reconnect SlideViews and SlideScenes if multiple scenes would
    /// show the same page.
    void distributeNavigationEvents(const int page) const;

    /// Notify history of given page that it needs to store only
    /// preferences().history_length_hidden_slides steps.
    void limitHistoryInvisible(const int page) const;

signals:
    /// Notify all associated SlidesScenes that paths have changed.
    void pathsUpdated() const;
    /// Send a navigation signal (to master).
    void nagivationSignal(const int page) const;
    /// Notify that views need to be updated.
    void update() const;
};

#endif // PDFMASTER_H
