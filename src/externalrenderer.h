#ifndef EXTERNALRENDERER_H
#define EXTERNALRENDERER_H

#include <QProcess>
#include "src/abstractrenderer.h"
#include "src/pdfmaster.h"

#define MAX_PROCESS_TIME_MS 60000

class ExternalRenderer : public AbstractRenderer, QObject
{
    QString const renderingCommand;
    QStringList renderingArguments;
    const PdfMaster * const pdfMaster;

    /// Arguments to renderingCommand for rendering page with resolution.
    const QStringList getArguments(const int page, const qreal resolution, const QString &format = "PNG") const;

public:
    /// Command should contain the fields %file and %page. Additionally at least
    /// one of the fields %resolution or %width and %height is required.
    ExternalRenderer(const QString& command, const QStringList& arguments, const PdfMaster * const master);
    /// Render page to a QPixmap.
    /// Try to use "%format" in command.
    const QPixmap renderPixmap(const int page, const qreal resolution) const override;
    /// Render page to PNG image in a QByteArray.
    const QByteArray * renderPng(const int page, const qreal resolution) const override;
};

#endif // EXTERNALRENDERER_H
