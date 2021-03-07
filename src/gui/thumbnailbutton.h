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
    ThumbnailButton(const int page, QWidget *parent = NULL) : QLabel(parent), page(page)
    {setFocusPolicy(Qt::FocusPolicy::StrongFocus);}

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;

signals:
    void sendNavigationSignal(int page);
};

#endif // THUMBNAILBUTTON_H
