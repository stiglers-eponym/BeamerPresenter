#ifndef EXTERNALRENDERER_H
#define EXTERNALRENDERER_H

#include <QProcess>
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/pdfdocument.h"

#define MAX_PROCESS_TIME_MS 60000

/// Render PDF pages by calling an external program.
class ExternalRenderer : public AbstractRenderer
{
    /// Program used to render pages.
    QString const renderingCommand;
    /// Arguments for rendering program.
    /// These should contain the macros %file, %page, and %resolution.
    /// Alternatively to %resolution also the sizes %width and %height (in
    /// pixels) can be given. Optionally %format can be given, which should
    /// accept the arguments "PNG" and "PNM".
    QStringList renderingArguments;
    /// Document which should be rendered.
    const PdfDocument * const doc;

    /// Arguments to renderingCommand for rendering given page.
    /// Here all macros in renderingArguments are expanded using the arguments
    /// of this function.
    const QStringList getArguments(const int page, const qreal resolution, const QString &format = "PNG") const;

public:
    /// Constructor, initializes command and arguments. No checks are
    /// performed. Command should contain the fields %file and %page.
    /// Additionally at least one of the fields %resolution or %width and
    /// %height is required.
    ExternalRenderer(const QString& command, const QStringList& arguments, const PdfDocument * const doc, const PagePart page = FullPage);
    /// Trivial destructor.
    ~ExternalRenderer() override {};

    /// Render page to a QPixmap.
    /// Try to set %format to PNM. Resolution is given in dpi.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image stored in a QByteArray as part of a PngPixmap.
    /// Resolution is given in dpi.
    const PngPixmap * renderPng(const int page, const qreal resolution) const override;

    /// Check if renderer is valid and can in principle render pages.
    /// Requires that renderingCommand and renderingArguments are not empty
    /// and that renderingArguments contains %page. This does not check
    /// whether renderinCommand is a valid command.
    bool isValid() const override;
};

#endif // EXTERNALRENDERER_H
