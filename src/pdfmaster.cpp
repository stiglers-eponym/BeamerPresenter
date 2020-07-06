#include <src/pdfmaster.h>

PdfMaster::PdfMaster(const QString &filename) :
    pdfpath(filename)
{
    // Load the document
    if (!loadDocument())
        qFatal("Loading document failed");
}

PdfMaster::~PdfMaster()
{
    qDeleteAll(paths);
    paths.clear();
    delete document;
}

const QSizeF PdfMaster::getPageSize(const int page_number) const
{
    Poppler::Page const *page = document->page(page_number);
    if (page == nullptr)
        return QSizeF();
    return page->pageSizeF();
}

bool PdfMaster::loadDocument()
{
    QFileInfo const fileinfo(pdfpath);

    // Check if the file exists.
    if (!fileinfo.exists() || !fileinfo.isFile())
    {
        qCritical() << "Given filename is not a file.";
        return false;
    }
    // Check if the file has changed since last (re)load
    if (document != nullptr && fileinfo.lastModified() == lastModified)
        return false;

    // Load the document.
    Poppler::Document * newdoc = Poppler::Document::load(pdfpath);
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
    if (document == nullptr)
        document = newdoc;
    else
    {
        Poppler::Document * const olddoc = document;
        document = newdoc;
        delete olddoc;
    }
    return true;
}
