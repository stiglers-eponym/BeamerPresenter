#ifndef TOCBUTTON_H
#define TOCBUTTON_H

#include <QPushButton>

class TocButton : public QPushButton
{
    Q_OBJECT

private:
    QString dest;

public:
    TocButton(QString const& text = "", QString const& dest = "", QWidget * parent = nullptr);

signals:
    void activated(QString const& dest);
};

#endif // TOCBUTTON_H
