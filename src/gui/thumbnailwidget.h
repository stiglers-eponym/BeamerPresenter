#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QScrollArea>
#include <QScroller>
#include <QGridLayout>
#include "src/preferences.h"
#include "src/gui/thumbnailbutton.h"
#include "src/rendering/pdfdocument.h"
#ifdef INCLUDE_POPPLER
#include "src/rendering/popplerrenderer.h"
#endif
#ifdef INCLUDE_MUPDF
#include "src/rendering/mupdfrenderer.h"
#endif
#include "src/rendering/externalrenderer.h"

/**
 * @brief Widget showing thumbnail slides on grid layout in scroll area.
 *
 * TODO:
 *  don't change slide after touch scroll event
 *  cursor (selected frame marked by red margin)
 *  clear when resizing
 *  keyboard navigation
 */
class ThumbnailWidget : public QScrollArea
{
    Q_OBJECT

    AbstractRenderer *renderer = NULL;
    int columns = 4;

public:
    explicit ThumbnailWidget(QWidget *parent = NULL) : QScrollArea(parent) {}

    void setColumns(const int n_columns)
    {columns = n_columns;}

    void generate(const PdfDocument *document = NULL);

    bool hasHeightForWidth() const noexcept override
    {return true;}

public slots:
    /// Show event: generate thumbnails if necessary.
    void showEvent(QShowEvent*) override
    {if (!renderer) generate();}

    /// Focus event: generate thumbnails if necessary.
    void focusInEvent(QFocusEvent*) override
    {if (!renderer) generate();}

signals:
    void sendNavigationSignal(int page);
};

#endif // THUMBNAILWIDGET_H
