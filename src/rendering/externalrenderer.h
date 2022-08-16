// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef EXTERNALRENDERER_H
#define EXTERNALRENDERER_H

#include <QStringList>
#include "src/config.h"
#include "src/rendering/abstractrenderer.h"

#define MAX_PROCESS_TIME_MS 60000

class QPixmap;
class PngPixmap;
class PdfDocument;

/// Render PDF pages by calling an external program.
class ExternalRenderer : public AbstractRenderer
{
    /// Program used to render pages.
    QString const renderingCommand;

    /// Arguments for rendering program.
    /// These should contain the macros %file (file path), %page (page
    /// number, start counting from 1), and %resolution (in dpi).
    /// Alternatively to %resolution also the sizes %width and %height (in
    /// pixels) can be given. Optionally %format can be given, which should
    /// accept the arguments "png" and "pnm". The format in upper case letters
    /// can be accessed by %Format.
    QStringList renderingArguments;

    /// Document which should be rendered.
    const PdfDocument * const doc;

    /// Arguments to renderingCommand for rendering given page.
    /// Here all macros in renderingArguments are expanded using the arguments
    /// of this function.
    const QStringList getArguments(const int page, const qreal resolution, const QString &format = "png") const;

public:
    /// Constructor, initializes command and arguments. No checks are
    /// performed. Command should contain the fields %file and %page.
    /// Additionally at least one of the fields %resolution or %width and
    /// %height is required.
    ExternalRenderer(const QString& command, const QStringList& arguments, const PdfDocument * const doc, const PagePart page = FullPage);

    /// Trivial destructor.
    ~ExternalRenderer() override {};

    /// Render page to a QPixmap.
    /// Try to set %format to pnm. Resolution is given in pixels per point
    /// (dpi/72).
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;

    /// Render page to PNG image stored in a QByteArray as part of a PngPixmap.
    /// Resolution is given in pixels per point (dpi/72).
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// Check if renderer is valid and can in principle render pages.
    /// Requires that renderingCommand and renderingArguments are not empty
    /// and that renderingArguments contains %page. This does not check
    /// whether renderinCommand is a valid command.
    bool isValid() const override;
};

#endif // EXTERNALRENDERER_H
