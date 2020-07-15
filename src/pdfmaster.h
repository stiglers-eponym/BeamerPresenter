#ifndef PDFMASTER_H
#define PDFMASTER_H

#include <set>
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
    PdfDocument* document = nullptr;

    /// Lookup table for page labels.
    std::set<int> overlay_slide_indices;

    /// Map of pixmap cache maps for storing cached slides.
    /// keys are int(1000*resolution).
    QMap<int, PixCache*> pixcaches;

    /// Map page numbers to containers of paths.
    /// Paths can be drawn per slide label by creating references to the main
    /// path list from other slide numbers.
    QMap<int, PathContainer*> paths;

    // TODO: multimedia

    /// Generate overlay_slide_indices.
    void populateOverlaySlidesSet();


public:
    /// Create a new PdfMaster from a given file name.
    explicit PdfMaster(const QString &filename);
    ~PdfMaster();

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument();

    /// Get path to PDF file.
    const QString &getFilename() const {return document->getPath();}

    /// Get size of page in points (floating point precision).
    const QSizeF getPageSize(const int page_number) const;

    const PdfDocument * getDocument() const {return document;}

    int numberOfPages() const {return document->numberOfPages();}

    int overlaysShifted(const int start, const int shift_overlay) const;

public slots:
    /// Paths have changed on SlideView sender. Update paths and send out a
    /// signal to all SlideScenes.
    void updatePaths(const SlideView *sender);
    void receiveAction(const Action action);

signals:
    /// Notify all associated SlidesScenes that paths have changed.
    void pathsUpdated();
    void update();
};

#endif // PDFMASTER_H
