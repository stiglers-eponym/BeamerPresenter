#ifndef THUMBNAILBUTTON_H
#define THUMBNAILBUTTON_H

#include <QLabel>
#include <QMouseEvent>

/**
 * @brief ThumbnailButton: pushable button showing page preview
 */
class ThumbnailButton : public QLabel
{
    Q_OBJECT

    const int page;

public:
    ThumbnailButton(const int page, QWidget *parent = NULL) : QLabel(parent), page(page) {}

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void sendNavigationSignal(int page);
};

#endif // THUMBNAILBUTTON_H
