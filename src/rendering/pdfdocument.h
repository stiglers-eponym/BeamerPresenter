#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QDateTime>
#include "src/rendering/pngpixmap.h"
/*
#ifdef INCLUDE_MUPDF
#include <mupdf/fitz/color.h>
#include <mupdf/fitz/transition.h>
#endif
*/

struct PdfLink {
    /// Positive values of type are interpreted as page numbers.
    /// Negative values are interpreted as LinkType.
    int type;
    QString target;
};

struct SlideTransition {
    enum Type
    {
        Replace = 0,
/*
#ifdef INCLUDE_MUPDF
        // Allow static_cast from fz_transition::type.
        // It would also work with static cast in poppler, but that seems
        // quite unsafe (although they probably won't change the order).
        Split = FZ_TRANSITION_SPLIT,
        Blinds = FZ_TRANSITION_BLINDS,
        Box = FZ_TRANSITION_BOX,
        Wipe = FZ_TRANSITION_WIPE,
        Dissolve = FZ_TRANSITION_DISSOLVE,
        Glitter = FZ_TRANSITION_GLITTER,
        Fly = FZ_TRANSITION_FLY,
        Push = FZ_TRANSITION_PUSH,
        Cover = FZ_TRANSITION_COVER,
        Uncover = FZ_TRANSITION_UNCOVER,
        Fade = FZ_TRANSITION_FADE,
#else
*/
        Split,
        Blinds,
        Box,
        Wipe,
        Dissolve,
        Glitter,
        Fly,
        Push,
        Cover,
        Uncover,
        Fade,
        FlyRectangle,
    };
    enum Properties
    {
        Outwards = 1,
        Vertical = 2,
    };
    qint8 type = Replace;
    /// first bit: inward (0) or outward (1) direction.
    /// second bit: horizontal (0) or vertical (1) direction.
    qint8 properties = 0;
    /// Angle in degrees of the direction of the direction.
    qint16 angle = 0;
    /// Transition duration in s.
    float duration = 0.;
    /// Only relevant for Fly and FlyRectangle, in [0,1].
    /// Starting point for "flying" relative to the usual "fly" path.
    float scale = 1.;
};

/// Abstract class for handling PDF documents.
class PdfDocument
{

protected:
    QDateTime lastModified;
    QString path;

public:
    enum PdfBackend {
#ifdef INCLUDE_POPPLER
        PopplerBackend = 0,
#endif
#ifdef INCLUDE_MUPDF
        MuPdfBackend = 1,
#endif
    };
    explicit PdfDocument(const QString &filename) : path(filename) {}
    virtual ~PdfDocument() {}
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    virtual const QPixmap getPixmap(const int page, const qreal resolution) const = 0;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    virtual const PngPixmap* getPng(const int page, const qreal resolution) const = 0;
    virtual bool loadDocument() = 0;
    /// Size of page in points (72*inch).
    virtual const QSizeF pageSize(const int page) const = 0;
    virtual int numberOfPages() const = 0;
    virtual const QString label(const int page) const = 0;
    virtual int overlaysShifted(const int start, const int shift_overlay) const = 0;
    virtual bool isValid() const = 0;
    /// Link at given position (in point = inch/72)
    virtual const PdfLink linkAt(const int page, const QPointF &position) const = 0;
    const QString & getPath() const {return path;}
    virtual const SlideTransition transition(const int page) const = 0;
};

#endif // PDFDOCUMENT_H
