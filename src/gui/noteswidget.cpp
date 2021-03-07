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

void NotesWidget::load(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::ReadOnly | QFile::Text);
    if (file.isReadable())
    {
        QXmlStreamReader reader(&file);
        while (!reader.atEnd())
        {
            debug_msg(DebugWidgets) << reader.name();
            if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "speakernotes")
            {
                const QString identifier = reader.attributes().value("identifier").toString();
                if (identifier == "number")
                    per_page = true;
                else if (identifier == "label")
                    per_page = false;
            }
            if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "page")
            {
                const QString label = reader.attributes().value(per_page ? "number" : "label").toString();
                if (!label.isEmpty())
                    text_per_slide.insert(label, reader.readElementText());
            }
        }
        if (!reader.hasError())
            file_path = filename;
        else
            qWarning() << "Parsing xml failed:" << reader.errorString();

        if (preferences()->document)
#ifdef QT_FEATUER_textmarkdownreader
            setMarkdown(text_per_slide.value(preferences()->document->pageLabel(preferences()->page)));
#else
            setText(text_per_slide.value(preferences()->document->pageLabel(preferences()->page)));
#endif
    }
}

void NotesWidget::save(const QString &filename)
{
#ifdef QT_FEATURE_textmarkdownwriter
    text_per_slide.insert(page_label, toMarkdown());
#else
    text_per_slide.insert(page_label, toPlainText());
#endif
    QFile file(filename);
    file.open(QFile::WriteOnly | QFile::Text);
    if (file.isWritable())
    {
        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(0);
        writer.writeStartDocument();
        writer.writeStartElement("speakernotes");
        writer.writeAttribute("identifier", per_page ? "number" : "label");
        const QString label = per_page ? "number" : "label";
        for (auto it = text_per_slide.cbegin(); it != text_per_slide.cend(); ++it)
        {
            writer.writeStartElement("page");
            writer.writeAttribute(label, it.key());
            writer.writeCharacters(it.value());
            writer.writeEndElement();
        }
        writer.writeEndElement();
        writer.writeEndDocument();
        if (!writer.hasError())
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
#ifdef QT_FEATURE_textmarkdownwriter
    text_per_slide.insert(page_label, toMarkdown());
#else
    text_per_slide.insert(page_label, toPlainText());
#endif
    if (per_page)
        page_label = QString::number(page);
    else
    {
        const PdfDocument *doc = preferences()->document;
        page_label = doc ? doc->pageLabel(page) : "0";
    }
#ifdef QT_FEATURE_textmarkdownreader
    setMarkdown(text_per_slide.value(page_label));
#else
    setPlainText(text_per_slide.value(page_label));
#endif
}
