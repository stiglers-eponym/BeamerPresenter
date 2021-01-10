#ifndef NOTESWIDGET_H
#define NOTESWIDGET_H

#include <QFile>
#include <QDebug>
#include <QTextEdit>
#include <QFileDialog>
#include <QKeyEvent>

/**
 * @brief NotesWidget: Editable notes in Markdown format
 *
 * Widgte in flexible GUI of BeamerPresenter which represents markdown
 * formated text which can be loaded, saved, and edited.
 */
class NotesWidget : public QTextEdit
{
    Q_OBJECT
    QString file_path;

protected:
    void keyPressEvent(QKeyEvent *event) override;

public:
    NotesWidget(QWidget *parent);

    bool hasHeightForWidth() const noexcept override
    {return true;}

    void load(const QString &filename);
    void save(const QString &filename);
    void updateMarkdown() {setMarkdown(toMarkdown());}

public slots:
    void load() {load(QFileDialog::getOpenFileName());}
    void save() {save(file_path.isEmpty() ? QFileDialog::getSaveFileName() : file_path);}
    void saveAs() {save(QFileDialog::getSaveFileName());}
};

#endif // NOTESWIDGET_H
