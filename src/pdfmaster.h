// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PDFMASTER_H
#define PDFMASTER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include "src/config.h"
#include "src/enumerates.h"

class SlideScene;
class PathContainer;
class QGraphicsItem;
class QBuffer;
class PdfDocument;
class QXmlStreamReader;
class QXmlStreamWriter;
struct SlideTransition;

namespace drawHistory {
    struct Step;
}

/**
 * Full document including PDF and paths / annotations added by user.
 * This should also manage drawings and multimedia content of the PDF.
 */
class PdfMaster : public QObject
{
    Q_OBJECT

public:
    /// Flags for different kinds of unsaved changes.
    enum Flags
    {
        UnsavedDrawings = 1 << 0,
        UnsavedTimes = 1 << 1,
        UnsavedNotes = 1 << 2,
    };

private:
    /// Document representing the PDF
    PdfDocument *document {nullptr};

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

    /// Time at which a slide should be finished.
    QMap<int, quint32> target_times;

    /// Flags for unsaved changes.
    unsigned char _flags = 0;

public:
    /// Create empty, uninitialized PdfMaster.
    /// Call this function, then connect this to events, then
    /// call PdfMaster::initialize(filename).
    explicit PdfMaster() {}

    /// Create a new PdfMaster from a given file name.
    explicit PdfMaster(const QString &filename)
    {initialize(filename);}

    /// Destructor. Deletes paths and document.
    ~PdfMaster();

    /// Must be called after constructor before doing anything with this.
    /// Returns true if it was initialized successfully.
    void initialize(const QString &filename);

    /// get function for _flags
    unsigned char &flags() noexcept
    {return _flags;}

    /// Load PDF file.
    void loadDocument(const QString &filename);

    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument();

    /// Get path to PDF file.
    const QString &getFilename() const;

    /// Get the list of SlideScenes connected to this PDF.
    QList<SlideScene*> &getScenes()
    {return scenes;}

    /// Get size of page in points (floating point precision).
    const QSizeF getPageSize(const int page_number) const;

    /// Get PdfDocument.
    PdfDocument *getDocument() const
    {return document;}

    /// Return true if document contains pages of different size.
    bool flexiblePageSizes() const noexcept;

    /// Clear history of given page.
    void clearHistory(const int page, const int remaining_entries) const;

    /// Slide transition when reaching the given page number.
    const SlideTransition transition(const int page) const;

    /// Number of pages in the document.
    int numberOfPages() const;

    /// Get page number of start shifted by shift_overlay.
    /// Here in shift_overlay the bits of ShiftOverlay::FirstOverlay and
    /// ShiftOverlay::LastOverlay control the interpretation of the shift.
    /// Shifting with overlays means that every page with a different page
    /// label starts a new "real" side.
    int overlaysShifted(const int start, const int shift_overlay) const;

    /// Save drawings to gzip-compressed xml file.
    /// This does not check whether filename is valid and accessible!
    void saveXopp(const QString &filename);

    /// Load drawings from gzip-compressed xml file.
    /// This does not check whether filename is valid and accessible!
    /// If no document is loaded, this will call loadDocument(path)
    /// with the pdf file path from the xopp file.
    void loadXopp(const QString &filename);
    /// Reload only the \<beamerpresenter\> element of Xopp file.
    void reloadXoppProperties();
    /// Unzip file to buffer.
    QBuffer *loadZipToBuffer(const QString &filename);
    /// Helper function for loadXopp: read a \<page\> element
    void readPageFromStream(QXmlStreamReader &reader, bool &nontrivial_page_part);
    /// Helper function for loadXopp: read the \<beamerpresenter\> element
    void readPropertiesFromStream(QXmlStreamReader &reader);

    /// Get path container at given page. If overlay_mode==Cumulative, this may
    /// create and return a copy of a previous path container.
    /// page (part) number is given as (page | page_part).
    PathContainer *pathContainerCreate(int page);

    /// Get path container at given page.
    /// page (part) number is given as (page | page_part).
    PathContainer *pathContainer(int page) const
    {return paths.value(page, NULL);}

    /// Get file path at which drawings are saved.
    const QString &drawingsPath() const noexcept
    {return drawings_path;}

    /// Clear all drawings including history.
    void clearAllDrawings();

    /// Check if page currently contains any drawings (ignoring history).
    bool hasDrawings() const noexcept;

public slots:
    /// Handle the given action.
    void receiveAction(const Action action);

    /// Add a new path (or QGraphicsItem) to paths[page].
    /// Page (part) number is given as (page | page_part).
    /// If item is NULL: create the container if it does not exist yet.
    void receiveNewPath(int page, QGraphicsItem *item)
    {replacePath(page, NULL, item);}

    /// Replace an existing path (or QGraphicsItem) in paths[page] by the gievn new one.
    /// Old or new item can be NULL, then only a new item will be created or an
    /// existing one will be removed, respectively.
    /// Page (part) number is given as (page | page_part).
    /// If both items are NULL, only the container is created (if it doesn't exist yet).
    void replacePath(int page, QGraphicsItem *olditem, QGraphicsItem *newitem);

    /// Add history step with transformations and color changes.
    /// Page (part) number is given as (page | page_part).
    void addHistoryStep(int page, drawHistory::Step *step);

    /// Add new paths.
    void addItems(int page, const QList<QGraphicsItem*> &items);
    /// Remove paths.
    void removeItems(int page, const QList<QGraphicsItem*> &items);

    /// Send navigation events to all SlideScenes reading from this document.
    /// This is done centrally via PdfMaster because it may be necessary
    /// to reconnect SlideViews and SlideScenes if multiple scenes would
    /// show the same page.
    void distributeNavigationEvents(const int page) const;

    /// Get path container at given page. If overlay_mode==Cumulative, this may
    /// create and return a copy of a previous path container.
    /// page (part) number is given as (page | page_part).
    void requestNewPathContainer(PathContainer **container, int page)
    {*container = pathContainerCreate(page);}

    /// Set time for page and write it to target_times.
    void setTimeForPage(const int page, const quint32 time) noexcept;

    /// Get time for given page and write it to time.
    void getTimeForPage(const int page, quint32 &time) const noexcept;

    /// Set UnsavedDrawings flag.
    void newUnsavedDrawings() noexcept
    {_flags |= UnsavedDrawings;}

    /// Bring given items to foreground and add history step.
    void bringToForeground(int page, const QList<QGraphicsItem*> &to_foreground);

signals:
    /// Write notes from notes widgets to stream writer.
    void writeNotes(QXmlStreamWriter &writer) const;
    /// Read notes in notes widgets from stream reader.
    void readNotes(QXmlStreamReader &reader) const;
    /// Set total time of presentation (preferences().total_time).
    void setTotalTime(const QTime time) const;
};

#endif // PDFMASTER_H
