#include "src/gui/noteswidget.h"
#include "src/preferences.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QKeyEvent>

NotesWidget::NotesWidget(const bool per_page, QWidget *parent) :
    QTextEdit(parent),
    per_page(per_page)
{
    setReadOnly(false);
    setAutoFormatting(QTextEdit::AutoAll);
}

void NotesWidget::loadNotes(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    if (file.isReadable())
    {
        QXmlStreamReader reader(&file);
        reader.readNext();
        while (!reader.atEnd())
        {
            if (reader.readNext() == QXmlStreamReader::StartElement)
            {
                if (reader.name() == "speakernotes")
                    readNotes(reader);
                else if (reader.name() == "xournal")
                {
                    // Trying to read a file which should probably be read by PdfMaster.
                    reader.clear();
                    file.close();
                    emit loadDrawings(filename);
                    return;
                }
                break;
            }
        }
        if (!reader.hasError())
            file_path = filename;
        else
            qWarning() << "Parsing xml failed:" << reader.errorString();
    }
}

void NotesWidget::readNotes(QXmlStreamReader &reader)
{
    /// It is assumed that reader has just reached the beginning of speakernotes
    debug_msg(DebugWidgets) << "start reading notes for notes widget";
    if (reader.name() != "speakernotes")
    {
        warn_msg << "Tried to read notes, but current element in xml tree is not speakernotes";
        return;
    }
    const QString identifier = reader.attributes().value("identifier").toString();
    if (identifier == "number")
        per_page = true;
    else if (identifier == "label")
        per_page = false;
    while (reader.readNextStartElement())
    {
        debug_msg(DebugWidgets) << "read notes:" << reader.name();
        if (reader.name() == "page-notes")
        {
            const QString label = reader.attributes().value(per_page ? "number" : "label").toString();
            if (!label.isEmpty())
                text_per_slide.insert(label, reader.readElementText());
        }
        if (!reader.isEndElement())
            reader.skipCurrentElement();
    }

    if (preferences()->document)
#ifdef QT_FEATURE_textmarkdownreader
        setMarkdown(text_per_slide.value(preferences()->document->pageLabel(preferences()->page)));
#else
        setText(text_per_slide.value(preferences()->document->pageLabel(preferences()->page)));
#endif
}

void NotesWidget::save(QString filename)
{
    if (filename.isEmpty())
    {
        filename = QFileDialog::getSaveFileName(
                    NULL,
                    "Save notes",
                    "",
                    "Note files (*.xml);;BeamerPresenter/Xournal++ files (*.bpr *.xopp);;All files (*)"
                );
        if (filename.endsWith(".bpr", Qt::CaseInsensitive) || filename.endsWith(".xopp", Qt::CaseInsensitive))
            emit saveDrawings(filename);
        else
            saveNotes(filename);
    }
    else
        saveNotes(file_path);
}

void NotesWidget::load()
{
    const QString filename = QFileDialog::getOpenFileName(
                NULL,
                "Open notes",
                "",
                "Note files (*.xml);;BeamerPresenter/Xournal++ files (*.bpr *.xopp *.xoj *.xml);;All files (*)"
            );
    QMimeDatabase db;
    const QMimeType type = db.mimeTypeForFile(filename, QMimeDatabase::MatchContent);
    if (type.name() == "application/gzip")
        emit loadDrawings(filename);
    else if (type.name() == "application/xml")
        loadNotes(filename);
    else
        qWarning() << "Could not open file: unknown file type" << type.name();
}

void NotesWidget::saveNotes(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    if (!file.isWritable())
    {
        qWarning() << "Saving notes failed, file is not writable:" << filename;
        return;
    }
    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(0);
    writer.writeStartDocument();
    writeNotes(writer);
    writer.writeEndDocument();
    if (writer.hasError())
        qWarning() << "Saving notes resulted in an error";
    else
        file_path = filename;
}

void NotesWidget::writeNotes(QXmlStreamWriter &writer)
{
#ifdef QT_FEATURE_textmarkdownwriter
    text_per_slide.insert(page_label, toMarkdown());
#else
    text_per_slide.insert(page_label, toPlainText());
#endif
    writer.writeStartElement("speakernotes");
    writer.writeAttribute("identifier", per_page ? "number" : "label");
    const QString label = per_page ? "number" : "label";
    for (auto it = text_per_slide.cbegin(); it != text_per_slide.cend(); ++it)
    {
        if (!it->isEmpty())
        {
            writer.writeStartElement("page-notes");
            writer.writeAttribute(label, it.key());
            writer.writeCharacters(it.value());
            writer.writeEndElement();
        }
    }
    writer.writeEndElement();
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
        save(file_path);
        event->accept();
        break;
    case Qt::Key_S | Qt::ShiftModifier | Qt::ControlModifier:
        save("");
        event->accept();
        break;
#if defined(QT_FEATURE_textmarkdownreader) && defined(QT_FEATURE_textmarkdownwriter)
    case Qt::Key_M | Qt::ControlModifier:
        updateMarkdown();
        break;
#endif
    case Qt::Key_Plus | Qt::ControlModifier:
    case Qt::Key_Plus | Qt::ShiftModifier | Qt::ControlModifier:
        zoomIn();
        break;
    case Qt::Key_Minus | Qt::ControlModifier:
        zoomOut();
        break;
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        event->ignore();
        break;
    default:
        QTextEdit::keyPressEvent(event);
    }
}

void NotesWidget::pageChanged(const int page)
{
    // Save current text to old page_label.
#ifdef QT_FEATURE_textmarkdownwriter
    QString string = toMarkdown();
#else
    QString string = toPlainText();
#endif
    if (text_per_slide.value(page_label) != string)
    {
        text_per_slide.insert(page_label, string);
        emit newUnsavedChanges();
    }
    // Update page_label.
    if (per_page)
        page_label = QString::number(page);
    else
    {
        const PdfDocument *doc = preferences()->document;
        page_label = doc ? doc->pageLabel(page) : "0";
    }
    // Load text from new page_label.
#ifdef QT_FEATURE_textmarkdownreader
    setMarkdown(text_per_slide.value(page_label));
#else
    setPlainText(text_per_slide.value(page_label));
#endif
}
