#ifndef THUMBNAILBUTTON_H
#define THUMBNAILBUTTON_H

#include <QLabel>

class QMouseEvent;

/**
 * @brief ThumbnailButton: pushable button showing page preview
 */
class ThumbnailButton : public QLabel
{
    Q_OBJECT

    const int page;

public:
    ThumbnailButton(const int page, QWidget *parent = NULL);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    /// add red margin when this gets focus
    void focusInEvent(QFocusEvent*) override;
    /// remove red margin when this looses focus
    void focusOutEvent(QFocusEvent*) override;

signals:
    void sendNavigationSignal(int page);
};

#endif // THUMBNAILBUTTON_H
