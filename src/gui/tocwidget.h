#ifndef TOCWIDGET_H
#define TOCWIDGET_H

#include <QDebug>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include "src/rendering/pdfdocument.h"
#include "src/preferences.h"
#include "src/gui/tocbutton.h"

/**
 * @brief TOCwidget class: show document outline.
 *
 * The document outline is saved as a tree structure of TOCbuttons.
 * The tree root is first_button.
 */
class TOCwidget : public QWidget
{
    Q_OBJECT

    /// Root of TOCbutton tree representing the outline.
    TOCbutton *first_button = NULL;

public:
    /// Trivial constructor, does not create the outline tree.
    explicit TOCwidget(QWidget *parent = NULL) : QWidget(parent) {}

    /// Destructor: TOCbuttons are deleted recursively.
    ~TOCwidget()
    {delete first_button;}

    /// Generate the TOC from given document or preferences()->document.
    void generateTOC(const PdfDocument *document = NULL);

    /// Actually this is nonsense, but currently the layout only works with
    /// this option set.
    bool hasHeightForWidth() const override
    {return true;}

public slots:
    /// Show event: generate outline if necessary. Expand to current position.
    void showEvent(QShowEvent*) override
    {if (first_button) expandTo(preferences()->page); else generateTOC();}

    /// Focus event: generate outline if necessary.
    void focusInEvent(QFocusEvent*) override
    {generateTOC();}

    /// Expand all sections, subsections, ... which contain the given page.
    void expandTo(const int page);

signals:
    /// Send navigation event to master.
    void sendNavigationSignal(const int page);
};

#endif // TOCWIDGET_H
