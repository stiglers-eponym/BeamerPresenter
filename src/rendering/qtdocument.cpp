// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QVector>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QSizeF>
#include <QByteArray>
#include <QPdfDocument>

#include "src/log.h"
#include "src/preferences.h"
#include "src/rendering/qtdocument.h"
#include "src/rendering/qtrenderer.h"
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
                    tr("Error while loading file"),
                    tr("Given filename is not a file: ") + fileinfo.baseName());
        return false;
    }
    // Check if the file has changed since last (re)load
    if (doc->status() == QPdfDocument::Status::Ready && fileinfo.lastModified() == lastModified)
        return false;

    // Load the document.
    doc->close();
    switch (doc->load(path))
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::FileNotFound:
#else
    case QPdfDocument::DocumentError::FileNotFoundError:
#endif
        qCritical() << tr("Could not load document: file not found") << path;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::DataNotYetAvailable:
#else
    case QPdfDocument::DocumentError::DataNotYetAvailableError:
#endif
        qCritical() << tr("Could not load document: data not yet available") << path;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::InvalidFileFormat:
#else
    case QPdfDocument::DocumentError::InvalidFileFormatError:
#endif
        qCritical() << tr("Could not load document: invalid file format") << path;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::IncorrectPassword:
#else
    case QPdfDocument::DocumentError::IncorrectPasswordError:
#endif
        // Try to unlock a locked document.
        {
            qWarning() << "Document is locked.";
            // Use a QInputDialog to ask for the password.
            bool ok;
            QString const password = QInputDialog::getText(
                        nullptr,
                        tr("Document is locked!"),
                        tr("Please enter password (leave empty to cancel)."),
                        QLineEdit::Password,
                        QString(),
                        &ok
                        );
            // Check if a password was entered and try to unlock document.
            // Only user password is required, since we only read the document.
            if (!ok || password.isEmpty())
            {
                preferences()->showErrorMessage(
                            tr("Error while loading file"),
                            tr("No password provided for locked document"));
                return false;
            }
            doc->setPassword(password);
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
            if (doc->load(path) != QPdfDocument::Error::None)
#else
            if (doc->load(path) != QPdfDocument::NoError)
#endif
            {
                preferences()->showErrorMessage(
                            tr("Error while loading file"),
                            tr("Invalid password provided for locked document"));
                return false;
            }
        }
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::UnsupportedSecurityScheme:
#else
    case QPdfDocument::DocumentError::UnsupportedSecuritySchemeError:
#endif
        qCritical() << tr("Could not load document: unsupported security scheme") << path;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::Unknown:
#else
    case QPdfDocument::DocumentError::UnknownError:
#endif
        qCritical() << tr("Could not load document: unknown error") << path;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(6,4,0))
    case QPdfDocument::Error::None:
#else
    case QPdfDocument::DocumentError::NoError:
#endif
        break;
    }

    // Save the modification time.
    lastModified = fileinfo.lastModified();
    flexible_page_sizes = -1;

    return true;
}

AbstractRenderer *QtDocument::createRenderer(const PagePart part) const
{
    return new QtRenderer(this, part);
}

const QPixmap QtDocument::getPixmap(const int page, const qreal resolution, const PagePart page_part) const
{
    if (resolution <= 0 || page < 0 || page >= doc->pageCount())
    {
        qWarning() << "Tried to render invalid page or invalid resolution" << page;
        return QPixmap();
    }
    const QImage image = doc->render(page, (resolution*doc->PAGESIZE_FUNCTION(page)).toSize(), render_options);
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
        return nullptr;
    }
    QImage image = doc->render(page, (resolution*doc->PAGESIZE_FUNCTION(page)).toSize(), render_options);
    if (image.isNull())
    {
        qWarning() << "Rendering page to image failed";
        return nullptr;
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
        return nullptr;
    }
    return new PngPixmap(bytes, page, resolution);
}

bool QtDocument::flexiblePageSizes() noexcept
{
    if (flexible_page_sizes >= 0 || !isValid())
        return flexible_page_sizes;
    const QSizeF ref_size = doc->PAGESIZE_FUNCTION(0);
    for (int page=1; page<doc->pageCount(); page++)
    {
        if (doc->PAGESIZE_FUNCTION(page) != ref_size)
        {
            flexible_page_sizes = 1;
            return 1;
        }
    }
    flexible_page_sizes = 0;
    return 0;
}
