#ifndef NOTESWIDGET_H
#define NOTESWIDGET_H

#include <QTextEdit>
#include <QFileDialog>

class QKeyEvent;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief Editable rich-text notes
 *
 * Widgte in flexible GUI of BeamerPresenter which represents rich text
 * which can be edited, loaded, and saved (as HTML).
 *
 * File structure of the saved xml files: xml tree with root element
 * "speakernotes" containing elements "page". Each "page" element has
 * an attribute "label" representing the page label (a string) and
 * contains HTML text.
 *
 * Keyboard shortcuts for the page label are hard-coded:
 * Ctrl+S for saving, Ctrl+Shift+S for saving under a different file
 * name and Ctrl+O for loading a different file.
 */
class NotesWidget : public QTextEdit
{
    Q_OBJECT

    /// File where everything is loaded / saved.
    QString file_path;

    /// Map slide labels to HTML text
    QMap<QString, QString> text_per_slide;

    /// Currently shown page label.
    QString page_label;

    /// Are notes saved per page? Otherwise per page label.
    bool per_page;

public:
    /// Nearly trivial constructor.
    NotesWidget(const bool per_page = false, QWidget *parent = NULL);

    /// Preferred height depends on width (as required for FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Load (only) notes from file. (Not the same as loading drawings and notes!)
    void loadNotes(const QString &filename);

    /// Save (only) notes to file. (Not the same as saving drawings and notes!)
    void saveNotes(const QString &filename);

protected:
    /// Handle keyboard shortcuts (mainly save and load).
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Choose a file using file dialog and load notes from that file.
    void load();
    /// Save notes to given file name. If *filename* is empty, choose from file dialog.
    void save(QString filename);
    /// Switch page, show notes for new page.
    void pageChanged(const int page);
    /// Write notes to stream. This is used for saving as own notes file and
    /// for saving notes as part of a BeamerPresenter file including also
    /// drawings and slide times.
    void writeNotes(QXmlStreamWriter &writer);
    /// Load notes from stream. This is used for loading as own notes file and
    /// for loading notes as part of a BeamerPresenter file including also
    /// drawings and slide times.
    void readNotes(QXmlStreamReader &reader);

signals:
    /// Notify Master of new unsaved changes.
    void newUnsavedChanges() const;
    /// Notify Master that drawings should be saved.
    void saveDrawings(const QString filename);
    /// Notify Master that drawings should be loaded.
    void loadDrawings(const QString filename);
};

#endif // NOTESWIDGET_H
