#ifndef TOCWIDGET_H
#define TOCWIDGET_H

#include <QDebug>
#include <QWidget>
#include <QBoxLayout>
#include <QPushButton>
#include "src/rendering/pdfdocument.h"
#include "src/preferences.h"

class TOCwidget : public QWidget
{
    Q_OBJECT

    TOCwidget *tree_parent = NULL;
    QList<TOCwidget*> tree_children;
    const int page;

public:
    explicit TOCwidget(QWidget *parent = nullptr) : QWidget(parent), page(-1) {}

    explicit TOCwidget(const QList<PdfOutlineEntry> &outline, TOCwidget *parent, const int entry);

    void generateTOC(const PdfDocument *document = nullptr);

    /// Actually this is nonsense, but currently the layout only works with
    /// this option set.
    bool hasHeightForWidth() const override
    {return true;}

protected:
    bool event(QEvent *event) override;

signals:
    void sendNavigationSignal(const int page);

};

#endif // TOCWIDGET_H
