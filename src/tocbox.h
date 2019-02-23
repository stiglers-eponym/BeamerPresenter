#ifndef TOCBOX_H
#define TOCBOX_H

#include <QWidget>
#include <QMenu>
#include <QDebug>
#include <QVBoxLayout>
#include <QDomDocument>
#include "tocbutton.h"
#include "tocaction.h"

class TocBox : public QWidget
{
    Q_OBJECT

private:
    QStringList const indentStrings = {"  ", "    ➤ ", "       - ", "          + "};
    QDomDocument const * toc = nullptr;
    int unfoldLevel = 2;
    QVBoxLayout* layout;
    QList<TocButton*> buttons;
    QList<QMenu*> menus;
    void recursiveTocCreator(QDomNode const& node, int const level);
    bool need_update = true;

public:
    TocBox(QWidget* parent = nullptr);
    ~TocBox();
    void createToc(QDomDocument const* toc);
    void setUnfoldLevel(int const level);
    bool needUpdate() {return need_update;}
    bool hasToc() {return toc!=nullptr;}

signals:
    void sendDest(QString const & dest);
};

#endif // TOCBOX_H
