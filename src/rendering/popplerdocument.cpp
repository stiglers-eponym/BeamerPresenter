#include "src/rendering/popplerdocument.h"

PopplerDocument::PopplerDocument(const QString &filename) :
    PdfDocument(filename)
{
    // Load the document
    if (!loadDocument())
        qFatal("Loading document failed");
}

PopplerDocument::~PopplerDocument()
{
    delete doc;
}

int PopplerDocument::numberOfPages() const
{
    return doc->numPages();
}

const QString PopplerDocument::label(const int page) const
{
    return doc->page(page)->label();
}

const QSizeF PopplerDocument::pageSize(const int page) const
{
    return doc->page(page)->pageSizeF();
}

bool PopplerDocument::loadDocument()
{
    // Check if the file exists.
    QFileInfo const fileinfo(path);
    if (!fileinfo.exists() || !fileinfo.isFile())
    {
        qCritical() << "Given filename is not a file.";
        return false;
    }
    // Check if the file has changed since last (re)load
    if (doc != nullptr && fileinfo.lastModified() == lastModified)
        return false;

    // Load the document.
    Poppler::Document * newdoc = Poppler::Document::load(path);
    if (newdoc == nullptr)
    {
        qCritical() << "Failed to load document.";
        return false;
    }

    // Try to unlock a locked document.
    if (newdoc->isLocked())
    {
        qWarning() << "Document is locked.";
        // Use a QInputDialog to ask for the password.
        bool ok;
        QString const password = QInputDialog::getText(
                    nullptr,
                    "Document is locked!",
                    "Please enter password (leave empty to cancel).",
                    QLineEdit::Password,
                    QString(),
                    &ok
                    );
        // Check if a password was entered.
        if (!ok || password.isEmpty())
        {
            delete newdoc;
            newdoc = nullptr;
            qCritical() << "No password provided for locked document";
            return false;
        }
        // Try to unlock document.
        if (!newdoc->unlock(QByteArray(), password.toLatin1()))
        {
            delete newdoc;
            newdoc = nullptr;
            qCritical() << "Unlocking document failed: wrong password or bug.";
            return false;
        }
    }
    // Save the modification time.
    lastModified = fileinfo.lastModified();

    // Set rendering hints.
    newdoc->setRenderHint(Poppler::Document::TextAntialiasing);
    newdoc->setRenderHint(Poppler::Document::TextHinting);
    newdoc->setRenderHint(Poppler::Document::TextSlightHinting);
    newdoc->setRenderHint(Poppler::Document::Antialiasing);
    newdoc->setRenderHint(Poppler::Document::ThinLineShape);

    // Update document and delete old document.
    if (doc == nullptr)
        doc = newdoc;
    else
    {
        const Poppler::Document * const olddoc = doc;
        doc = newdoc;
        delete olddoc;
    }
    return true;
}

const QPixmap PopplerDocument::getPixmap(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0 || page >= doc->numPages())
        return QPixmap();
    const Poppler::Page * const popplerPage = doc->page(page);
    if (popplerPage == nullptr)
    {
        qWarning() << "Tried to render invalid page" << page;
        return QPixmap();
    }
    return QPixmap::fromImage(popplerPage->renderToImage(72.*resolution, 72.*resolution));
}

const PngPixmap * PopplerDocument::getPng(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0)
        return nullptr;
    const Poppler::Page * const popplerPage = doc->page(page);
    if (popplerPage == nullptr)
    {
        qWarning() << "Tried to render invalid page" << page;
        return nullptr;
    }
    const QImage image = popplerPage->renderToImage(72.*resolution, 72.*resolution);
    if (image.isNull())
    {
        qWarning() << "Rendering page to image failed";
        return nullptr;
    }
    QByteArray* const bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG"))
    {
        qWarning() << "Saving page as PNG image failed";
        delete bytes;
        return nullptr;
    }
    return new PngPixmap(bytes, page, resolution);
}

bool PopplerDocument::isValid() const
{
    return doc != nullptr && !doc->isLocked();
}
