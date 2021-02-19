#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QScrollArea>
#include <QScroller>
#include <QThread>
#include <QGridLayout>
#include "src/preferences.h"
#include "src/gui/thumbnailbutton.h"
#include "src/gui/thumbnailthread.h"
#include "src/rendering/pdfdocument.h"


/**
 * @brief Widget showing thumbnail slides on grid layout in scroll area.
 *
 * TODO:
 *  check destructors, there might be memory leaks
 *  don't change slide after touch scroll event
 *  cursor (selected frame marked by red margin)
 *  clear when resizing
 *  keyboard navigation
 */
class ThumbnailWidget : public QScrollArea
{
    Q_OBJECT

    ThumbnailThread *render_thread = NULL;
    int columns = 4;
    int ref_width = 0;
    bool skip_overlays = false;

public:
    explicit ThumbnailWidget(QWidget *parent = NULL) : QScrollArea(parent) {}

    ~ThumbnailWidget();

    void setColumns(const int n_columns) noexcept
    {columns = n_columns;}

    void skipOverlays() noexcept
    {skip_overlays = true;}

    void generate(const PdfDocument *document = NULL);

    bool hasHeightForWidth() const noexcept override
    {return true;}

    QSize sizeHint() const noexcept override
    {return {100, 200};}

public slots:
    /// Show event: generate thumbnails if necessary.
    void showEvent(QShowEvent*) override
    {generate();}

    /// Focus event: generate thumbnails if necessary.
    void focusInEvent(QFocusEvent*) override
    {generate();}

    void receiveThumbnail(ThumbnailButton *button, const QPixmap pixmap)
    {if (button) button->setPixmap(pixmap);}

signals:
    void sendNavigationSignal(int page);
    void sendToRenderThread(ThumbnailButton *button, qreal resolution, int page);
    void startRendering();
};

#endif // THUMBNAILWIDGET_H
