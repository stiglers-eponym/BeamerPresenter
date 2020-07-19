#include "src/rendering/externalrenderer.h"

ExternalRenderer::ExternalRenderer(const QString& command, const QStringList &arguments, const PdfDocument * const doc) :
    renderingCommand(command),
    renderingArguments(arguments),
    doc(doc)
{
    renderingArguments.replaceInStrings("%file", doc->getPath());
}

const QStringList ExternalRenderer::getArguments(const int page, const qreal resolution, const QString &format) const
{
    QStringList command = renderingArguments;
    command.replaceInStrings("%page", QString::number(page));
    command.replaceInStrings("%resolution", QString::number(resolution));
    command.replaceInStrings("%format", format);

    // Calculate size of target image using page and resolution.
    // TODO: first check if calculating the size is needed.
    const QSize size = (resolution * doc->pageSize(page)).toSize();
    command.replaceInStrings("%width", QString::number(size.width()));
    command.replaceInStrings("%height", QString::number(size.height()));
    return command;
}

const PngPixmap * ExternalRenderer::renderPng(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0)
        return nullptr;
    QProcess *process = new QProcess();
    process->start(renderingCommand, getArguments(page, resolution, "PNG"), QProcess::ReadOnly);
    if (!process->waitForFinished(MAX_PROCESS_TIME_MS))
    {
        // TODO: clean up correctly
        process->kill();
        delete process;
        return nullptr;
    }
    const QByteArray * data = new QByteArray(process->readAllStandardOutput());
    // TODO: handle error messages and exit code sent by process.
    process->deleteLater();
    return new PngPixmap(data, page, resolution);
}

const QPixmap ExternalRenderer::renderPixmap(const int page, const qreal resolution) const
{
    if (resolution <= 0 || page < 0)
        return QPixmap();
    QProcess *process = new QProcess();
    process->start(renderingCommand, getArguments(page, resolution, "PNM"), QProcess::ReadOnly);
    if (!process->waitForFinished(MAX_PROCESS_TIME_MS))
    {
        // TODO: clean up correctly
        process->kill();
        delete process;
        return QPixmap();
    }
    const QByteArray data = process->readAllStandardOutput();
    process->deleteLater();
    QPixmap pixmap;
    if (!pixmap.loadFromData(data))
        qWarning() << "Failed to load data from external renderer";
    return pixmap;
}

bool ExternalRenderer::isValid() const
{
    /* Very basic check:
    * Is a command defined?
    * Does it take arguments?
    * Do these arguments include file path and page?
    */
    return  !renderingCommand.isEmpty()
            && !renderingArguments.isEmpty()
            && renderingArguments.indexOf(QRegExp(".*%file.*")) != -1
            && renderingArguments.indexOf(QRegExp(".*%page.*")) != -1;
}
