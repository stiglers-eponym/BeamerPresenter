// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QFileInfo>
#include <QInputDialog>
#include <QBuffer>
#include "src/rendering/qtdocument.h"
#include "src/rendering/pngpixmap.h"

QtDocument::QtDocument(const QString &filename) :
    PdfDocument(filename),
    doc(new QPdfDocument())
{
    //render_options.setRenderFlags(QPdf::RenderOptimizedForLcd);
    // Load the document
    if (!loadDocument())
        qFatal("Loading document failed");
    debug_msg(DebugRendering, "Loaded PDF document in Qt");
}

bool QtDocument::loadDocument()
{
    // Check if the file exists.
    const QFileInfo fileinfo(path);
    if (!fileinfo.exists() || !fileinfo.isFile())
    {
        preferences()->showErrorMessage(
                    QObject::tr("Error while loading file"),
                    QObject::tr("Given filename is not a file: ") + fileinfo.baseName());
        return false;
    }
    // Check if the file has changed since last (re)load
    if (doc->status() == QPdfDocument::Ready && fileinfo.lastModified() == lastModified)
        return false;

    // Load the document.
    doc->close();
    switch (doc->load(path))
    {
    case QPdfDocument::DocumentError::FileNotFoundError:
        qCritical() << "Could not load document: file not found" << path;
        break;
    case QPdfDocument::DocumentError::DataNotYetAvailableError:
        qCritical() << "Could not load document: data not yet available" << path;
        break;
    case QPdfDocument::DocumentError::InvalidFileFormatError:
        qCritical() << "Could not load document: invalid file format" << path;
        break;
    case QPdfDocument::DocumentError::IncorrectPasswordError:
        // Try to unlock a locked document.
        {
            qWarning() << "Document is locked.";
            // Use a QInputDialog to ask for the password.
            bool ok;
            QString const password = QInputDialog::getText(
                        NULL,
                        QObject::tr("Document is locked!"),
                        QObject::tr("Please enter password (leave empty to cancel)."),
                        QLineEdit::Password,
                        QString(),
                        &ok
                        );
            // Check if a password was entered and try to unlock document.
            // Only user password is required, since we only read the document.
            if (!ok || password.isEmpty())
            {
                preferences()->showErrorMessage(
                            QObject::tr("Error while loading file"),
                            QObject::tr("No password provided for locked document"));
                return false;
            }
            doc->setPassword(password);
            if (doc->load(path) != QPdfDocument::NoError)
            {
                preferences()->showErrorMessage(
                            QObject::tr("Error while loading file"),
                            QObject::tr("Invalid password provided for locked document"));
                return false;
            }
        }
        break;
    case QPdfDocument::DocumentError::UnsupportedSecuritySchemeError:
        qCritical() << "Could not load document: unsupported security scheme" << path;
        break;
    case QPdfDocument::DocumentError::UnknownError:
        qCritical() << "Could not load document: unknown error" << path;
        break;
    case QPdfDocument::DocumentError::NoError:
        break;
    }

    // Save the modification time.
    lastModified = fileinfo.lastModified();
    flexible_page_sizes = -1;

    return true;
}

const QPixmap QtDocument::getPixmap(const int page, const qreal resolution, const PagePart page_part) const
{
    if (resolution <= 0 || page < 0 || page >= doc->pageCount())
    {
        qWarning() << "Tried to render invalid page or invalid resolution" << page;
        return QPixmap();
    }
    const QImage image = doc->render(page, (resolution*doc->pageSize(page)).toSize(), render_options);
    switch (page_part)
    {
    case LeftHalf:
        return QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
    case RightHalf:
        return QPixmap::fromImage(image.copy((image.width()+1)/2, 0, image.width()/2, image.height()));
    default:
        return QPixmap::fromImage(image);
    }
}

const PngPixmap * QtDocument::getPng(const int page, const qreal resolution, const PagePart page_part) const
{
    if (resolution <= 0 || page < 0 || page >= doc->pageCount())
    {
        qWarning() << "Tried to render invalid page or invalid resolution" << page;
        return NULL;
    }
    QImage image = doc->render(page, (resolution*doc->pageSize(page)).toSize(), render_options);
    if (image.isNull())
    {
        qWarning() << "Rendering page to image failed";
        return NULL;
    }
    switch (page_part)
    {
    case LeftHalf:
        image = image.copy(0, 0, image.width()/2, image.height());
        break;
    case RightHalf:
        image = image.copy((image.width()+1)/2, 0, image.width()/2, image.height());
        break;
    default:
        break;
    }
    QByteArray* const bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG"))
    {
        qWarning() << "Saving page as PNG image failed";
        delete bytes;
        return NULL;
    }
    return new PngPixmap(bytes, page, resolution);
}

bool QtDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || !isValid())
        return flexible_page_sizes;
    const QSizeF ref_size = doc->pageSize(0);
    for (int page=1; page<doc->pageCount(); page++)
    {
        if (doc->pageSize(page) != ref_size)
        {
            flexible_page_sizes = 1;
            return 1;
        }
    }
    flexible_page_sizes = 0;
    return 0;
}
