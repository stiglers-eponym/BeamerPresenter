#include "externalrenderer.h"

ExternalRenderer::ExternalRenderer(int const page, QObject* parent) : QProcess(parent)
{
    this->page = page;
    connect(this, SIGNAL(finished(int const, QProcess::ExitStatus)), this, SLOT(returnImage(int const, QProcess::ExitStatus)));
    //execute(command, arguments << QString(page));
}

void ExternalRenderer::returnImage(int const exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != 0 || exitCode != 0) {
        qWarning() << "Call to external renderer failed, exit code" << exitCode;
        return;
    }
    bytes = new QByteArray(readAllStandardOutput());
    emit sendImage(bytes, page);
}
