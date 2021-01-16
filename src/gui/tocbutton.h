#ifndef TOCBUTTON_H
#define TOCBUTTON_H

#include <QPushButton>
#include <QCheckBox>

/// Node in tree of page widgets.
class TOCbutton : public QPushButton
{
    Q_OBJECT

    friend class TOCwidget;
    QCheckBox *expand_button;
    const int page;
    TOCbutton *tree_next = NULL;
    TOCbutton *tree_child = NULL;

public:
    /// does takes ownership of expand_button
    TOCbutton(const QString &title, const int _page, QCheckBox *expand_button, QWidget *parent = nullptr);
    ~TOCbutton() {delete expand_button;}

    void expand();
    void expand_full();
    void collapse();
    void collapse_hide();

signals:
    void sendNavigationEvent(const int page);
};

#endif // TOCBUTTON_H
