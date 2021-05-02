#ifndef TOCBUTTON_H
#define TOCBUTTON_H

#include <QPushButton>
#include <QCheckBox>

/**
 * @brief Node in tree of outline entry buttons
 *
 * A button representing an outline entry. This is part of a tree representing
 * the full outline of the document. The tree structure is encoded by a
 * pointer to the first child (tree_child) and a pointer to the next node on
 * the same level (tree_next). E.g. tree_child->tree_next is the second child
 * (if it exists).
 *
 * Additionally to the main button this owns expand_button, which controls
 * whether the subtree starting at this node should be shown (expanded) or
 * hidden (collapsed).
 */
class TOCbutton : public QPushButton
{
    Q_OBJECT

    friend class TOCwidget;

    /// Button controlling whether the children of this node should be shown.
    QCheckBox *expand_button;

    /// Target page in document of this outline entry.
    const int page;

    /// Next element on same level in tree structure.
    TOCbutton *tree_next = NULL;

    /// First child element in tree structure.
    TOCbutton *tree_child = NULL;

    /// Toggle show / hide children.
    void toggleVisibility();

public:
    /// Constructor: Only directly uses the given values to initialize
    /// properties of this. Takes ownership of expand_button.
    TOCbutton(const QString &title, const int _page, QCheckBox *expand_button, QWidget *parent = NULL);

    /// Destructor: recursively delete the associated subtree.
    ~TOCbutton() {delete expand_button; delete tree_child; delete tree_next;}

    /// Show all direct child nodes.
    void expand();

    /// Recursively show the full subtree (all child nodes).
    void expand_full();

    /// Hide all child nodes.
    void collapse();

    /// Hide this and all child nodes (required for recursive implementation
    /// of collapse());
    void collapse_hide();

    TOCbutton *next() const
    {return tree_next;}

    TOCbutton *child() const
    {return tree_child;}

signals:
    void sendNavigationEvent(const int page);
};

#endif // TOCBUTTON_H
