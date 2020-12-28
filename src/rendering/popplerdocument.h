#ifndef POPPLERDOCUMENT_H
#define POPPLERDOCUMENT_H

#include <poppler/qt5/poppler-qt5.h>
#include <QFileInfo>
#include <QInputDialog>
#include <set>
#include "src/rendering/pdfdocument.h"
#include "src/enumerates.h"

/*
static const QMap<Poppler::PageTransition::Type, SlideTransition::Type> mapTransitionTypes
{
    {Poppler::PageTransition::Replace, SlideTransition::Replace},
    {Poppler::PageTransition::Split, SlideTransition::Split},
    {Poppler::PageTransition::Blinds, SlideTransition::Blinds},
    {Poppler::PageTransition::Box, SlideTransition::Box},
    {Poppler::PageTransition::Wipe, SlideTransition::Wipe},
    {Poppler::PageTransition::Dissolve, SlideTransition::Dissolve},
    {Poppler::PageTransition::Glitter, SlideTransition::Glitter},
    {Poppler::PageTransition::Fly, SlideTransition::Fly},
    {Poppler::PageTransition::Push, SlideTransition::Push},
    {Poppler::PageTransition::Cover, SlideTransition::Cover},
    {Poppler::PageTransition::Uncover, SlideTransition::Uncover},
    {Poppler::PageTransition::Fade, SlideTransition::Fade},
};
*/

class PopplerDocument : public PdfDocument
{
    const Poppler::Document *doc {nullptr};

    /// Lookup table for page labels.
    std::set<int> overlay_slide_indices;

    /// Generate overlay_slide_indices.
    void populateOverlaySlidesSet();

public:
    PopplerDocument(const QString &filename);
    ~PopplerDocument() override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const QPixmap getPixmap(const int page, const qreal resolution, const PagePart page_part) const;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const PngPixmap* getPng(const int page, const qreal resolution, const PagePart page_part) const;
    /// Load or reload the file. Return true if the file was updated and false
    /// otherwise.
    bool loadDocument() override;
    /// Size of page in points (72*inch).
    const QSizeF pageSize(const int page) const override;
    int numberOfPages() const override;
    bool isValid() const override;
    const QString label(const int page) const override;
    int overlaysShifted(const int start, const int shift_overlay) const override;

    /// Link at given position (in point = inch/72)
    const PdfLink linkAt(const int page, const QPointF &position) const override;
    virtual const VideoAnnotation annotationAt(const int page, const QPointF &position) const override;
    virtual QList<VideoAnnotation>* annotations(const int page) const override;

    const SlideTransition transition(const int page) const override;
};

#endif // POPPLERDOCUMENT_H
