#include "noteswidget.h"

NotesWidget::NotesWidget(QWidget *parent) : QTextEdit(parent)
{
    setReadOnly(false);
    setAutoFormatting(QTextEdit::AutoAll);
}

void NotesWidget::load(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    if (file.isReadable())
    {
        setMarkdown(file.readAll());
        file_path = filename;
    }
}

void NotesWidget::save(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    if (file.isWritable())
    {
        file.write(toMarkdown().toUtf8());
        file_path = filename;
    }
}

void NotesWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key() | event->modifiers())
    {
    case Qt::Key_O | Qt::ControlModifier:
        load();
        event->accept();
        break;
    case Qt::Key_S | Qt::ControlModifier:
        save();
        event->accept();
        break;
    case Qt::Key_S | Qt::ShiftModifier | Qt::ControlModifier:
        saveAs();
        event->accept();
        break;
    case Qt::Key_M | Qt::ControlModifier:
        updateMarkdown();
        break;
    case Qt::Key_Plus | Qt::ControlModifier:
    case Qt::Key_Plus | Qt::ShiftModifier | Qt::ControlModifier:
        zoomIn();
        break;
    case Qt::Key_Minus | Qt::ControlModifier:
        zoomOut();
        break;
    default:
        QTextEdit::keyPressEvent(event);
    }
}
