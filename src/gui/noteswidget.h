#ifndef NOTESWIDGET_H
#define NOTESWIDGET_H

#include <QFile>
#include <QDebug>
#include <QTextEdit>
#include <QFileDialog>
#include <QKeyEvent>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "src/preferences.h"

/**
 * @brief NotesWidget: Editable notes in Markdown format
 *
 * Widgte in flexible GUI of BeamerPresenter which represents markdown
 * formated text which can be loaded, saved, and edited.
 *
 * File structure of the saved xml files: xml tree with root element
 * "speakernotes" containing elements "page". Each "page" element has
 * an attribute "label" representing the page label (a string) and
 * contains text in markdown format.
 *
 * Keyboard shortcuts for the page label are hard-coded:
 * Ctrl+S for saving, Ctrl+Shift+S for saving under a different file
 * name and Ctrl+O for loading a different file.
 *
 * TODO: option to parse HTML instead of markdown.
 */
class NotesWidget : public QTextEdit
{
    Q_OBJECT

    /// File where everything is loaded / saved.
    QString file_path;

    /// Map slide labels to markdown formatted text
    QMap<QString, QString> text_per_slide;

    /// Currently shown page label.
    QString page_label;

    /// Are notes saved per page? Otherwise per page label.
    bool per_page;

public:
    NotesWidget(const bool per_page = false, QWidget *parent = NULL);

    bool hasHeightForWidth() const noexcept override
    {return true;}

    void load(const QString &filename);
    void save(const QString &filename);
#if defined(QT_FEATURE_textmarkdownreader) && defined(QT_FEATURE_textmarkdownwriter)
    void updateMarkdown() {setMarkdown(toMarkdown());}
#else
    void updateMarkdown() const noexcept {}
#endif

protected:
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void load() {load(QFileDialog::getOpenFileName());}
    void save() {save(file_path.isEmpty() ? QFileDialog::getSaveFileName() : file_path);}
    void saveAs() {save(QFileDialog::getSaveFileName());}
    void pageChanged(const int page);
};

#endif // NOTESWIDGET_H
