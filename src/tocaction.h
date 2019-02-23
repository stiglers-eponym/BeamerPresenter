#ifndef TOCACTION_H
#define TOCACTION_H

#include <QAction>

class TocAction : public QAction
{
    Q_OBJECT

private:
    QString dest;

public:
    TocAction(QString const& text = "", QString const& dest = "", QWidget * parent = nullptr);

signals:
    void activated(QString const& dest);
};

#endif // TOCACTION_H
