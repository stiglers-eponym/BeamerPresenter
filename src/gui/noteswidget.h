#ifndef NOTESWIDGET_H
#define NOTESWIDGET_H

#include <QTextEdit>
#include <QFileDialog>

class QKeyEvent;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief NotesWidget: Editable notes
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
    NotesWidget(const bool per_page = false, QWidget *parent = NULL);

    bool hasHeightForWidth() const noexcept override
    {return true;}

    void loadNotes(const QString &filename);
    void saveNotes(const QString &filename);

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void load();
    void save(QString filename);
    void pageChanged(const int page);
    void writeNotes(QXmlStreamWriter &writer);
    void readNotes(QXmlStreamReader &reader);

signals:
    void newUnsavedChanges() const;
    void saveDrawings(const QString filename);
    void loadDrawings(const QString filename);
};

#endif // NOTESWIDGET_H
